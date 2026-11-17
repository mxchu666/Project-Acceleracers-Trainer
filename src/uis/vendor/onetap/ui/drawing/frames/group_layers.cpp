#include "group_layers.hpp"

namespace remake
{
    namespace
    {
        ImU32 Pack(const ImVec4& color, float alpha)
        {
            return IM_COL32(static_cast<int>(color.x * 255.0f), static_cast<int>(color.y * 255.0f), static_cast<int>(color.z * 255.0f), static_cast<int>(color.w * 255.0f * alpha));
        }
    }

    void DrawGroupLayers(ImDrawList* draw, ImVec2 min, ImVec2 max, float title_x, float title_width, const UiTheme& theme, float alpha)
    {
        const float right = max.x;
        const ImU32 face = Pack(theme.group_color, alpha * theme.layer_alpha.group_face);
        const ImU32 edge = Pack(theme.border_color, alpha * theme.layer_alpha.group_outline);
        const ImU32 black = Pack(theme.group_black_line_color, alpha * theme.layer_alpha.group_black_edge);
        draw->AddRectFilled(ImVec2(min.x + 1, min.y + 1), ImVec2(right, max.y), face);
        draw->AddRectFilled(ImVec2(min.x, min.y), ImVec2(title_x - 2.0f, min.y + 1), edge);
        draw->AddRectFilled(ImVec2(title_x + title_width + 2.0f, min.y), ImVec2(right + 1, min.y + 1), edge);
        draw->AddRectFilled(ImVec2(min.x, min.y), ImVec2(min.x + 1, max.y + 1), edge);
        draw->AddRectFilled(ImVec2(right, min.y), ImVec2(right + 1, max.y + 1), edge);
        draw->AddRectFilled(ImVec2(right + 1, min.y), ImVec2(right + 2, max.y + 1), black);
        draw->AddRectFilled(ImVec2(min.x, max.y), ImVec2(right + 1, max.y + 1), edge);
        draw->AddRectFilled(ImVec2(min.x, max.y + 1), ImVec2(right + 2, max.y + 2), black);
    }
}
