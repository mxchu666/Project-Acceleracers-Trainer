#pragma once

#include "imgui.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace remake
{
    enum class InspectKind
    {
        Bool,
        Int,
        Color,
        Background
    };

    struct InspectItem
    {
        std::string name;
        std::string key;
        InspectKind kind;
        void* value;
        ImVec4* color;
        int min;
        int max;
        ImVec2 min_pos;
        ImVec2 max_pos;
        bool visible = true;
        float alpha = 1.0f;
    };

    struct InspectorState
    {
        std::vector<InspectItem> items;
        InspectItem selected_item{};
        bool has_selected_item = false;
        bool visible = false;
        std::unordered_map<std::string, ImVec4> item_colors;
        std::unordered_set<std::string> item_color_set;
    };
}
