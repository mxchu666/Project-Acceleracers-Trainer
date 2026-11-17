#pragma once

#include "imgui.h"
#include "../../core/ui_theme.h"

namespace remake
{
    void DrawWindowLayers(ImDrawList* draw, ImVec2 min, ImVec2 max, const UiTheme& theme, float alpha);
    void DrawPanelLayers(ImDrawList* draw, ImVec2 min, ImVec2 max, const UiTheme& theme, float alpha);
}
