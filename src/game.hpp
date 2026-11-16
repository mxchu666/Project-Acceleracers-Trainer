#pragma once
// Project Acceleracers - game layer. Every offset below is copied from
// "sdk latest\4.27.2-18319896+++UE4+Release-4.27-ProjectAcceleracer\CppSDK\SDK".

#include "ue.hpp"
#include "realms.hpp"
#include <cctype>
#include <cmath>
#include <mutex>

namespace acc {

namespace off {
// ACar_Base_C  (0x07C0)
constexpr std::size_t Car_AccelechargerUses       = 0x414;
constexpr std::size_t Car_MaxNitrox               = 0x418;
constexpr std::size_t Car_Nitrox                  = 0x41C;
constexpr std::size_t Car_NitroxForce             = 0x420;
constexpr std::size_t Car_JJForce                 = 0x428;
constexpr std::size_t Car_EngineHealth            = 0x438;
constexpr std::size_t Car_NitroxCooldown          = 0x444;
constexpr std::size_t Car_isAI                    = 0x44C;
constexpr std::size_t Car_NitroxCurrentCooldown   = 0x520;
constexpr std::size_t Car_StunTime                = 0x524;
constexpr std::size_t Car_EngineBlown             = 0x54A;
constexpr std::size_t Car_AbilityInUse            = 0x54E;
constexpr std::size_t Car_AbilityCooldown         = 0x54F;
constexpr std::size_t Car_AccelechargerActive     = 0x55B;
constexpr std::size_t Car_VehicleMovement         = 0x288;
constexpr std::size_t Car_CurrentMap              = 0x77C;  // Realms::ERealms

// Hazard gates and physics state (Car_Base_C). The canGet* bytes are the
// per-effect switches every realm hazard reads before applying anything.
constexpr std::size_t Car_DamageRate              = 0x52C;
constexpr std::size_t Car_ThrottleInput           = 0x538;
constexpr std::size_t Car_OnGround                = 0x548;
constexpr std::size_t Car_isGettingDamaged        = 0x555;
constexpr std::size_t Car_canGetDamaged           = 0x556;
constexpr std::size_t Car_canGetShocked           = 0x557;
constexpr std::size_t Car_canGetWindPushed        = 0x558;
constexpr std::size_t Car_canGetWaterSlowed       = 0x559;
constexpr std::size_t Car_canGetLowRevDamage      = 0x561;
constexpr std::size_t Car_WindForce               = 0x570;
constexpr std::size_t Car_VehicleVelocity         = 0x57C;

// PhysXVehicles
constexpr std::size_t VM_Wheels                   = 0x1A8;  // TArray<UVehicleWheel*>
constexpr std::size_t Wheel_TireConfig            = 0x60;
constexpr std::size_t Tire_FrictionScale          = 0x30;

// UMyGameInstance_C (0x08C0)
constexpr std::size_t GI_UnlockedCars             = 0x1B8;
constexpr std::size_t GI_SaveGameRef              = 0x340;
constexpr std::size_t GI_SaveGameName             = 0x478;
constexpr std::size_t GI_RealmRecordTimes         = 0x528;
constexpr std::size_t GI_UnlockedAccelechargers   = 0x728;
constexpr std::size_t GI_IsInRealm                = 0x869;

// UMyPlayerSaveGame_C (0x0218)
constexpr std::size_t SG_UnlockedCars             = 0x160;
constexpr std::size_t SG_RealmRecords             = 0x1B0;
constexpr std::size_t SG_UnlockedAccelechargers   = 0x200;

// APortal_C
constexpr std::size_t Portal_LevelIn              = 0x250;  // FName
constexpr std::size_t Portal_LevelOut             = 0x258;  // FName, realm package this portal leads to
constexpr std::size_t Portal_Acceledrome          = 0x261;
constexpr std::size_t Portal_Mesh                 = 0x240;  // UStaticMeshComponent

// Enums
constexpr int CarCount          = 113;  // ECarList::CarList_MAX
constexpr int AccelechargerCount = 17;  // EAccelechargers::Accelechargers_MAX
constexpr int RealmCount        = 47;   // Realms::ERealms::Realms_MAX

// FTimespan
constexpr ue::i64 TicksPerSecond = 10000000;
} // namespace off

enum class Feature : unsigned {
    UnlimitedNitrox  = 1u << 0,
    UnlimitedJumpJets= 1u << 1,
    PerfectHandling  = 1u << 2,
    Noclip           = 1u << 3,
};

struct CarStats {
    float nitrox       = 0.f;
    float maxNitrox    = 0.f;
    float nitroxForce  = 0.f;
    int   uses         = 0;
    bool  valid        = false;
};

inline std::uintptr_t LocalCar() {
    const std::uintptr_t pawn = ue::PlayerPawn();
    if (!pawn || !ue::ClassOf(pawn)) return 0;
    // Root is AWheeledVehicle; the BP class sits on top. Reject anything that
    // is not the car (spectator pawns etc.) by probing VehicleMovement.
    std::uintptr_t vm = 0;
    if (!ue::Read(pawn + off::Car_VehicleMovement, vm) || !ue::Plausible(vm)) return 0;
    return pawn;
}

inline CarStats ReadCarStats(std::uintptr_t car) {
    CarStats s{};
    if (!car) return s;
    ue::Read(car + off::Car_Nitrox, s.nitrox);
    ue::Read(car + off::Car_MaxNitrox, s.maxNitrox);
    ue::Read(car + off::Car_NitroxForce, s.nitroxForce);
    ue::Read(car + off::Car_AccelechargerUses, s.uses);
    s.valid = s.maxNitrox > 0.f && s.maxNitrox < 1.0e6f;
    return s;
}

inline bool InRealm() {
    bool v = false;
    const std::uintptr_t gi = ue::GameInstance();
    if (!gi || !ue::Read(gi + off::GI_IsInRealm, v)) return false;
    return v;
}

// ---------------------------------------------------------------- toggles

inline void ApplyUnlimitedNitrox(std::uintptr_t car, bool on) {
    if (!car) return;
    float maxNitrox = 0.f;
    if (!ue::Read(car + off::Car_MaxNitrox, maxNitrox) || maxNitrox <= 0.f) return;
    if (on) {
        ue::Write(car + off::Car_Nitrox, maxNitrox);
        ue::Write(car + off::Car_NitroxCurrentCooldown, 0.f);
    }
}

inline void ApplyUnlimitedJumpJets(std::uintptr_t car, bool on) {
    if (!car) return;
    if (!on) return;
    int32_t uses = 99;
    ue::Write(car + off::Car_AccelechargerUses, uses);
    ue::Write(car + off::Car_NitroxCurrentCooldown, 0.f);
    ue::Write(car + off::Car_StunTime, 0.f);
    bool cd = false;
    ue::Write(car + off::Car_AbilityCooldown, cd);
}

// Charges left on the equipped Accelecharger (Car_Base_C::AccelechargerUses).
inline void ApplyAccelechargerUses(std::uintptr_t car, int uses) {
    if (!car || uses <= 0) return;
    int32_t v = uses;
    ue::Write(car + off::Car_AccelechargerUses, v);
    ue::Write(car + off::Car_NitroxCurrentCooldown, 0.f);
}

// Engine health points. EngineBlown is the flag the game sets once health hits
// zero, so it has to be cleared or the car stays dead no matter the health.
inline void ApplyEngineHealth(std::uintptr_t car, float hp) {
    if (!car || hp <= 0.f) return;
    ue::Write(car + off::Car_EngineHealth, hp);
    bool blown = false;
    ue::Write(car + off::Car_EngineBlown, blown);
}

// TireConfig is a shared UDataAsset: patching one wheel grips every car that
// runs that tire. Originals are kept so the toggle is reversible.
inline void ApplyPerfectHandling(std::uintptr_t car, bool on, float grip) {
    static constexpr int kMaxWheels = 12;
    static float  saved[kMaxWheels];
    static std::uintptr_t savedCfg[kMaxWheels];
    static int    savedCount = 0;
    static bool   applied    = false;

    if (!on) {
        for (int i = 0; i < savedCount; ++i) ue::Write(savedCfg[i] + off::Tire_FrictionScale, saved[i]);
        savedCount = applied = false;
        return;
    }

    std::uintptr_t vm = 0;
    if (!ue::Read(car + off::Car_VehicleMovement, vm) || !ue::Plausible(vm)) return;

    std::uintptr_t wheels = 0;
    int32_t        count  = 0;
    if (!ue::Read(vm + off::VM_Wheels, wheels) || !ue::Read(vm + off::VM_Wheels + 8, count)) return;
    if (!ue::Plausible(wheels) || count <= 0 || count > kMaxWheels) return;

    for (int i = 0; i < count; ++i) {
        std::uintptr_t wheel = 0;
        if (!ue::Read(wheels + static_cast<std::uintptr_t>(i) * 8, wheel) || !ue::Plausible(wheel)) continue;
        std::uintptr_t cfg = 0;
        if (!ue::Read(wheel + off::Wheel_TireConfig, cfg) || !ue::Plausible(cfg)) continue;

        if (!applied) {
            float orig = 1.f;
            if (ue::Read(cfg + off::Tire_FrictionScale, orig) && savedCount < kMaxWheels) {
                savedCfg[savedCount] = cfg;
                saved[savedCount]     = orig;
                ++savedCount;
            }
        }
        ue::Write(cfg + off::Tire_FrictionScale, grip);
    }
    applied = true;
}

// ---------------------------------------------------------------- hazards

// One canGet* byte, saved on first latch so switching the toggle off puts the
// car back the way the game had it. Assumes one car at a time, which a single
// player pawn guarantees.
struct BoolGate {
    std::uintptr_t car      = 0;
    std::uint8_t   original = 1;
    bool           armed    = false;
};

inline void SetBoolGate(BoolGate& g, std::uintptr_t car, std::size_t fieldOff, bool latch) {
    if (!latch) {
        // Only restore when the saved car is still the live one; after a level
        // travel the old pointer may have been reused by an unrelated object.
        if (g.armed && g.car && g.car == car) ue::Write(g.car + fieldOff, g.original);
        g = BoolGate{};
        return;
    }
    if (!car) return;
    if (!g.armed || g.car != car) {
        std::uint8_t v = 1;
        if (!ue::Read(car + fieldOff, v)) return;
        g.original = v;
        g.car      = car;
        g.armed    = true;
    }
    std::uint8_t off = 0;
    ue::Write(car + fieldOff, off);
}

// Storm Realm lightning: the strike path checks canGetShocked, then canGetDamaged,
// then accumulates StunTime/DamageRate. All four are shut, so a bolt neither hurts
// nor locks the car. Wind push is included because the same realm uses it to
// shove the car off the track and it would fight noclip.
inline void ApplyGodMode(std::uintptr_t car, bool on) {
    static BoolGate dmg, shock, wind, lowRev;
    static float     savedRate = 0.f;
    static bool      rateSaved = false;

    const std::uintptr_t prevCar = dmg.car;

    SetBoolGate(dmg,    car, off::Car_canGetDamaged,      on);
    SetBoolGate(shock,  car, off::Car_canGetShocked,      on);
    SetBoolGate(wind,   car, off::Car_canGetWindPushed,   on);
    SetBoolGate(lowRev, car, off::Car_canGetLowRevDamage, on);

    if (!on) {
        if (rateSaved && prevCar && prevCar == car) {
            // DamageRate is a constant the Blueprint multiplies hits by, so it is
            // restored rather than left at zero.
            float rate = savedRate;
            ue::Write(prevCar + off::Car_DamageRate, rate);
        }
        rateSaved = false;
        return;
    }
    if (!car) return;

    if (!rateSaved) {
        float rate = 1.f;
        if (ue::Read(car + off::Car_DamageRate, rate)) {
            savedRate = rate;
            rateSaved = true;
        }
    }

    float        zero = 0.f;
    std::uint8_t no   = 0;
    ue::Write(car + off::Car_DamageRate, zero);
    ue::Write(car + off::Car_StunTime, zero);
    ue::Write(car + off::Car_isGettingDamaged, no);
    ue::Write(car + off::Car_WindForce, ue::FVector{0.f, 0.f, 0.f});
}

// Hydro tires: canGetWaterSlowed is the gate the water volume checks before it
// drags the car, so clearing it keeps full speed through Water Realm pools.
inline void ApplyHydroWater(std::uintptr_t car, bool on) {
    static BoolGate water;
    SetBoolGate(water, car, off::Car_canGetWaterSlowed, on);
}

// Spiked tires bite into the Desert sand and the car stalls while the throttle is
// still down. VehicleVelocity is only the Blueprint's cached mirror, so the fix
// has to go through the rigid body: horizontal velocity is lifted back to a floor.
// Only fires while throttling, grounded, and already slower than the floor, so it
// is an unstick and not a speed hack.
inline void ApplyNoSandStall(std::uintptr_t car, float floorSpeed) {
    if (!car || floorSpeed <= 0.f) return;

    float throttle = 0.f;
    if (!ue::Read(car + off::Car_ThrottleInput, throttle) || throttle < 0.1f) return;

    bool grounded = false;
    if (!ue::Read(car + off::Car_OnGround, grounded) || !grounded) return;

    ue::FVector v{};
    if (!ue::Read(car + off::Car_VehicleVelocity, v)) return;
    if (std::sqrt(v.x * v.x + v.y * v.y) >= floorSpeed) return;

    // The car's own heading, not the camera: a stalled car must not launch
    // sideways when the camera happens to be turned.
    alignas(16) std::uint8_t fbuf[0x10] = {};
    if (!ue::CallByName(car, "GetActorForwardVector", fbuf)) return;

    ue::FVector fwd{};
    std::memcpy(&fwd, fbuf, sizeof(fwd));
    const float len = std::sqrt(fwd.x * fwd.x + fwd.y * fwd.y);
    if (len < 0.01f) return;

    std::uintptr_t root = 0;
    if (!ue::Read(car + ue::lay::AActor_RootComponent, root) || !ue::Plausible(root)) return;

    // PrimitiveComponent_SetPhysicsLinearVelocity: FVector(0x00) bAddToCurrent(0x0C)
    // pad(3) FName(0x10) -> 0x18.
    alignas(16) std::uint8_t buf[0x18] = {};
    const ue::FVector push{fwd.x / len * floorSpeed, fwd.y / len * floorSpeed, v.z};
    std::memcpy(buf, &push, sizeof(push));
    ue::CallByName(root, "SetPhysicsLinearVelocity", buf);
}

// ---------------------------------------------------------------- actors

inline bool GetActorLocation(std::uintptr_t actor, ue::FVector& out) {
    alignas(16) std::uint8_t buf[0x40] = {};
    if (!ue::CallByName(actor, "K2_GetActorLocation", buf)) return false;
    std::memcpy(&out, buf, sizeof(ue::FVector));
    return true;
}

inline bool SetActorLocation(std::uintptr_t actor, const ue::FVector& loc, bool sweep, bool teleport) {
    // Actor_K2_SetActorLocation: FVector(0x0C) bSweep(1) pad FHitResult(0x88 @0x10) bTeleport(0x98) ret(0x99)
    alignas(16) std::uint8_t buf[0x100] = {};
    std::memcpy(buf, &loc, sizeof(ue::FVector));
    buf[0x0C] = sweep ? 1 : 0;
    buf[0x98] = teleport ? 1 : 0;
    return ue::CallByName(actor, "K2_SetActorLocation", buf);
}

inline bool SetActorCollision(std::uintptr_t actor, bool enabled) {
    alignas(16) std::uint8_t buf[0x10] = {};
    buf[0] = enabled ? 1 : 0;
    return ue::CallByName(actor, "SetActorEnableCollision", buf);
}

inline bool SetRootSimulatePhysics(std::uintptr_t actor, bool simulate) {
    std::uintptr_t root = 0;
    if (!ue::Read(actor + ue::lay::AActor_RootComponent, root) || !ue::Plausible(root)) return false;
    alignas(16) std::uint8_t buf[0x10] = {};
    buf[0] = simulate ? 1 : 0;
    return ue::CallByName(root, "SetSimulatePhysics", buf);
}

inline ue::FRotator ControlRotation() {
    ue::FRotator r{};
    const std::uintptr_t pc = ue::LocalPlayerController();
    if (pc) ue::Read(pc + ue::lay::AController_ControlRotation, r);
    return r;
}

// ---------------------------------------------------------------- realm finish

// FName properties on actors: read the packed index and resolve through GNames.
inline bool ReadFNameAt(std::uintptr_t obj, std::size_t fieldOff, char* out, std::size_t cap) {
    ue::FName n{};
    if (!ue::Read(obj + fieldOff, n)) return false;
    return ue::FNameToString(n, out, cap);
}

// Case- and separator-insensitive comparison key, so "Realm_Micro_Card",
// "realm micro card" and "MicroCard" all land on the same entry.
inline void NormalizeName(const char* in, char* out, std::size_t cap) {
    std::size_t n = 0;
    if (cap == 0) return;
    for (; *in && n + 1 < cap; ++in) {
        const unsigned char c = static_cast<unsigned char>(*in);
        if (c == '_' || c == ' ' || c == '-' || c == '.') continue;
        out[n++] = static_cast<char>(std::tolower(c));
    }
    out[n] = 0;
}

// Index into kRealmPortals for a map/level name, or -1.
inline int RealmIndexForName(const char* rawName) {
    if (!rawName || !rawName[0]) return -1;

    char hay[192];
    NormalizeName(rawName, hay, sizeof(hay));
    if (!hay[0]) return -1;

    // Exact match first, then the longest substring: a name that contains another
    // realm's name ("Realm_Storm_part_2") must not win by table order.
    int         best    = -1;
    std::size_t bestLen = 0;

    for (int i = 0; i < kRealmPortalCount; ++i) {
        for (int field = 0; field < 2; ++field) {
            char key[96];
            NormalizeName(field ? kRealmPortals[i].name : kRealmPortals[i].map, key, sizeof(key));
            const std::size_t len = std::strlen(key);
            if (!len) continue;

            if (std::strcmp(key, hay) == 0) return i;
            if (len > bestLen && std::strstr(hay, key)) {
                best    = i;
                bestLen = len;
            }
        }
    }
    return best;
}

// One actor scan per world, refreshed on a timer: ClassNameIs costs an FName
// decode and a realm carries thousands of actors, so the result is cached rather
// than recomputed from the UI thread and the pin loop every frame.
struct RealmProbeData {
    std::uintptr_t world   = 0;
    std::uintptr_t portal  = 0;
    int            portals = 0;
    int            levels  = 0;
    int            actors  = 0;
    bool           hub     = false;
    int            realm   = -1;
    std::uint64_t  stamp   = 0;
};

inline RealmProbeData Probe() {
    static RealmProbeData  cache{};
    static std::mutex      lock;
    static std::uintptr_t  memoCls[256] = {};
    static bool            memoHit[256] = {};

    std::lock_guard<std::mutex> guard(lock);

    const std::uint64_t  now   = GetTickCount64();
    const std::uintptr_t world = ue::PlayWorld();
    if (cache.stamp && cache.world == world && now - cache.stamp < 500) return cache;

    cache = RealmProbeData{};
    cache.world = world;
    cache.stamp = now;
    if (!world) return cache;

    // Every actor of a class shares one UClass pointer, so one decode per class.
    std::memset(memoCls, 0, sizeof(memoCls));
    cache.levels = ue::LevelCount(world);

    std::uintptr_t found[64];
    int            count = 0;
    ue::ForEachActor(world, [&](std::uintptr_t actor) {
        ++cache.actors;
        const std::uintptr_t cls = ue::ClassOf(actor);
        if (!cls) return;
        const std::size_t slot = (cls >> 4) & 0xFF;
        if (memoCls[slot] != cls) {
            memoCls[slot] = cls;
            memoHit[slot] = ue::NameIs(cls, "Portal_C");
        }
        if (!memoHit[slot] || count >= 64) return;
        found[count++] = actor;
    });
    cache.portals = count;

    // A realm map carries one Portal_C, its exit. The Acceledrome carries one per
    // realm and flags them as its own, so a lone portal wins outright and a mixed
    // set falls back to the only portal that is not the hub's.
    std::uintptr_t portal = 0;
    if (count == 1) {
        portal = found[0];
    } else if (count > 1) {
        std::uintptr_t only = 0;
        int            hits = 0;
        for (int i = 0; i < count; ++i) {
            bool hub = false;
            if (!ue::Read(found[i] + off::Portal_Acceledrome, hub) || hub) continue;
            only = found[i];
            if (++hits > 1) break;
        }
        if (hits == 1) portal = only;
    }

    if (portal) {
        cache.portal = portal;

        bool hub = false;
        if (ue::Read(portal + off::Portal_Acceledrome, hub)) cache.hub = hub;

        char out[192] = {};
        if (!cache.hub && ReadFNameAt(portal, off::Portal_LevelOut, out, sizeof(out)))
            cache.realm = RealmIndexForName(out);
    }

    // Second source: the world's own name, then its outer chain, which carries the
    // map package name even when the object name is generic.
    char buf[192] = {};
    std::uintptr_t o = world;
    for (int depth = 0; cache.realm < 0 && o && depth < 4; ++depth) {
        if (ue::ObjectName(o, buf, sizeof(buf))) cache.realm = RealmIndexForName(buf);
        o = ue::OuterOf(o);
    }
    return cache;
}

inline std::uintptr_t FindSolePortal(bool* isHub = nullptr) {
    const RealmProbeData p = Probe();
    if (isHub) *isHub = p.hub;
    return p.portal;
}

inline int CurrentRealm() {
    return Probe().realm;
}

// The live portal actor is placed by the game itself, so prefer it over the
// dump table; the table covers frames before the actor has spawned.
inline bool RealmPortalTarget(ue::FVector& out, bool* fromTable = nullptr) {
    const RealmProbeData p = Probe();

    if (p.portal && !p.hub && GetActorLocation(p.portal, out)) {
        if (fromTable) *fromTable = false;
        return true;
    }

    if (p.realm >= 0 && kRealmPortals[p.realm].hasCoords) {
        out = ue::FVector{kRealmPortals[p.realm].x, kRealmPortals[p.realm].y, kRealmPortals[p.realm].z};
        if (fromTable) *fromTable = true;
        return true;
    }

    if (p.portal) {
        const bool ok = GetActorLocation(p.portal, out);
        if (fromTable) *fromTable = false;
        return ok;
    }

    if (fromTable) *fromTable = false;
    return false;
}

// Portal_C's Overlap handler is the finish line itself: this is the UFunction the
// game runs when the car drives through the ring. Calling it directly finishes the
// realm without depending on the car's collision producing a sweep while it is
// teleported, which is the part that differed from realm to realm.
inline bool CompleteRealm(std::uintptr_t portal, std::uintptr_t car) {
    if (!portal || !car) return false;

    // Portal_C_BndEvt__Portal_..._ComponentBeginOverlap: OverlappedComponent(0x00)
    // OtherActor(0x08) OtherComp(0x10) OtherBodyIndex(0x18) bFromSweep(0x1C)
    // SweepResult(0x20, 0x88) -> 0xA8 total. The component pointers are filled
    // because Blueprint graphs read them without a null check.
    alignas(16) std::uint8_t buf[0xA8] = {};

    std::uintptr_t overlapped = 0;
    std::uintptr_t otherComp  = 0;
    ue::Read(portal + off::Portal_Mesh, overlapped);
    ue::Read(car + ue::lay::AActor_RootComponent, otherComp);

    std::memcpy(buf + 0x00, &overlapped, sizeof(overlapped));
    std::memcpy(buf + 0x08, &car, sizeof(car));
    std::memcpy(buf + 0x10, &otherComp, sizeof(otherComp));

    constexpr const char* kOverlap =
        "BndEvt__Portal_K2Node_ComponentBoundEvent_0_ComponentBeginOverlapSignature__DelegateSignature";
    return ue::CallByName(portal, kOverlap, buf);
}

// ---------------------------------------------------------------- progression

inline void UnlockAllCars(std::uintptr_t gi) {
    if (!gi) return;
    for (int i = 0; i < off::CarCount; ++i) {
        alignas(16) std::uint8_t buf[0x10] = {};
        buf[0] = static_cast<std::uint8_t>(i);
        ue::CallByName(gi, "UnlockCar", buf);
    }
}

inline void UnlockAllAccelechargers(std::uintptr_t gi) {
    if (!gi) return;
    for (int i = 0; i < off::AccelechargerCount; ++i) {
        alignas(16) std::uint8_t buf[0x10] = {};
        buf[0] = static_cast<std::uint8_t>(i);
        ue::CallByName(gi, "UnlockAccelecharger", buf);
    }
}

inline void SetAllRealmRecords(std::uintptr_t gi, float seconds) {
    if (!gi) return;
    if (seconds < 0.01f) seconds = 0.01f;
    const ue::i64 ticks = static_cast<ue::i64>(seconds * static_cast<double>(off::TicksPerSecond));

    for (int i = 0; i < off::RealmCount; ++i) {
        // MyGameInstance_C_SaveGameTime: Realm(0x00) pad(0x07) FTimespan(0x08)
        alignas(16) std::uint8_t buf[0x20] = {};
        buf[0] = static_cast<std::uint8_t>(i);
        std::memcpy(buf + 0x08, &ticks, sizeof(ticks));
        ue::CallByName(gi, "SaveGameTime", buf);
    }
}

// Save_Game takes an FString by value. Reusing the instance's own buffer keeps
// the callee from ever owning memory we allocated.
inline bool SaveProgress(std::uintptr_t gi) {
    if (!gi) return false;
    ue::FString name{};
    if (!ue::Read(gi + off::GI_SaveGameName, name)) return false;
    if (!name.data || name.num <= 0 || name.num > 512) return false;

    alignas(16) std::uint8_t buf[0x20] = {};
    std::memcpy(buf, &name, sizeof(name));
    return ue::CallByName(gi, "Save_Game", buf);
}

} // namespace acc
