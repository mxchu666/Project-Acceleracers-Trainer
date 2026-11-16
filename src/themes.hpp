#pragma once

#include <imgui.h>

// UI palettes lifted from the packs in imGUIS/. Each entry records where the
// colors came from; the audit of those packs lives in the repo README.
namespace theme {

struct Palette {
    const char* name;
    const char* credit;

    ImU32 text, dim, muted;
    ImU32 accent, accentHot, cyan;
    ImU32 panel, panel2, stroke, track;
    ImU32 windowBg, titleBg;

    // window / frame / grab / child rounding
    float rounding[4];
};

inline const Palette kPalettes[] = {
    // Built-in orange look from the first trainer build.
    {"Default", "original trainer style",
     IM_COL32(232, 232, 240, 255), IM_COL32(205, 205, 216, 255), IM_COL32(126, 128, 146, 255),
     IM_COL32(255, 122,  24, 255), IM_COL32(210,  96,  16, 255), IM_COL32(34, 211, 238, 255),
     IM_COL32( 28,  28,  38, 255), IM_COL32( 36,  36,  48, 255), IM_COL32(52,  52,  68, 255),
     IM_COL32( 60,  60,  76, 255),
     IM_COL32( 16,  16,  22, 255), IM_COL32( 22,  22,  30, 255),
     {12.f, 7.f, 7.f, 8.f}},

    // gui.hpp + neverlose_menu.cpp (imGUIS/neverlose-last): blue accent, navy cards.
    {"Neverlose", "imGUIS / neverlose-last",
     IM_COL32(255, 255, 255, 255), IM_COL32(236, 240, 248, 255), IM_COL32(130, 133, 143, 255),
     IM_COL32( 77, 125, 255, 255), IM_COL32( 60, 100, 220, 255), IM_COL32( 92, 214, 140, 255),
     IM_COL32( 17,  19,  27, 255), IM_COL32( 11,  18,  35, 255), IM_COL32( 40,  44,  56, 255),
     IM_COL32( 29,  33,  43, 255),
     IM_COL32(  5,   9,  16, 255), IM_COL32( 10,  12,  18, 255),
     {14.f, 9.f, 9.f, 10.f}},

    // The skeet.cc pack ships stock ImGui with no palette; this is the classic
    // look the pack is named after: near-black frames, green accent.
    {"Skeet", "imGUIS / skeet.cc-master (derived)",
     IM_COL32(217, 217, 217, 255), IM_COL32(190, 190, 190, 255), IM_COL32(130, 132, 136, 255),
     IM_COL32(153, 255, 102, 255), IM_COL32(120, 214,  74, 255), IM_COL32(120, 230, 140, 255),
     IM_COL32( 24,  24,  24, 255), IM_COL32( 30,  30,  30, 255), IM_COL32( 45,  45,  45, 255),
     IM_COL32( 44,  44,  44, 255),
     IM_COL32( 17,  17,  17, 255), IM_COL32( 21,  21,  21, 255),
     { 2.f, 2.f, 2.f, 2.f}},

    // ui/core/ui_theme.h (imGUIS/onetap ui remake): red checkbox fill, grey controls.
    {"Onetap", "imGUIS / onetap ui remake",
     IM_COL32(206, 206, 206, 255), IM_COL32(180, 180, 180, 255), IM_COL32(153, 153, 153, 255),
     IM_COL32(181,   0,  41, 255), IM_COL32(156,   0,  16, 255), IM_COL32(214,  90, 110, 255),
     IM_COL32( 36,  36,  36, 255), IM_COL32( 44,  44,  44, 255), IM_COL32( 44,  44,  44, 255),
     IM_COL32( 69,  69,  69, 255),
     IM_COL32( 23,  23,  23, 255), IM_COL32( 35,  35,  35, 255),
     { 3.f, 3.f, 3.f, 4.f}},

    // FatalUITest/menu/Menu.cpp (imGUIS/project-139): charcoal frames, blue-violet accent.
    {"Fatal UI", "imGUIS / project-139",
     IM_COL32(250, 250, 250, 255), IM_COL32(210, 210, 210, 255), IM_COL32(158, 158, 158, 255),
     IM_COL32(102, 102, 255, 255), IM_COL32( 80,  80, 220, 255), IM_COL32(102, 180, 255, 255),
     IM_COL32( 38,  38,  38, 255), IM_COL32( 46,  46,  46, 255), IM_COL32( 34,  34,  34, 255),
     IM_COL32(102, 102, 102, 255),
     IM_COL32( 21,  21,  21, 255), IM_COL32( 26,  26,  26, 255),
     { 6.f, 6.f, 6.f, 6.f}},
};

inline constexpr int Count = static_cast<int>(sizeof(kPalettes) / sizeof(kPalettes[0]));

// Live palette the menu draws with. Apply() refreshes these.
inline ImU32 text, dim, muted, accent, accentHot, cyan, panel, panel2, stroke, track, windowBg, titleBg;

int  Current();
void Apply(int index);

} // namespace theme
