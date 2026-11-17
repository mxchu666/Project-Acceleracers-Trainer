// Include-path shim (src\uis\compat sits ahead of third_party\imgui in the
// include directories). It forwards to the real ImGui 1.90.9 headers and adds
// the one 1.92-era call the vendored Fatal menu uses. Vendored files stay
// byte-identical to the pack originals.
#pragma once

#include "../../../third_party/imgui/imgui.h"

#ifndef IMGUI_DISABLE

namespace ImGui
{
    // ImGui 1.92 moved per-size font metrics into ImFontBaked. The vendored
    // Fatal menu reads GetFontBaked()->Descent. 1.90.9 keeps Ascent/Descent on
    // ImFont itself, so the shim surfaces them through a minimal stand-in.
    struct ImFontBakedShim
    {
        float Size;
        float Ascent;
        float Descent;
    };

    ImFontBakedShim* GetFontBaked();
}

#endif // IMGUI_DISABLE
