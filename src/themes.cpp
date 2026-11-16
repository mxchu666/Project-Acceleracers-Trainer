#include "themes.hpp"

namespace theme {

namespace {
int g_current = 0;
}

int Current() { return g_current; }

void Apply(int index) {
    if (index < 0 || index >= Count) index = 0;
    g_current = index;

    const Palette& p = kPalettes[index];
    text = p.text;         dim = p.dim;       muted = p.muted;
    accent = p.accent;     accentHot = p.accentHot; cyan = p.cyan;
    panel = p.panel;       panel2 = p.panel2; stroke = p.stroke; track = p.track;
    windowBg = p.windowBg; titleBg = p.titleBg;

    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding = p.rounding[0];
    s.FrameRounding  = p.rounding[1];
    s.GrabRounding   = p.rounding[1];
    s.ChildRounding  = p.rounding[3];
    s.PopupRounding  = p.rounding[3];
    s.WindowPadding    = ImVec2(0, 0);
    s.FramePadding     = ImVec2(10, 6);
    s.ItemSpacing      = ImVec2(8, 6);
    s.WindowBorderSize = 0.f;
    s.FrameBorderSize  = 0.f;

    const ImVec4 win   = ImGui::ColorConvertU32ToFloat4(p.windowBg);
    const ImVec4 pan   = ImGui::ColorConvertU32ToFloat4(p.panel);
    const ImVec4 pan2  = ImGui::ColorConvertU32ToFloat4(p.panel2);
    const ImVec4 acc   = ImGui::ColorConvertU32ToFloat4(p.accent);
    const ImVec4 accH  = ImGui::ColorConvertU32ToFloat4(p.accentHot);
    const ImVec4 trk   = ImGui::ColorConvertU32ToFloat4(p.track);
    const ImVec4 strk  = ImGui::ColorConvertU32ToFloat4(p.stroke);
    const ImVec4 txt   = ImGui::ColorConvertU32ToFloat4(p.text);
    const ImVec4 mut   = ImGui::ColorConvertU32ToFloat4(p.muted);

    s.Colors[ImGuiCol_Text]           = txt;
    s.Colors[ImGuiCol_TextDisabled]   = mut;
    s.Colors[ImGuiCol_WindowBg]       = win;
    s.Colors[ImGuiCol_ChildBg]        = pan;
    s.Colors[ImGuiCol_PopupBg]        = win;
    s.Colors[ImGuiCol_Border]         = strk;
    s.Colors[ImGuiCol_FrameBg]        = pan2;
    s.Colors[ImGuiCol_FrameBgHovered] = trk;
    s.Colors[ImGuiCol_FrameBgActive]  = acc;
    s.Colors[ImGuiCol_Button]         = pan2;
    s.Colors[ImGuiCol_ButtonHovered]  = acc;
    s.Colors[ImGuiCol_ButtonActive]   = accH;
    s.Colors[ImGuiCol_SliderGrab]     = acc;
    s.Colors[ImGuiCol_SliderGrabActive] = accH;
    s.Colors[ImGuiCol_CheckMark]      = acc;
    s.Colors[ImGuiCol_Header]         = pan2;
    s.Colors[ImGuiCol_HeaderHovered]  = acc;
    s.Colors[ImGuiCol_HeaderActive]   = accH;
    s.Colors[ImGuiCol_Separator]      = strk;
    s.Colors[ImGuiCol_ScrollbarBg]    = win;
    s.Colors[ImGuiCol_ScrollbarGrab]  = pan2;
    s.Colors[ImGuiCol_TitleBg]        = ImGui::ColorConvertU32ToFloat4(p.titleBg);
    s.Colors[ImGuiCol_TitleBgActive]  = ImGui::ColorConvertU32ToFloat4(p.titleBg);
    s.Colors[ImGuiCol_TitleBgCollapsed] = ImGui::ColorConvertU32ToFloat4(p.titleBg);
}

} // namespace theme
