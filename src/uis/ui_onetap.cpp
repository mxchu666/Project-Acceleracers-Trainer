// onetap ui remake (imGUIS/extracted/onetap). The ui\ tree is vendored
// byte-identical; this adapter builds its fonts, runs remake::Draw(UiState&)
// and syncs the named UiState booleans against the trainer config.

#include "ui_packs.hpp"
#include "ui_assets.hpp"
#include "../features.hpp"

#include <imgui.h>

#include "vendor/onetap/ui/api/draw.hpp"
#include "vendor/onetap/ui/state/ui_state.hpp"

#include <filesystem>
#include <string>

namespace {

remake::UiState g_state{};

std::string FirstExisting(const std::wstring& a, const std::wstring& b) {
    if (!a.empty() && std::filesystem::exists(a)) {
        char narrow[MAX_PATH * 2];
        const int n = WideCharToMultiByte(CP_ACP, 0, a.c_str(), -1,
                                          narrow, sizeof(narrow), nullptr, nullptr);
        if (n > 0) return std::string(narrow, narrow + n - 1);
    }
    if (!b.empty() && std::filesystem::exists(b)) {
        char narrow[MAX_PATH * 2];
        const int n = WideCharToMultiByte(CP_ACP, 0, b.c_str(), -1,
                                          narrow, sizeof(narrow), nullptr, nullptr);
        if (n > 0) return std::string(narrow, narrow + n - 1);
    }
    return {};
}

void SyncIn() {
    // Named bindings (documented in the README):
    g_state.auto_bhop          = g_cfg.unlimitedNitrox;
    g_state.auto_strafe        = g_cfg.unlimitedJumpJets;
    g_state.no_recoil          = g_cfg.perfectHandling;
    g_state.rage_enable        = g_cfg.noclip;
    g_state.rage_silent        = g_cfg.godMode;
    g_state.visuals_no_smoke   = g_cfg.hydroWater;
    g_state.visuals_no_flash   = g_cfg.noSandStall;
    g_state.hitchance_enabled  = g_cfg.accelechargerBoost;
    g_state.fake_lag           = g_cfg.engineBoost;
    g_state.fov                = g_cfg.accelechargerUses > 100 ? 100 : g_cfg.accelechargerUses;
    g_state.menu_open          = true;
}

void SyncOut() {
    g_cfg.unlimitedNitrox    = g_state.auto_bhop;
    g_cfg.unlimitedJumpJets  = g_state.auto_strafe;
    g_cfg.perfectHandling    = g_state.no_recoil;
    g_cfg.noclip             = g_state.rage_enable;
    g_cfg.godMode            = g_state.rage_silent;
    g_cfg.hydroWater         = g_state.visuals_no_smoke;
    g_cfg.noSandStall        = g_state.visuals_no_flash;
    g_cfg.accelechargerBoost = g_state.hitchance_enabled;
    g_cfg.engineBoost        = g_state.fake_lag;
    g_cfg.accelechargerUses  = g_state.fov > 0 ? g_state.fov : 1;

    // one-shot buttons: the pack sets the flag, the trainer queues the bit
    if (g_state.save_clicked) {
        g_state.save_clicked = false;
        features::Queue(ACT_SaveProgress);
    }
    if (g_state.unlock_inventory) {
        g_state.unlock_inventory = false;
        features::Queue(ACT_UnlockCars);
    }
}

} // namespace

namespace uionetap {

void BuildFonts() {
    // onetap main.cpp 140-190 with the trainer's stb rasterizer (the pack uses
    // the FreeType monochrome loader; without FreeType the stb fallback with
    // PixelSnapH keeps the pixel look).
    const std::string tahoma_path =
        FirstExisting(ui::DllRelativePathW(L"assets\\onetap\\fs-tahoma-8px.otf"),
                      L"C:\\Windows\\Fonts\\tahoma.ttf");
    const std::string pixel_path =
        FirstExisting(ui::DllRelativePathW(L"assets\\onetap\\smallest_pixel-7.ttf"), L"");
    const std::string icon_path =
        FirstExisting(ui::DllRelativePathW(L"assets\\onetap\\csgo_icons.ttf"), L"");

    ImGuiIO& io = ImGui::GetIO();

    ImFontConfig font_cfg{};
    font_cfg.OversampleH         = 1;
    font_cfg.OversampleV         = 1;
    font_cfg.RasterizerMultiply  = 1.0f;
    font_cfg.PixelSnapH          = true;

    ImFont* tahoma = tahoma_path.empty() ? nullptr
        : io.Fonts->AddFontFromFileTTF(tahoma_path.c_str(), 13.0f, &font_cfg,
                                       io.Fonts->GetGlyphRangesCyrillic());

    ImFontConfig brand_cfg{};
    brand_cfg.OversampleH        = 1;
    brand_cfg.OversampleV        = 1;
    brand_cfg.RasterizerMultiply = 1.0f;
    brand_cfg.PixelSnapH         = true;

    ImFont* brand_font = tahoma_path.empty() ? nullptr
        : io.Fonts->AddFontFromFileTTF(tahoma_path.c_str(), 15.0f, &brand_cfg,
                                       io.Fonts->GetGlyphRangesCyrillic());
    ImFont* esp_font   = pixel_path.empty() ? nullptr
        : io.Fonts->AddFontFromFileTTF(pixel_path.c_str(), 11.0f, &brand_cfg,
                                       io.Fonts->GetGlyphRangesCyrillic());
    static const ImWchar icon_ranges[] = {0xE000, 0xF8FF, 0};
    ImFont* weapon_icon_font = icon_path.empty() ? nullptr
        : io.Fonts->AddFontFromFileTTF(icon_path.c_str(), 14.0f, &font_cfg, icon_ranges);

    if (!tahoma) tahoma = io.Fonts->AddFontDefault();

    g_state.brand_font        = brand_font ? brand_font : tahoma;
    g_state.esp_font          = esp_font ? esp_font : tahoma;
    g_state.weapon_icon_font  = weapon_icon_font;
}

void Draw() {
    SyncIn();

    remake::Draw(g_state);

    SyncOut();
}

} // namespace uionetap
