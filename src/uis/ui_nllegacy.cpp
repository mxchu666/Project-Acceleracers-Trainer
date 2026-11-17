// neverlose.cc ImGui - CSGO:Legacy pack (imGUIS/extracted/neverlose).
// neverlose_menu.{hpp,cpp}, blur.*, hashes.hpp, bytes.hpp are byte-identical
// to the pack, with one appended accessor block at EOF of neverlose_menu.cpp
// exposing the file-static State so the trainer can bind its features.

#include "ui_packs.hpp"
#include "ui_assets.hpp"
#include "../features.hpp"

#include <imgui.h>
#include <imgui_internal.h>

// Pack font + image blobs (array names collide with the CS2 pack's, so they
// stay inside an anonymous namespace in this TU only).
namespace {
#include "vendor/nl_legacy/bytes.hpp"
}

#include "vendor/nl_legacy/neverlose_menu.hpp"
#include "vendor/nl_legacy/hashes.hpp"

#include <d3d11.h>

// Accessors appended at EOF of the vendored neverlose_menu.cpp (same TU).
namespace nllegacy {
bool*  TogglePtr(int id);
int*   SelectPtr(int id);
float* SliderPtr(int id);
bool*  SliderInitPtr(int id);
}

namespace {

ID3D11ShaderResourceView* g_avatar = nullptr;

void EnsureAssets() {
    if (g_avatar) return;
    g_avatar = ui::CreateTextureFromMemory(ui::D3DDevice(), esliboganet, sizeof esliboganet);
}

// Legacy pack ids: control_id + page * 24, Rage is page 0, Legit page 1.
enum LegacyToggle {
    LT_Enabled            = 0,   // Rage: Enabled
    LT_SilentAim          = 1,   // -> unlimitedNitrox
    LT_AutomaticFire      = 2,   // -> unlimitedJumpJets
    LT_AimThroughWalls    = 3,   // -> perfectHandling
    LT_History            = 6,   // -> noclip
    LT_DuckPeek           = 9,   // -> godMode
    LT_QuickPeek          = 10,  // -> hydroWater
    LT_QuickStop          = 16,  // -> noSandStall
    LT_LegitEnable        = 24,  // Legit: Enabled -> accelechargerBoost
    LT_LegitAimbot        = 25,  // -> engineBoost
};
enum LegacySlider {
    LS_FieldOfView        = 5,   // Rage: Field of View -> accelechargerUses (0-100)
};

void SyncIn() {
    *nllegacy::TogglePtr(LT_SilentAim)         = g_cfg.unlimitedNitrox;
    *nllegacy::TogglePtr(LT_AutomaticFire)     = g_cfg.unlimitedJumpJets;
    *nllegacy::TogglePtr(LT_AimThroughWalls)   = g_cfg.perfectHandling;
    *nllegacy::TogglePtr(LT_History)           = g_cfg.noclip;
    *nllegacy::TogglePtr(LT_DuckPeek)          = g_cfg.godMode;
    *nllegacy::TogglePtr(LT_QuickPeek)         = g_cfg.hydroWater;
    *nllegacy::TogglePtr(LT_QuickStop)         = g_cfg.noSandStall;
    *nllegacy::TogglePtr(LT_LegitEnable)       = g_cfg.accelechargerBoost;
    *nllegacy::TogglePtr(LT_LegitAimbot)       = g_cfg.engineBoost;
    *nllegacy::SliderPtr(LS_FieldOfView) =
        static_cast<float>(g_cfg.accelechargerUses > 100 ? 100 : g_cfg.accelechargerUses) / 100.f;
}

void SyncOut() {
    g_cfg.unlimitedNitrox    = *nllegacy::TogglePtr(LT_SilentAim);
    g_cfg.unlimitedJumpJets  = *nllegacy::TogglePtr(LT_AutomaticFire);
    g_cfg.perfectHandling    = *nllegacy::TogglePtr(LT_AimThroughWalls);
    g_cfg.noclip             = *nllegacy::TogglePtr(LT_History);
    g_cfg.godMode            = *nllegacy::TogglePtr(LT_DuckPeek);
    g_cfg.hydroWater         = *nllegacy::TogglePtr(LT_QuickPeek);
    g_cfg.noSandStall        = *nllegacy::TogglePtr(LT_QuickStop);
    g_cfg.accelechargerBoost = *nllegacy::TogglePtr(LT_LegitEnable);
    g_cfg.engineBoost        = *nllegacy::TogglePtr(LT_LegitAimbot);
    const float fov          = *nllegacy::SliderPtr(LS_FieldOfView);
    g_cfg.accelechargerUses  = static_cast<int>(fov * 100.f + 0.5f) > 0
                                   ? static_cast<int>(fov * 100.f + 0.5f) : 1;
}

} // namespace

namespace uinllegacy {

void BuildFonts() {
    // zip main.cpp 104-123, minus the PingFang Chinese merge (no CJK glyphs in
    // the trainer's labels): [0] = SSTMedium@15 (+ FontAwesome merged), [1] =
    // SSTBold@16. Vendored code reaches them by index.
    const std::string sstMedium = ui::DllRelativePath("assets\\nl_legacy\\SSTMedium.TTF");
    const std::string sstBold   = ui::DllRelativePath("assets\\nl_legacy\\SSTBold.TTF");
    const std::string faSolid   = ui::DllRelativePath("assets\\nl_legacy\\fa-solid-900.ttf");

    ImGuiIO& io = ImGui::GetIO();

    ImFontConfig text_config;
    text_config.OversampleH         = 3;
    text_config.PixelSnapH          = true;
    text_config.RasterizerMultiply  = 1.0f;

    if (!sstMedium.empty())
        io.Fonts->AddFontFromFileTTF(sstMedium.c_str(), 15.0f, &text_config);
    else
        io.Fonts->AddFontDefault();

    static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode   = true;
    icons_config.PixelSnapH  = true;
    if (!faSolid.empty())
        io.Fonts->AddFontFromFileTTF(faSolid.c_str(), 14.0f, &icons_config, icon_ranges);

    if (!sstBold.empty())
        io.Fonts->AddFontFromFileTTF(sstBold.c_str(), 16.0f, &text_config);
}

void Draw() {
    EnsureAssets();
    SyncIn();

    // reveal = 1.0: the pack's 0.48 s intro slide plays on the first frames.
    RenderNeverloseMenu(g_avatar, 1.0f);

    SyncOut();
}

} // namespace uinllegacy
