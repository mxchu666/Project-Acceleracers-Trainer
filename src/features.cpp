#include "features.hpp"

#include <atomic>
#include <chrono>

Settings      g_cfg;
volatile LONG g_running  = 0;
volatile bool g_menuOpen = false;

namespace {

std::atomic<unsigned> g_pending{0};

constexpr double kPi = 3.14159265358979323846;

// ---- noclip bookkeeping -------------------------------------------------
std::uintptr_t g_noclipCar      = 0;
std::uintptr_t g_noclipWorld    = 0;
bool           g_noclipPrepared = false;

// ---- realm finish bookkeeping -------------------------------------------
// The car is a physics vehicle, so one SetActorLocation does not stick and the
// portal overlap can miss. Parking the car on the portal for two seconds with
// its momentum cleared makes the overlap register. Time-based, not tick-based,
// so the window does not stretch with the frame rate.
ue::FVector    g_pinTarget{};
std::uint64_t  g_pinUntil = 0;
std::uintptr_t g_pinCar   = 0;

constexpr std::uint64_t kPinMs = 2000;

void KillVelocity(std::uintptr_t car);

void BeginNoclip(std::uintptr_t car) {
    if (g_noclipPrepared) return;
    // Collision goes off so the car sweeps through world geometry. Physics stays
    // ON on purpose: SetSimulatePhysics(false) pulls the body out of the physics
    // scene, and the vehicle movement component's UpdateState then writes the
    // stale body transform back over the actor every tick. That is what made the
    // car refuse to climb in Storm Realm - it stuck wherever the body still was.
    acc::SetActorCollision(car, false);
    g_noclipPrepared = true;
}

void EndNoclip() {
    if (g_noclipPrepared && g_noclipCar) {
        acc::SetActorCollision(g_noclipCar, true);
        acc::SetRootSimulatePhysics(g_noclipCar, true);   // in case an earlier build left it off
    }
    g_noclipPrepared = false;
    g_noclipCar      = 0;
    g_noclipWorld    = 0;
}

void NoclipTick(std::uintptr_t car, double dt) {
    if (car != g_noclipCar) {
        EndNoclip();
        g_noclipCar   = car;
        g_noclipWorld = ue::World();
    }
    BeginNoclip(car);

    ue::FVector loc{};
    if (!acc::GetActorLocation(car, loc)) return;

    const ue::FRotator r = acc::ControlRotation();
    const double yaw     = r.yaw   * kPi / 180.0;
    const double pitch   = r.pitch * kPi / 180.0;

    const ue::FVector fwd{
        static_cast<float>(std::cos(yaw) * std::cos(pitch)),
        static_cast<float>(std::sin(yaw) * std::cos(pitch)),
        static_cast<float>(std::sin(pitch))};
    const ue::FVector right{
        static_cast<float>(-std::sin(yaw)),
        static_cast<float>(std::cos(yaw)),
        0.f};
    const ue::FVector up{0.f, 0.f, 1.f};

    ue::FVector move{0.f, 0.f, 0.f};
    auto down = [](int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; };

    auto add = [&move](const ue::FVector& v, float s) {
        move.x += v.x * s;
        move.y += v.y * s;
        move.z += v.z * s;
    };

    if (down('W')) add(fwd, 1.f);
    if (down('S')) add(fwd, -1.f);
    if (down('D')) add(right, 1.f);
    if (down('A')) add(right, -1.f);
    if (down(VK_SPACE)) add(up, 1.f);
    if (down(VK_CONTROL) || down('C')) add(up, -1.f);

    const float len = std::sqrt(move.x * move.x + move.y * move.y + move.z * move.z);
    if (len > 0.001f) {
        const float step = g_cfg.noclipSpeed * static_cast<float>(dt) / len;
        loc.x += move.x * step;
        loc.y += move.y * step;
        loc.z += move.z * step;
    }

    // Re-seating every tick also cancels gravity while the menu is idle. The
    // rigid-body velocity is cleared in the same tick because the physics step
    // integrates between frames: without it the body keeps the momentum it had on
    // entry and drifts while the actor is being teleported back.
    acc::SetActorLocation(car, loc, false, true);
    KillVelocity(car);
}

bool RunActions(unsigned bits) {
    const std::uintptr_t gi = ue::GameInstance();

    if (bits & ACT_UnlockCars)    acc::UnlockAllCars(gi);
    if (bits & ACT_UnlockAccel)   acc::UnlockAllAccelechargers(gi);
    if (bits & ACT_SetRecords)    acc::SetAllRealmRecords(gi, g_cfg.recordSeconds);
    if (bits & ACT_SaveProgress)  acc::SaveProgress(gi);

    if (bits & ACT_FinishRealm) {
        // The overlap handler is the finish line. Reaching it directly works in
        // every realm, including the three whose .umap dumps carried no actors,
        // because the portal actor is found live in the loaded level.
        bool hub = false;
        const std::uintptr_t portal = acc::FindSolePortal(&hub);
        const std::uintptr_t car    = acc::LocalCar();
        if (portal && !hub && car && acc::CompleteRealm(portal, car)) return true;

        // Fallback only: park the car in the ring for two seconds. Arming it after
        // a successful handler call would teleport the car into a level that is
        // already travelling.
        ue::FVector target{};
        if (acc::RealmPortalTarget(target)) {
            g_pinTarget = target;
            g_pinCar    = car;
            g_pinUntil  = GetTickCount64() + kPinMs;
        }
    }
    return false;
}

// Zeroes the rigid-body velocity so the car stays parked instead of coasting
// back out of the portal volume.
void KillVelocity(std::uintptr_t car) {
    std::uintptr_t root = 0;
    if (!ue::Read(car + ue::lay::AActor_RootComponent, root) || !ue::Plausible(root)) return;

    // UPrimitiveComponent::SetPhysicsLinearVelocity(FVector(0x0C) bAddToCurrent(1) pad FName(8) @0x10)
    alignas(16) std::uint8_t buf[0x20] = {};
    ue::CallByName(root, "SetPhysicsLinearVelocity", buf);
}

// Returns true while the pin is still running.
bool PinTick(std::uintptr_t car) {
    if (!g_pinUntil) return false;
    if (GetTickCount64() > g_pinUntil || car != g_pinCar) {
        g_pinUntil = 0;
        return false;
    }

    acc::SetActorCollision(car, true);          // overlap events need collision on

    // The trigger volume is centred on the portal actor, so the root is the one
    // point guaranteed to be inside it. A lift here just risks parking the car
    // above the trigger.
    acc::SetActorLocation(car, g_pinTarget, false, true);
    KillVelocity(car);
    return true;
}

// Runs on the game thread, from the ProcessEvent hook.
void FrameTick(double dt) {
    const unsigned bits = g_pending.exchange(0);
    // A finish that reached the portal handler is already travelling; touching the
    // world again in this same frame is how a travel turns into an assert.
    if (bits && RunActions(bits)) return;

    const std::uintptr_t car = acc::LocalCar();
    if (!car) {
        g_pinCar   = 0;
        g_pinUntil = 0;
        if (g_noclipPrepared) EndNoclip();
        return;
    }

    if (g_cfg.unlimitedNitrox)    acc::ApplyUnlimitedNitrox(car, true);
    if (g_cfg.unlimitedJumpJets)  acc::ApplyUnlimitedJumpJets(car, true);
    if (g_cfg.perfectHandling)    acc::ApplyPerfectHandling(car, true, g_cfg.grip);
    if (g_cfg.accelechargerBoost) acc::ApplyAccelechargerUses(car, g_cfg.accelechargerUses);
    if (g_cfg.engineBoost)        acc::ApplyEngineHealth(car, g_cfg.engineHp);

    acc::ApplyGodMode(car, g_cfg.godMode);
    acc::ApplyHydroWater(car, g_cfg.hydroWater);
    if (g_cfg.noSandStall) acc::ApplyNoSandStall(car, g_cfg.sandFloor);

    if (PinTick(car)) return;

    if (g_cfg.noclip) NoclipTick(car, dt);
    else if (g_noclipPrepared) EndNoclip();
}

void Shutdown() {
    g_pinUntil = 0;
    if (g_noclipPrepared) EndNoclip();
    acc::ApplyPerfectHandling(0, false, 0.f);
    acc::ApplyGodMode(0, false);
    acc::ApplyHydroWater(0, false);
}

} // namespace

namespace features {

void Start() {
    g_running = 1;
}

void Stop() {
    g_running = 0;
    Shutdown();
}

void Queue(unsigned bits) { g_pending.fetch_or(bits); }

// ProcessEvent fires many times per frame; the guard keeps the engine work at
// roughly 120 Hz regardless of how chatty Blueprint gets.
void Frame() {
    if (!g_running) return;

    static std::uint64_t last = 0;
    const std::uint64_t  now  = GetTickCount64();
    if (last && now - last < 8) return;

    double dt = last ? static_cast<double>(now - last) / 1000.0 : 0.016;
    last = now;
    if (dt > 0.1) dt = 0.1;

    FrameTick(dt);
}

} // namespace features
