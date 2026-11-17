// neverlose.cc ImGui - CS2 pack (imGUIS/extracted/neverlose_rar).
// gui.{hpp,cpp}, color_t.hpp, hashes.hpp, bytes.hpp are byte-identical to the
// pack. The menu frame below is the pack's example main.cpp 135-290 ported to
// D3D11 injection: same widgets, same statics, trainer features bound to the
// demo state arrays.

#define IMGUI_DEFINE_MATH_OPERATORS

#include "ui_packs.hpp"
#include "ui_assets.hpp"
#include "../features.hpp"

#include <imgui.h>
#include <imgui_internal.h>

// Pack font + image blobs. bytes.hpp array names collide with the Legacy
// pack's, so it is kept inside an anonymous namespace in this TU only.
namespace {
#include "vendor/nl_cs2/bytes.hpp"
}

#include "vendor/nl_cs2/gui.hpp"
#include "vendor/nl_cs2/hashes.hpp"

using namespace ImGui;

// main.cpp 21-22
#define ALPHA    ( ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoBorder )

namespace {

ID3D11ShaderResourceView* g_avatar = nullptr;

void EnsureAssets() {
    if (g_avatar) return;
    // The pack example draws the esliboganet avatar; it ships inside bytes.hpp.
    g_avatar = ui::CreateTextureFromMemory(ui::D3DDevice(), esliboganet, sizeof esliboganet);
}

} // namespace

namespace uinlcs2 {

void BuildFonts() {
    // main.cpp 82-89, verbatim: [0] = museo500@14 with FontAwesome merged,
    // [1] = museo900@28. The frame reaches them through io.Fonts->Fonts[n].
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromMemoryTTF(museo500_binary, sizeof museo500_binary, 14);
    static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode   = true;
    icons_config.PixelSnapH  = true;
    io.Fonts->AddFontFromMemoryTTF(&font_awesome_binary, sizeof font_awesome_binary, 13,
                                   &icons_config, icon_ranges);

    io.Fonts->AddFontFromMemoryTTF(museo900_binary, sizeof museo900_binary, 28);
}

void Draw() {
    EnsureAssets();

    ImGuiIO& io = ImGui::GetIO();

    static bool bools[50]{};
    static int  ints[50]{}, combo = 0;
    vector<const char*> items = {"Option", "Option 1", "Option 2", "Option 3", "Option 4", "Option 5", ICON_FA_AXE_BATTLE " Option 6", "Option 7", "Option 8", "Option 9"};
    static char buf[64];

    static float color[4] = {1.f, 1.f, 1.f, 1.f};

    // Trainer bindings ride the demo state: the frame keeps its exact widgets,
    // the arrays sync in and out of g_cfg around it.
    bools[1]  = g_cfg.unlimitedNitrox;
    bools[2]  = g_cfg.unlimitedJumpJets;
    bools[3]  = g_cfg.perfectHandling;
    bools[4]  = g_cfg.noclip;
    bools[5]  = g_cfg.godMode;
    bools[6]  = g_cfg.hydroWater;
    bools[7]  = g_cfg.noSandStall;
    bools[8]  = g_cfg.accelechargerBoost;
    bools[9]  = g_cfg.engineBoost;
    ints[0]   = g_cfg.accelechargerUses > 100 ? 100 : g_cfg.accelechargerUses;

    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    ImGui::Begin("Hello, world!", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground); {

        auto window = GetCurrentWindow();
        auto draw   = window->DrawList;
        auto pos    = window->Pos;
        auto size   = window->Size;
        auto style  = GetStyle();

        gui.m_anim = ImLerp(gui.m_anim, 1.f, 0.045f);

        SetWindowSize(ImVec2(690, 500));

        draw->AddText(
            io.Fonts->Fonts[1], io.Fonts->Fonts[1]->FontSize, pos + ImVec2(170 / 2 - io.Fonts->Fonts[1]->CalcTextSizeA(io.Fonts->Fonts[1]->FontSize, FLT_MAX, 0, "NEVERLOSE").x / 2 + 1, 20), gui.accent_color.to_im_color(), "NEVERLOSE");
        draw->AddText(
            io.Fonts->Fonts[1], io.Fonts->Fonts[1]->FontSize, pos + ImVec2(170 / 2 - io.Fonts->Fonts[1]->CalcTextSizeA(io.Fonts->Fonts[1]->FontSize, FLT_MAX, 0, "NEVERLOSE").x / 2, 20), GetColorU32(ImGuiCol_Text), "NEVERLOSE");

        draw->AddLine(pos + ImVec2(0, size.y - 50), pos + ImVec2(170, size.y - 50), GetColorU32(ImGuiCol_WindowBg, 0.5f));
        if (g_avatar)
            draw->AddImageRounded(g_avatar, pos + ImVec2(15, size.y - 40), pos + ImVec2(45, size.y - 10), ImVec2(0, 0), ImVec2(1, 1), ImColor(1.f, 1.f, 1.f, 1.f), 100);
        draw->AddText(pos + ImVec2(50, size.y - 40), gui.text.to_im_color(), "evrope");
        draw->AddText(pos + ImVec2(50, size.y - 25), gui.text_disabled.to_im_color(), "Till:");
        draw->AddText(pos + ImVec2(50 + CalcTextSize("Till: ").x, size.y - 25), gui.accent_color.to_im_color(), "Lifetime");

        SetCursorPos(ImVec2(10, 70));
        BeginChild("##tabs", ImVec2(150, size.y - 120));

        gui.group_title("Aimbot");
        if (gui.tab(ICON_FA_CROSSHAIRS, "Ragebot", gui.m_tab == 0) && gui.m_tab != 0)
            gui.m_tab = 0, gui.m_anim = 0.f;

        if (gui.tab(ICON_FA_GHOST, "Anti-aim", gui.m_tab == 1) && gui.m_tab != 1)
            gui.m_tab = 1, gui.m_anim = 0.f;

        if (gui.tab(ICON_FA_MOUSE, "Legitbot", gui.m_tab == 2) && gui.m_tab != 2)
            gui.m_tab = 2, gui.m_anim = 0.f;

        Spacing(), Spacing(), Spacing();

        gui.group_title("Visuals");
        if (gui.tab(ICON_FA_USER, "Players", gui.m_tab == 3) && gui.m_tab != 3)
            gui.m_tab = 3, gui.m_anim = 0.f;

        if (gui.tab(ICON_FA_PALLET, "World", gui.m_tab == 4) && gui.m_tab != 4)
            gui.m_tab = 4, gui.m_anim = 0.f;

        Spacing(), Spacing(), Spacing();

        gui.group_title("Miscellaneous");
        if (gui.tab(ICON_FA_HAMMER, "Main", gui.m_tab == 5) && gui.m_tab != 5)
            gui.m_tab = 5, gui.m_anim = 0.f;

        if (gui.tab(ICON_FA_CODE, "Scripts", gui.m_tab == 6) && gui.m_tab != 6)
            gui.m_tab = 6, gui.m_anim = 0.f;

        EndChild();

        SetCursorPos(ImVec2(190, 20));
        Button(ICON_FA_SAVE " Save", ImVec2(100, 25));

        PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

        SetCursorPos(ImVec2(300, 20));
        BeginChild("##subtabs", ImVec2(240, 25));

        GetWindowDrawList()->AddRectFilled(GetWindowPos(), GetWindowPos() + GetWindowSize(), gui.button.to_im_color(), 4);
        GetWindowDrawList()->AddRect(GetWindowPos(), GetWindowPos() + GetWindowSize(), gui.border.to_im_color(), 4);

        for (int i = 0; i < gui.rage_subtabs.size(); ++i) {
            if (gui.subtab(gui.rage_subtabs.at(i), gui.m_rage_subtab == i, gui.rage_subtabs.size(), i == 0 ? ImDrawFlags_RoundCornersLeft : i == gui.rage_subtabs.size() ? ImDrawFlags_RoundCornersRight : 0) && gui.m_rage_subtab != i)
                gui.m_rage_subtab = i, gui.m_anim = 0.f;

            if (i != gui.rage_subtabs.size() - 1)
                SameLine();
        }

        EndChild();

        PopStyleVar();

        PushStyleVar(ImGuiStyleVar_Alpha, gui.m_anim);
        PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 8));

        SetCursorPos(ImVec2(185, 81 - (5 * gui.m_anim)));
        BeginChild("##childs", ImVec2(size.x - 200, size.y - 96));

        switch (gui.m_tab) {

        case 0:

            gui.group_box(ICON_FA_FEATHER " Feather", ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2, 210)); {

                Checkbox("Skoro t'ma otstypit " ICON_FA_DAGGER, &bools[0]);
                SliderInt("Slider", &ints[0], 0, 100, "%d%%");
                Combo("Combo", &combo, items.data(), items.size());
                InputText("Input", buf, sizeof buf);
                ColorEdit4("Color", color, ALPHA);
                Button("Button", ImVec2(GetWindowWidth(), 25));

            } gui.end_group_box();

            SameLine();

            gui.group_box(ICON_FA_CROWN " Crown", ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2, GetWindowHeight())); {

                for (int i = 1; i < 50; ++i) {
                    Checkbox(std::to_string(i).c_str(), &bools[i]);

                    if (i != 49)
                        Separator();
                }

            } gui.end_group_box();

            break;

        case 1:

            gui.group_box(ICON_FA_BABY " Baby", ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2, GetWindowHeight() / 2 - GetStyle().ItemSpacing.y / 2)); {

            } gui.end_group_box();

            gui.group_box(ICON_FA_AD " Ad", ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2, GetWindowHeight() / 2 - GetStyle().ItemSpacing.y / 2)); {

            } gui.end_group_box();

            SameLine(), SetCursorPosY(0);

            gui.group_box("Non icon name", ImVec2(GetWindowWidth() / 2 - GetStyle().ItemSpacing.x / 2, GetWindowHeight())); {

            } gui.end_group_box();

            break;

        case 2:

            break;
        }

        EndChild();

        PopStyleVar(2);

    } ImGui::End();

    PopStyleVar();

    // write-back
    g_cfg.unlimitedNitrox    = bools[1];
    g_cfg.unlimitedJumpJets  = bools[2];
    g_cfg.perfectHandling    = bools[3];
    g_cfg.noclip             = bools[4];
    g_cfg.godMode            = bools[5];
    g_cfg.hydroWater         = bools[6];
    g_cfg.noSandStall        = bools[7];
    g_cfg.accelechargerBoost = bools[8];
    g_cfg.engineBoost        = bools[9];
    g_cfg.accelechargerUses  = ints[0] > 0 ? ints[0] : 1;
}

} // namespace uinlcs2
