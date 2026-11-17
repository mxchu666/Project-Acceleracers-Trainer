#pragma once

#include "imgui.h"
#include "../../core/ui_theme.h"

namespace remake
{
    void DrawGroupLayers(ImDrawList* draw, ImVec2 min, ImVec2 max, float title_x, float title_width, const UiTheme& theme, float alpha);
}
