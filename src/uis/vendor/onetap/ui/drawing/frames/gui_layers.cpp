#include "gui_layers.hpp"

namespace remake
{
    namespace
    {
        ImU32 Pack(const ImVec4& color, float alpha)
        {
            return IM_COL32(static_cast<int>(color.x * 255.0f), static_cast<int>(color.y * 255.0f), static_cast<int>(color.z * 255.0f), static_cast<int>(color.w * 255.0f * alpha));
        }

        void Fill(ImDrawList* draw, ImVec2 min, ImVec2 max, const ImVec4& color, float alpha)
        {
            draw->AddRectFilled(min, max, Pack(color, alpha));
        }
    }

    void DrawWindowLayers(ImDrawList* draw, ImVec2 min, ImVec2 max, const UiTheme& theme, float alpha)
    {
        Fill(draw, min, max, theme.outer_shadow_color, alpha);
        Fill(draw, ImVec2(min.x + 1, min.y + 1), ImVec2(max.x - 1, max.y - 1), theme.outer_outer_line_color, alpha);
        Fill(draw, ImVec2(min.x + 2, min.y + 2), ImVec2(max.x - 2, max.y - 2), theme.outer_between_color, alpha);
        Fill(draw, ImVec2(min.x + 5, min.y + 5), ImVec2(max.x - 5, max.y - 5), theme.outer_inner_line_color, alpha);
        Fill(draw, ImVec2(min.x + 6, min.y + 6), ImVec2(max.x - 6, max.y - 6), theme.outer_face_color, alpha);
    }

    void DrawPanelLayers(ImDrawList* draw, ImVec2 min, ImVec2 max, const UiTheme& theme, float alpha)
    {
        Fill(draw, min, max, theme.panel_outer_line_color, alpha);
        Fill(draw, ImVec2(min.x + 1, min.y + 1), ImVec2(max.x - 1, max.y - 1), theme.panel_between_color, alpha);
        Fill(draw, ImVec2(min.x + 2, min.y + 2), ImVec2(max.x - 2, max.y - 2), theme.panel_inner_line_color, alpha);
        Fill(draw, ImVec2(min.x + 3, min.y + 3), ImVec2(max.x - 3, max.y - 3), theme.panel_inside_color, alpha);
    }
}
