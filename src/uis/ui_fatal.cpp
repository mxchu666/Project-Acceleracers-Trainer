// project-139 / FatalUI pack (imGUIS/extracted/project139). Menu.cpp,
// FatalWidgets.{h,cpp} and the Client.h structs are vendored byte-identical;
// the pack's test-prop section of Client.h carries trainer bindings. The font
// globals and ClientBind resolvers the vendored code links against live here.

#include "ui_packs.hpp"
#include "ui_assets.hpp"
#include "../features.hpp"

#include <imgui.h>
#include <imgui_internal.h>

#include <fstream>
#include <functional> // Properties.h uses std::function without including it
#include <iterator>
#include <string>

#include "vendor/fatal/menu/Menu.h"
#include "vendor/fatal/menu/Client.h"

// ---------------------------------------------------------------------------
// Font globals the vendored Menu.cpp declares extern (Menu.cpp 8-10).
ImFont* Medium12    = nullptr;
ImFont* ExtraBold20 = nullptr;
ImFont* IconFont    = nullptr;

// ---------------------------------------------------------------------------
// ImGui 1.92 shim (see src/uis/compat/imgui.h). The vendored Fatal menu reads
// GetFontBaked()->Descent for its username layout; 1.90.9 keeps those metrics
// on ImFont itself.
namespace ImGui {

ImFontBakedShim* GetFontBaked() {
    static ImFontBakedShim shim {};
    if (ImFont* f = ImGui::GetFont()) {
        shim.Size    = f->FontSize;
        shim.Ascent  = f->Ascent;
        shim.Descent = f->Descent;
    } else {
        shim = ImFontBakedShim {};
    }
    return &shim;
}

} // namespace ImGui

namespace {

// Reads one of the pack's font blobs shipped next to the DLL. The vendored
// font_*.h declare ~8.6 MB base85 string literals, which exhaust MSVC's
// parser heap (C1060) even one per translation unit, so the text ships as a
// runtime asset and AddFontFromMemoryCompressedBase85TTF (strlen-based, it
// decodes in place) consumes it from here.
std::string LoadBase85(const char* relative) {
    const std::string path = ui::DllRelativePath(relative);
    if (path.empty()) return {};

    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file) return {};

    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

// One-shot action backing store. The Fatal menu flips these through
// TrainerBool; Draw() drains them into features::Queue() for the game thread.
bool s_unlockCars   = false;
bool s_unlockAccel  = false;
bool s_saveProgress = false;
bool s_setRecords   = false;
bool s_finishRealm  = false;

// Accelecharger charges are an int in the trainer config; the Fatal slider is
// a double property, so it writes through this float mirror.
float s_accelUses = 99.f;

} // namespace

// ---------------------------------------------------------------------------
// ClientBind resolvers (declared in vendor/fatal/menu/Client.h).
namespace Client {
namespace ClientBind {

bool* BoolTarget(int id) {
    switch (id) {
    case B_UnlimitedNitrox:    return &g_cfg.unlimitedNitrox;
    case B_UnlimitedJumpJets:  return &g_cfg.unlimitedJumpJets;
    case B_PerfectHandling:    return &g_cfg.perfectHandling;
    case B_Noclip:             return &g_cfg.noclip;
    case B_GodMode:            return &g_cfg.godMode;
    case B_HydroWater:         return &g_cfg.hydroWater;
    case B_NoSandStall:        return &g_cfg.noSandStall;
    case B_AccelechargerBoost: return &g_cfg.accelechargerBoost;
    case B_EngineBoost:        return &g_cfg.engineBoost;
    default:                   break;
    }
    static bool none = false;
    return &none;
}

bool* ActionTarget(int id) {
    switch (id) {
    case B_UnlockCars:   return &s_unlockCars;
    case B_UnlockAccel:  return &s_unlockAccel;
    case B_SaveProgress: return &s_saveProgress;
    case B_SetRecords:   return &s_setRecords;
    case B_FinishRealm:  return &s_finishRealm;
    default:             break;
    }
    static bool none = false;
    return &none;
}

float* FloatTarget(int id) {
    switch (id) {
    case F_Grip:              return &g_cfg.grip;
    case F_EngineHp:          return &g_cfg.engineHp;
    case F_NoclipSpeed:       return &g_cfg.noclipSpeed;
    case F_SandFloor:         return &g_cfg.sandFloor;
    case F_AccelechargerUses: return &s_accelUses;
    case F_RecordSeconds:     return &g_cfg.recordSeconds;
    default:                  break;
    }
    static float none = 0.f;
    return &none;
}

} // namespace ClientBind
} // namespace Client

// ---------------------------------------------------------------------------

namespace uifatal {

void BuildFonts() {
    // fatalui_test.cpp loadFonts(), verbatim sizes: Medium@14, ExtraBold@24,
    // iconfont@16 over E000-F8FF. The vendored Menu.cpp reaches them through
    // the globals above; AddFontDefault covers missing assets.
    ImGuiIO& io = ImGui::GetIO();

    const std::string medium    = LoadBase85("assets\\fatal\\font_medium.base85");
    const std::string extrabold = LoadBase85("assets\\fatal\\font_extrabold.base85");
    const std::string iconfont  = LoadBase85("assets\\fatal\\iconfont.base85");

    Medium12    = medium.empty()    ? io.Fonts->AddFontDefault()
                                    : io.Fonts->AddFontFromMemoryCompressedBase85TTF(medium.c_str(), 14);
    ExtraBold20 = extrabold.empty() ? io.Fonts->AddFontDefault()
                                    : io.Fonts->AddFontFromMemoryCompressedBase85TTF(extrabold.c_str(), 24);

    static constexpr ImWchar iconRange[] = { 0xE000, 0xF8FF, 0 };
    IconFont    = iconfont.empty()  ? io.Fonts->AddFontDefault()
                                    : io.Fonts->AddFontFromMemoryCompressedBase85TTF(iconfont.c_str(), 16, nullptr, iconRange);
}

void Draw() {
    // The pack's props capture their initial values at static-init, which can
    // run before the trainer config's - push the live values through once.
    static bool synced = false;
    if (!synced) {
        synced = true;
        Client::gripProp.set(g_cfg.grip);
        Client::engineHpProp.set(g_cfg.engineHp);
        Client::noclipSpeed.set(g_cfg.noclipSpeed);
        Client::sandFloorProp.set(g_cfg.sandFloor);
        Client::accelUses.set(static_cast<double>(g_cfg.accelechargerUses));
        Client::recordSecs.set(g_cfg.recordSeconds);
    }

    // Drain one-shot actions: the menu set the bool, the game thread runs it.
    if (s_unlockCars)   { s_unlockCars   = false; features::Queue(ACT_UnlockCars); }
    if (s_unlockAccel)  { s_unlockAccel  = false; features::Queue(ACT_UnlockAccel); }
    if (s_saveProgress) { s_saveProgress = false; features::Queue(ACT_SaveProgress); }
    if (s_setRecords)   { s_setRecords   = false; features::Queue(ACT_SetRecords); }
    if (s_finishRealm)  { s_finishRealm  = false; features::Queue(ACT_FinishRealm); }

    Fatal::Menu::draw();

    g_cfg.accelechargerUses = static_cast<int>(s_accelUses);
}

} // namespace uifatal
