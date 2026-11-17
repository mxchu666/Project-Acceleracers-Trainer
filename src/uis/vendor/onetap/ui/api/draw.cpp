#include "draw.hpp"
#include "../core/inspector.h"
#include "../core/ui_theme.h"
#include "../drawing/frames/gui_layers.hpp"
#include "../drawing/frames/group_layers.hpp"

#include <algorithm>
#include <cmath>
#include <cfloat>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <windows.h>

namespace remake
{
    namespace
    {
        ImVec2 InnerSize(482.0f, 500.0f);
        float GroupWidth = 211.0f;
        constexpr float ContentLeft = 52.0f;
        constexpr float ContentTop = 57.0f;
        UiTheme theme{};
        InspectorState inspector{};
        auto& inspect_items = inspector.items;
        auto& selected_item = inspector.selected_item;
        auto& has_selected_item = inspector.has_selected_item;
        auto& item_colors = inspector.item_colors;
        auto& item_color_set = inspector.item_color_set;

        std::filesystem::path ExecutableDirectory()
        {
            wchar_t buffer[MAX_PATH]{};
            const DWORD length = GetModuleFileNameW(nullptr, buffer, ARRAYSIZE(buffer));
            if (length == 0 || length >= ARRAYSIZE(buffer))
                return {};
            return std::filesystem::path(buffer, buffer + length).parent_path();
        }

        std::vector<std::filesystem::path> FindPreviewModels()
        {
            const std::filesystem::path exe_dir = ExecutableDirectory();
            const std::filesystem::path project_dir = exe_dir.parent_path();
            const std::filesystem::path bases[] = { exe_dir, project_dir };
            const std::filesystem::path rendering_files[] = {
                std::filesystem::path(L"rendering") / L"One_Piece_Figure.stl"
            };
            std::vector<std::filesystem::path> result;
            for (const auto& base : bases)
                for (const auto& relative : rendering_files)
                {
                    const auto candidate = base / relative;
                    std::error_code error;
                    if (std::filesystem::is_regular_file(candidate, error))
                        result.push_back(candidate);
                }
            if (!result.empty())
                return result;

            const std::filesystem::path fallback_names[] = {
                std::filesystem::path(L"assets") / L"models" / L"player.stl",
                L"single_color-simplified-simplified.stl"
            };
            for (const auto& base : bases)
                for (const auto& relative : fallback_names)
                {
                    const auto candidate = base / relative;
                    std::error_code error;
                    if (std::filesystem::is_regular_file(candidate, error))
                        result.push_back(candidate);
                }
            return result;
        }
        auto& background_color = theme.background_color;
        auto& group_color = theme.group_color;
        auto& border_color = theme.border_color;
        auto& text_color = theme.text_color;
        auto& control_color = theme.control_color;
        auto& track_color = theme.track_color;
        auto& checkbox_outline_color = theme.checkbox_outline_color;
        auto& checkbox_off_color = theme.checkbox_off_color;
        auto& checkbox_on_color = theme.checkbox_on_color;
        auto& checkbox_on_shade_color = theme.checkbox_on_shade_color;
        auto& checkbox_text_color = theme.checkbox_text_color;
        auto& slider_border_color = theme.slider_border_color;
        auto& slider_track_bottom_color = theme.slider_track_bottom_color;
        auto& slider_fill_shade_color = theme.slider_fill_shade_color;
        auto& slider_label_color = theme.slider_label_color;
        auto& slider_value_color = theme.slider_value_color;
        auto& slider_button_text_color = theme.slider_button_text_color;
        auto& control_border_color = theme.control_border_color;
        auto& control_text_color = theme.control_text_color;
        auto& control_arrow_color = theme.control_arrow_color;
        auto& button_text_color = theme.button_text_color;
        auto& group_black_line_color = theme.group_black_line_color;
        auto& tab_text_color = theme.tab_text_color;
        auto& tab_idle_line_color = theme.tab_idle_line_color;
        auto& tab_idle_line_bottom_color = theme.tab_idle_line_bottom_color;
        auto& tab_line_shadow_color = theme.tab_line_shadow_color;
        auto& tab_line_black_color = theme.tab_line_black_color;
        auto& outer_shadow_color = theme.outer_shadow_color;
        auto& outer_outer_line_color = theme.outer_outer_line_color;
        auto& outer_between_color = theme.outer_between_color;
        auto& outer_inner_line_color = theme.outer_inner_line_color;
        auto& outer_face_color = theme.outer_face_color;
        auto& panel_outer_line_color = theme.panel_outer_line_color;
        auto& panel_between_color = theme.panel_between_color;
        auto& panel_inner_line_color = theme.panel_inner_line_color;
        auto& panel_inside_color = theme.panel_inside_color;

        ImU32 OverrideColor(const char* name, float x, float y, ImU32 fallback, float alpha)
        {
            char key[128];
            std::snprintf(key, sizeof(key), "%s##%.0f,%.0f", name ? name : "item", x, y);
            auto it = item_colors.find(key);
            if (it == item_colors.end())
                it = item_colors.emplace(key, ImGui::ColorConvertU32ToFloat4(fallback)).first;
            if (item_color_set.find(key) == item_color_set.end())
                return fallback;
            return IM_COL32(static_cast<int>(it->second.x * 255.0f), static_cast<int>(it->second.y * 255.0f), static_cast<int>(it->second.z * 255.0f), static_cast<int>(it->second.w * 255.0f * alpha));
        }

        ImU32 PackColor(const ImVec4& color, float alpha)
        {
            return IM_COL32(static_cast<int>(color.x * 255), static_cast<int>(color.y * 255), static_cast<int>(color.z * 255), static_cast<int>(color.w * 255 * alpha));
        }

        void Inspect(const char* name, InspectKind kind, void* value, int min, int max, float x, float y, float w, float h)
        {
            char key[128];
            std::snprintf(key, sizeof(key), "%s##%.0f,%.0f", name ? name : "item", x, y);
            ImVec4* color = nullptr;
            if (kind != InspectKind::Color && kind != InspectKind::Background)
            {
                auto it = item_colors.find(key);
                if (it == item_colors.end())
                {
                    it = item_colors.emplace(key, ImVec4(0, 0, 0, 0)).first;
                }
                color = &it->second;
            }
            else if (kind == InspectKind::Color && value != nullptr)
                color = reinterpret_cast<ImVec4*>(value);
            inspect_items.push_back({ name ? name : "item", key, kind, value, color, min, max, ImVec2(x, y), ImVec2(x + w, y + h), true, 1.0f });
        }

        void SelectInspectItem()
        {
            const ImVec2 mouse = ImGui::GetIO().MousePos;
            if (!ImGui::GetIO().KeyShift || !ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                return;
            for (auto it = inspect_items.rbegin(); it != inspect_items.rend(); ++it)
            {
                if (mouse.x >= it->min_pos.x && mouse.x <= it->max_pos.x && mouse.y >= it->min_pos.y && mouse.y <= it->max_pos.y)
                {
                    selected_item = *it;
                    has_selected_item = true;
                    return;
                }
            }
            selected_item = { "Empty canvas", "empty_canvas", InspectKind::Background, &background_color, &background_color, 0, 0, ImVec2(0, 0), ImGui::GetIO().DisplaySize, true, 1.0f };
            has_selected_item = true;
        }

        ImU32 Color(int r, int g, int b, int a = 255)
        {
            return IM_COL32(r, g, b, a);
        }

        void DrawTextShadow(ImDrawList* draw, ImVec2 pos, ImU32 col, const char* text)
        {
            draw->AddText(ImVec2(std::round(pos.x + 1.0f), std::round(pos.y + 1.0f)), IM_COL32(0, 0, 0, 220), text);
            draw->AddText(ImVec2(std::round(pos.x), std::round(pos.y)), col, text);
        }

        struct Canvas
        {
            ImDrawList* draw;
            UiState& state;
            float alpha;
            float x;
            float y;
            float group_x;
            float group_y;
            float title_width;
            ImVec2 state_pos;
            float scale_x;
            float scale_y;
            std::string group_title;

            ImU32 C(int r, int g, int b, int a = 255) const
            {
                return Color(r, g, b, static_cast<int>(a * alpha));
            }

        ImU32 Accent(int a = 255) const
        {
            return IM_COL32(static_cast<int>(state.accent[0] * 255.0f), static_cast<int>(state.accent[1] * 255.0f), static_cast<int>(state.accent[2] * 255.0f), static_cast<int>(a * alpha));
        }

            void Rect(float x0, float y0, float x1, float y1, ImU32 col)
            {
                draw->AddRectFilled(ImVec2(std::round(x0), std::round(y0)), ImVec2(std::round(x1), std::round(y1)), col);
            }

            void Text(float x0, float y0, const char* text, ImU32 col, bool = false)
            {
                draw->AddText(ImVec2(std::round(x0), std::round(y0)), col, text);
            }

            bool Hit(float x0, float y0, float w, float h) const
            {
                const ImVec2 p = ImGui::GetIO().MousePos;
                return p.x > x0 && p.x < x0 + w && p.y > y0 && p.y < y0 + h;
            }

            bool Click(float x0, float y0, float w, float h, const char* id, const void* salt = nullptr)
            {
                ImGui::SetCursorScreenPos(ImVec2(x0, y0));
                ImGui::PushID(id);
                ImGui::PushID(salt);
                ImGui::PushID(static_cast<int>(x0));
                ImGui::PushID(static_cast<int>(y0));
                const bool hit = ImGui::InvisibleButton("hit", ImVec2(w, h));
                ImGui::PopID();
                ImGui::PopID();
                ImGui::PopID();
                ImGui::PopID();
                return hit;
            }

            void BeginColumn(bool right)
            {
                x = state_pos.x + (right ? 282.0f : ContentLeft);
                y = state_pos.y + ContentTop - state.content_scroll;
            }

            void BeginGroup(const char* title)
            {
                group_x = x - 10.0f * scale_x;
                group_y = y + 5.0f * scale_y;
                group_title = title;
                title_width = ImGui::CalcTextSize(title).x;
                y += 15.0f * scale_y;
                ImGui::PushID(title);
            }

            void EndGroup()
            {
                const float end_x = x - 10.0f * scale_x;
                const float end_y = y + 5.0f * scale_y;
                const float group_width = GroupWidth * scale_x;
                const float title_x = std::round(group_x + 10 * scale_x);
                const float title_y = std::round(group_y - 6 * scale_y);
                DrawGroupLayers(draw, ImVec2(std::round(group_x), std::round(group_y)), ImVec2(std::round(group_x + group_width), std::round(end_y)), title_x, std::round(title_width), theme, alpha);
                Text(title_x, title_y, group_title.c_str(), IM_COL32(static_cast<int>(text_color.x * 255), static_cast<int>(text_color.y * 255), static_cast<int>(text_color.z * 255), static_cast<int>(text_color.w * 255 * alpha)), true);
                y += 15.0f * scale_y;
                ImGui::PopID();
            }

            void Label(const char* text)
            {
                if (text)
                    Text(x + 20, y - 3, text, PackColor(slider_label_color, alpha), true);
                y += 15.0f;
            }

            void Check(const char* label, bool& value, bool small = false)
            {
                const float sx = x;
                const float sy = y;
                const float tw = label ? ImGui::CalcTextSize(label).x : 0;
                const float hitw = small ? 120.0f : 25.0f + tw;
                Inspect(label ? label : "checkbox", InspectKind::Bool, &value, 0, 1, sx, sy - 3, hitw, 14);
                if (Click(sx, sy - 3, hitw, 14, label ? label : "small", &value))
                    value = !value;
                Rect(sx, sy, sx + 8, sy + 8, PackColor(checkbox_outline_color, alpha));
                Rect(sx + 1, sy + 1, sx + 7, sy + 7, OverrideColor(label, sx, sy - 3, PackColor(checkbox_off_color, alpha), alpha));
                if (value)
                {
                    draw->AddRectFilledMultiColor(ImVec2(sx + 1, sy + 1), ImVec2(sx + 7, sy + 7), OverrideColor(label, sx, sy - 3, PackColor(checkbox_on_color, alpha), alpha), OverrideColor(label, sx, sy - 3, PackColor(checkbox_on_color, alpha), alpha), PackColor(checkbox_on_shade_color, alpha), PackColor(checkbox_on_shade_color, alpha));
                }
                if (label && !small)
                    Text(std::round(sx + 20), std::round(sy - 4), label, PackColor(checkbox_text_color, alpha), true);
                if (!(small && !label))
                    y += 15.0f;
            }

            void Slider(const char* label, int& value, int min, int max, const char* suffix = "")
            {
                const bool has_label = label && std::strlen(label) > 1;
                if (has_label)
                {
                    Text(x + 20, y - 3, label, PackColor(slider_label_color, alpha), true);
                    y += 15;
                }
                const float sx = x + 20;
                const float sy = y;
                const float width = 150;
                Inspect(label ? label : "slider", InspectKind::Int, &value, min, max, sx - 22, sy - 4, width + 44, 15);
                value = std::clamp(value, min, max);
                ImGui::PushID(label ? label : "slider");
                 const bool minus = Click(sx - 22, sy - 4, 16, 15, "minus");
                 const bool plus = Click(sx + width + 6, sy - 4, 16, 15, "plus");
                ImGui::PopID();
                if (minus && value > min)
                    --value;
                if (plus && value < max)
                    ++value;
                Click(sx - 1, sy - 1, width + 2, 8, "slider-track");
                if (ImGui::IsItemActive() && max > min)
                    value = min + static_cast<int>((max - min) * std::clamp((ImGui::GetIO().MousePos.x - sx) / width, 0.0f, 1.0f));
                const float pos = max == min ? 0 : width * static_cast<float>(value - min) / static_cast<float>(max - min);
                Text(sx - 18, sy - 4, "-", PackColor(slider_button_text_color, alpha));
                Text(sx + width + 10, sy - 4, "+", PackColor(slider_button_text_color, alpha));
                Rect(sx - 1, sy - 1, sx + width + 1, sy + 7, PackColor(slider_border_color, alpha));
                const ImU32 track = OverrideColor(label, sx - 22, sy - 4, IM_COL32(static_cast<int>(track_color.x * 255), static_cast<int>(track_color.y * 255), static_cast<int>(track_color.z * 255), static_cast<int>(track_color.w * 255 * alpha)), alpha);
                draw->AddRectFilledMultiColor(ImVec2(sx, sy), ImVec2(sx + width, sy + 6), track, track, PackColor(slider_track_bottom_color, alpha), PackColor(slider_track_bottom_color, alpha));
                if (pos > 0.0f)
                    draw->AddRectFilledMultiColor(ImVec2(sx, sy), ImVec2(sx + pos, sy + 6), OverrideColor(label, sx - 22, sy - 4, Accent(), alpha), OverrideColor(label, sx - 22, sy - 4, Accent(), alpha), PackColor(slider_fill_shade_color, alpha), PackColor(slider_fill_shade_color, alpha));
                char buf[64];
                std::snprintf(buf, sizeof(buf), "%d%s", value, suffix);
                const float text_w = ImGui::CalcTextSize(buf).x;
                const float plus_x = sx + width + 10.0f;
                const float center_x = sx + pos;
                const float left_x = std::min(center_x - text_w * 0.5f, plus_x - text_w - 1.0f);
                Text(std::max(sx + 1.0f, left_x), sy, buf, PackColor(slider_value_color, alpha));
                y += 15;
            }

            void Button(const char* label, bool& pressed)
            {
                const float bx = x + 20;
                const float by = y;
                Inspect(label, InspectKind::Bool, &pressed, 0, 1, bx, by, 150, 20);
                if (Click(bx, by, 150, 20, label))
                    pressed = true;
                Rect(bx - 1, by - 1, bx + 151, by + 21, PackColor(control_border_color, alpha));
                Rect(bx, by, bx + 150, by + 20, OverrideColor(label, bx, by, IM_COL32(static_cast<int>(control_color.x * 255), static_cast<int>(control_color.y * 255), static_cast<int>(control_color.z * 255), static_cast<int>(control_color.w * 255 * alpha)), alpha));
                const ImVec2 size = ImGui::CalcTextSize(label);
                Text(bx + (150.0f - size.x) * 0.5f, by + (20.0f - size.y) * 0.5f, label, PackColor(button_text_color, alpha));
                y += 25;
            }

            void ColorPicker(const char* id, float color[4])
            {
                struct PickerState
                {
                    bool open = false;
                    bool held = false;
                    bool held_alpha = false;
                    float color[3]{};
                    ImVec2 pos{};
                    ImVec2 size{179.0f, 179.0f};
                    ImVec2 marker{-1.0f, -1.0f};
                };
                static std::unordered_map<std::string, PickerState> pickers;
                PickerState& picker = pickers[id];
                constexpr float bar_h = 8.0f;
                constexpr float pad = 10.0f;
                const float sx = x + 150.0f;
                const float sy = y - 15.0f;
                ImGui::SetCursorScreenPos(ImVec2(sx, sy));
                ImGui::PushID(id);
                if (ImGui::InvisibleButton("swatch", ImVec2(20.0f, 8.0f)))
                {
                    picker.open = !picker.open;
                    picker.pos = ImVec2(sx, sy + 10.0f);
                    picker.color[0] = color[0];
                    picker.color[1] = color[1];
                    picker.color[2] = color[2];
                    if (picker.marker.x < 0.0f)
                    {
                        float h, s, v;
                        ImGui::ColorConvertRGBtoHSV(color[0], color[1], color[2], h, s, v);
                        picker.marker = ImVec2(h * picker.size.x, picker.size.y * 0.5f * (v < 1.0f ? 2.0f - v : s));
                    }
                }
                Rect(sx, sy, sx + 20.0f, sy + 8.0f, IM_COL32(0, 0, 0, static_cast<int>(alpha * 255.0f)));
                for (int i = 0; i < 6; ++i)
                    for (int j = 0; j < 2; ++j)
                        Rect(sx + 1.0f + i * 3.0f, sy + 1.0f + j * 3.0f, sx + 4.0f + i * 3.0f, sy + 4.0f + j * 3.0f, (i + j) & 1 ? C(153, 153, 153) : C(255, 255, 255));
                Rect(sx + 1.0f, sy + 1.0f, sx + 19.0f, sy + 7.0f, IM_COL32(static_cast<int>(color[0] * 255.0f), static_cast<int>(color[1] * 255.0f), static_cast<int>(color[2] * 255.0f), static_cast<int>(alpha * color[3] * 255.0f)));
                if (picker.open)
                {
                    ImGui::SetNextWindowPos(picker.pos, ImGuiCond_Always);
                    ImGui::SetNextWindowSize(ImVec2(picker.size.x + 2.0f + pad * 2.0f, picker.size.y + bar_h + 3.0f + pad * 2.0f), ImGuiCond_Always);
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(1, 1));
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
                    ImGui::PushStyleColor(ImGuiCol_WindowBg, C(16, 16, 16));
                    ImGui::Begin("picker_window", &picker.open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);
                    ImDrawList* picker_draw = ImGui::GetForegroundDrawList();
                    const ImVec2 picker_min = ImGui::GetWindowPos();
                    const ImVec2 picker_max(picker_min.x + picker.size.x + 2.0f + pad * 2.0f, picker_min.y + picker.size.y + bar_h + 3.0f + pad * 2.0f);
                    const ImVec2 mouse = ImGui::GetIO().MousePos;
                    const bool mouse_in_picker = mouse.x >= picker_min.x && mouse.x <= picker_max.x && mouse.y >= picker_min.y && mouse.y <= picker_max.y;
                    const bool mouse_in_swatch = mouse.x >= sx && mouse.x <= sx + 20.0f && mouse.y >= sy && mouse.y <= sy + 8.0f;
                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !mouse_in_picker && !mouse_in_swatch)
                        picker.open = false;
                    if (!picker.open)
                    {
                        ImGui::End();
                        ImGui::PopStyleColor();
                        ImGui::PopStyleVar(3);
                        ImGui::PopID();
                        return;
                    }
                    picker_draw->AddRectFilled(picker_min, picker_max, PackColor(background_color, 1.0f));
                    DrawWindowLayers(picker_draw, picker_min, picker_max, theme, 1.0f);
                    picker_draw->AddRectFilled(ImVec2(picker_min.x + pad, picker_min.y + pad), ImVec2(picker_max.x - pad, picker_max.y - pad), IM_COL32(0, 0, 0, 255));
                    const ImVec2 palette_min(picker_min.x + pad + 1.0f, picker_min.y + pad + 1.0f);
                    const ImVec2 palette_max(palette_min.x + picker.size.x, palette_min.y + picker.size.y);
                    ImGui::SetCursorScreenPos(palette_min);
                    ImGui::InvisibleButton("palette_area", picker.size);
                    const bool palette_clicked = ImGui::IsItemClicked();
                    for (int i = 0; i < static_cast<int>(picker.size.x); ++i)
                    {
                        ImVec4 hue_color;
                        ImGui::ColorConvertHSVtoRGB(static_cast<float>(i) / picker.size.x, 1.0f, 1.0f, hue_color.x, hue_color.y, hue_color.z);
                        const ImU32 top = IM_COL32(255, 255, 255, 255);
                        const ImU32 middle = IM_COL32(static_cast<int>(hue_color.x * 255.0f), static_cast<int>(hue_color.y * 255.0f), static_cast<int>(hue_color.z * 255.0f), 255);
                        const float px = palette_min.x + static_cast<float>(i);
                        picker_draw->AddRectFilledMultiColor(ImVec2(px, palette_min.y), ImVec2(px + 1.0f, palette_min.y + picker.size.y * 0.5f), top, top, middle, middle);
                        picker_draw->AddRectFilledMultiColor(ImVec2(px, palette_min.y + picker.size.y * 0.5f), ImVec2(px + 1.0f, palette_max.y), middle, middle, IM_COL32(0, 0, 0, 255), IM_COL32(0, 0, 0, 255));
                    }
                    if (palette_clicked)
                        picker.held = true;
                    if (picker.held && ImGui::IsMouseDown(ImGuiMouseButton_Left))
                    {
                        const float sample_x = std::clamp(mouse.x, palette_min.x, palette_max.x);
                        const float sample_y = std::clamp(mouse.y, palette_min.y, palette_max.y);
                        picker.marker = ImVec2(sample_x - palette_min.x, sample_y - palette_min.y);
                        const float hue = std::clamp((sample_x - palette_min.x) / picker.size.x, 0.0f, 1.0f);
                        const float half = picker.size.y * 0.5f;
                        const float sat = sample_y - palette_min.y > half ? 1.0f : std::clamp((sample_y - palette_min.y) / half, 0.0f, 1.0f);
                        const float val = sample_y - palette_min.y > half ? 1.0f - std::clamp((sample_y - palette_min.y - half) / half, 0.0f, 1.0f) : 1.0f;
                        ImGui::ColorConvertHSVtoRGB(hue, sat, val, color[0], color[1], color[2]);
                        picker.color[0] = color[0];
                        picker.color[1] = color[1];
                        picker.color[2] = color[2];
                    }
                    if (picker.held && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
                    {
                        color[0] = picker.color[0];
                        color[1] = picker.color[1];
                        color[2] = picker.color[2];
                        picker.held = false;
                    }
                    const ImVec2 mark(palette_min.x + picker.marker.x, palette_min.y + picker.marker.y);
                    picker_draw->PushClipRect(palette_min, palette_max, true);
                    picker_draw->AddCircle(mark, 4.5f, IM_COL32(0, 0, 0, 255), 0, 3.0f);
                    picker_draw->AddCircle(mark, 4.5f, IM_COL32(255, 255, 255, 255), 0, 1.0f);
                    picker_draw->PopClipRect();
                    const ImVec2 bar_min(palette_min.x, palette_max.y + 1.0f);
                    const ImVec2 bar_max(palette_max.x, bar_min.y + bar_h);
                    ImGui::SetCursorScreenPos(bar_min);
                    ImGui::InvisibleButton("alpha_area", ImVec2(picker.size.x, bar_h));
                    if (ImGui::IsItemClicked())
                        picker.held_alpha = true;
                    if (picker.held_alpha)
                    {
                        if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
                            color[3] = std::clamp((mouse.x - bar_min.x - 1.5f) / (picker.size.x - 3.0f), 0.0f, 1.0f);
                        else
                            picker.held_alpha = false;
                    }
                    for (int i = 0; i * 4 < static_cast<int>(picker.size.x); ++i)
                        for (int j = 0; j < 2; ++j)
                        {
                            const ImVec2 cell_min(bar_min.x + i * 4.0f, bar_min.y + j * 4.0f);
                            picker_draw->AddRectFilled(cell_min, ImVec2(std::min(cell_min.x + 4.0f, bar_max.x), cell_min.y + 4.0f), (i + j) & 1 ? IM_COL32(153, 153, 153, 255) : IM_COL32(255, 255, 255, 255));
                        }
                    const ImU32 opaque = ImGui::ColorConvertFloat4ToU32(ImVec4(color[0], color[1], color[2], 1.0f));
                    const ImU32 transparent = opaque & ~IM_COL32_A_MASK;
                    picker_draw->AddRectFilledMultiColor(bar_min, bar_max, transparent, opaque, opaque, transparent);
                    const float handle_x = std::round(bar_min.x + color[3] * (picker.size.x - 3.0f));
                    picker_draw->AddRectFilled(ImVec2(handle_x, bar_min.y), ImVec2(handle_x + 3.0f, bar_max.y), IM_COL32(0, 0, 0, 255));
                    picker_draw->AddRectFilled(ImVec2(handle_x + 1.0f, bar_min.y), ImVec2(handle_x + 2.0f, bar_max.y), IM_COL32(255, 255, 255, 255));
                    ImGui::End();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar(3);
                }
                ImGui::PopID();
            }

            void Dropdown(const char* label, int& value, const std::vector<const char*>& items, bool multi = false, unsigned int* selection_mask = nullptr)
            {
                const bool has_label = label && std::strlen(label) > 1;
                const float bx = x + 20;
                float by = y;
                y += 25;
                if (has_label)
                {
                    Text(bx, by - 3, label, PackColor(slider_label_color, alpha), true);
                    by += 15;
                    y += 15;
                }
                Inspect(label ? label : "dropdown", InspectKind::Int, &value, 0, static_cast<int>(items.size()) - 1, bx, by, 150, 20);
                if (!multi)
                    value = std::clamp(value, 0, static_cast<int>(items.size()) - 1);
                ImGui::SetCursorScreenPos(ImVec2(bx, by));
                ImGui::PushID(label);
                ImGui::PushID(bx);
                ImGui::PushID(by);
                ImGui::InvisibleButton("combo", ImVec2(150, 20));
                const bool open = ImGui::IsItemClicked();
                ImGui::PopID();
                ImGui::PopID();
                ImGui::PopID();
                Rect(bx - 1, by - 1, bx + 151, by + 21, PackColor(control_border_color, alpha));
                Rect(bx, by, bx + 150, by + 20, OverrideColor(label, bx, by, IM_COL32(static_cast<int>(control_color.x * 255), static_cast<int>(control_color.y * 255), static_cast<int>(control_color.z * 255), static_cast<int>(control_color.w * 255 * alpha)), alpha));
                std::string display;
                if (multi && selection_mask)
                {
                    for (int i = 0; i < static_cast<int>(items.size()); ++i)
                    {
                        if ((*selection_mask & (1u << i)) == 0)
                            continue;
                        if (!display.empty())
                            display += ", ";
                        display += items[i];
                    }
                    if (display.empty())
                        display = "off";
                    if (ImGui::CalcTextSize(display.c_str()).x > 105.0f)
                    {
                        while (!display.empty() && ImGui::CalcTextSize((display + "...").c_str()).x > 105.0f)
                            display.pop_back();
                        display += "...";
                    }
                }
                else
                    display = items[value];
                DrawTextShadow(draw, ImVec2(bx + 10, by + 3), PackColor(control_text_color, alpha), display.c_str());
                const float arrow_x = bx + 139.0f;
                const float arrow_mid_y = by + 10.0f;
                draw->AddTriangleFilled(ImVec2(arrow_x, arrow_mid_y - 2.0f), ImVec2(arrow_x + 6.0f, arrow_mid_y - 2.0f), ImVec2(arrow_x + 3.0f, arrow_mid_y + 2.0f), PackColor(control_arrow_color, alpha));
                ImGui::PushID(label);
                ImGui::PushID(bx);
                ImGui::PushID(by);
                if (open)
                    ImGui::OpenPopup("list");
                if (ImGui::IsPopupOpen("list"))
                {
                    ImGui::SetNextWindowPos(ImVec2(bx - 1.0f, by + 21), ImGuiCond_Appearing);
                    ImGui::SetNextWindowSize(ImVec2(151, 0), ImGuiCond_Appearing);
                    ImGui::SetNextWindowSizeConstraints(ImVec2(151, 0), ImVec2(151, ImGui::GetIO().DisplaySize.y * 0.65f));
                }
                 ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(151, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 8.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0, 0));
                ImGui::PushStyleColor(ImGuiCol_PopupBg, C(36, 36, 36));
                ImGui::PushStyleColor(ImGuiCol_Border, C(0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, C(30, 30, 30));
                ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, C(80, 80, 80));
                ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(0, 0, 0, 0));
                ImGui::SetNextWindowSizeConstraints(ImVec2(151, 0), ImVec2(151, ImGui::GetIO().DisplaySize.y * 0.65f));
                if (ImGui::BeginPopup("list", ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings))
                {
                    ImDrawList* popup_draw = ImGui::GetWindowDrawList();
                    for (int i = 0; i < static_cast<int>(items.size()); ++i)
                    {
                        const ImVec2 item_pos = ImGui::GetCursorScreenPos();
                        ImGui::PushID(i);
                         ImGui::InvisibleButton("row", ImVec2(143, 18));
                        const bool hovered = ImGui::IsItemHovered();
                        const bool selected = multi && selection_mask ? ((*selection_mask & (1u << i)) != 0) : value == i;
                        const bool clicked = ImGui::IsItemClicked();
                        ImGui::PopID();
                         popup_draw->AddText(ImVec2(item_pos.x + 6, item_pos.y + 2), selected ? Accent() : hovered ? Accent(140) : C(153, 153, 153), items[i]);
                        if (clicked)
                        {
                            if (multi && selection_mask)
                                *selection_mask ^= 1u << i;
                            else
                            {
                                value = i;
                                ImGui::CloseCurrentPopup();
                            }
                        }
                    }
                    ImGui::EndPopup();
                }
                ImGui::PopStyleColor(7);
                ImGui::PopStyleVar(8);
                ImGui::PopID();
                ImGui::PopID();
                ImGui::PopID();
            }

            void Key(const char* label, int& key)
            {
                Text(x + 20, y - 3, label, C(206, 206, 206), true);
                const float bx = x + 20;
                const float by = y + 12;
                Inspect(label, InspectKind::Int, &key, 0, 255, bx, by, 150, 20);
                bool& capturing = &key == &state.aim_key ? state.aim_key_capture : state.trigger_key_capture;
                ImGui::SetCursorScreenPos(ImVec2(bx, by));
                ImGui::PushID(label);
                ImGui::PushID(bx);
                ImGui::PushID(by);
                ImGui::InvisibleButton("key", ImVec2(150, 20));
                const bool started_capture = ImGui::IsItemClicked();
                if (started_capture)
                    capturing = true;
                Rect(bx - 1, by - 1, bx + 151, by + 21, PackColor(control_border_color, alpha));
                Rect(bx, by, bx + 150, by + 20, OverrideColor(label, bx, by, IM_COL32(static_cast<int>(control_color.x * 255), static_cast<int>(control_color.y * 255), static_cast<int>(control_color.z * 255), static_cast<int>(control_color.w * 255 * alpha)), alpha));
                const char* key_name = capturing ? "..." : key == 0 ? "none" : ImGui::GetKeyName(static_cast<ImGuiKey>(key));
                DrawTextShadow(draw, ImVec2(bx + 10, by + 3), PackColor(control_text_color, alpha), key_name && key_name[0] ? key_name : "none");
                if (capturing && !started_capture)
                {
                    if (ImGui::IsKeyPressed(ImGuiKey_Escape))
                    {
                        key = 0;
                        capturing = false;
                    }
                    else
                    {
                        for (int i = 0; i < 5; ++i)
                        {
                            if (!ImGui::IsMouseClicked(i))
                                continue;
                            key = ImGuiKey_MouseLeft + i;
                            capturing = false;
                            break;
                        }
                        for (int i = ImGuiKey_NamedKey_BEGIN; i < ImGuiKey_NamedKey_END; ++i)
                        {
                            if (!capturing)
                                break;
                            const ImGuiKey candidate = static_cast<ImGuiKey>(i);
                            if (candidate >= ImGuiKey_GamepadStart && candidate <= ImGuiKey_GamepadRStickDown)
                                continue;
                            if (ImGui::IsKeyPressed(candidate))
                            {
                                key = i;
                                capturing = false;
                                break;
                            }
                        }
                    }
                }
                ImGui::PopID();
                ImGui::PopID();
                ImGui::PopID();
                y += 40;
            }
        };

        void DrawTabs(Canvas& c, UiState& s)
        {
            static const char* names[] = { "legit", "rage", "players", "visuals", "misc", "config" };
            const float x = s.window_pos.x + 20;
            const float y = s.window_pos.y + 16;
            const float w = InnerSize.x / 6.0f;
            for (int i = 0; i < 6; ++i)
            {
                const float cx = x + w * i + w * 0.5f;
                const float text_w = ImGui::CalcTextSize(names[i]).x;
                const float text_x = cx - text_w * 0.5f;
                const Tab tab = static_cast<Tab>(i);
                Inspect(names[i], InspectKind::Color, s.accent, 0, 0, x + w * i, y, w, 24.0f);
                ImGui::SetCursorScreenPos(ImVec2(x + w * i, y));
                ImGui::PushID(i);
                ImGui::InvisibleButton("tab", ImVec2(w, 24.0f));
                if (ImGui::IsItemClicked())
                    s.tab = tab;
                const bool selected = s.tab == tab;
                c.Text(text_x, y + 1.0f, names[i], PackColor(tab_text_color, c.alpha));
                const float line_x = x + w * i + (i == 0 ? -3.0f : 2.0f);
                const float line_y = s.window_pos.y + 35.0f - (selected ? 1.0f : 0.0f);
                const float line_r = x + w * (i + 1) + (i == 5 ? 3.0f : -2.0f);
                const ImU32 line_top = selected ? c.Accent() : PackColor(tab_idle_line_color, c.alpha);
                const ImU32 line_bottom = selected ? c.C(160, 16, 37) : PackColor(tab_idle_line_bottom_color, c.alpha);
                const float line_h = selected ? 5.0f : 3.0f;
                if (selected)
                {
                    const ImU32 highlight = IM_COL32(255, 92, 112, static_cast<int>(c.alpha * 255.0f));
                    c.draw->AddRectFilledMultiColor(ImVec2(line_x, line_y), ImVec2(line_r, line_y + 1.0f), highlight, highlight, line_top, line_top);
                    c.draw->AddRectFilledMultiColor(ImVec2(line_x, line_y + 1.0f), ImVec2(line_r, line_y + line_h - 1.0f), line_top, line_top, line_bottom, line_bottom);
                    c.draw->AddRectFilled(ImVec2(line_x, line_y + line_h - 1.0f), ImVec2(line_r, line_y + line_h), c.C(95, 8, 22));
                }
                else
                    c.draw->AddRectFilledMultiColor(ImVec2(line_x, line_y), ImVec2(line_r, line_y + line_h), line_top, line_top, line_bottom, line_bottom);
                c.draw->AddRectFilledMultiColor(ImVec2(line_x, line_y + line_h), ImVec2(line_r, line_y + line_h + 1.0f), PackColor(tab_line_shadow_color, c.alpha), PackColor(tab_line_shadow_color, c.alpha), PackColor(tab_line_black_color, c.alpha), PackColor(tab_line_black_color, c.alpha));
                ImGui::PopID();
            }
        }

        void DrawBrand(Canvas& c, const UiState& s)
        {
            if (!s.brand_font)
                return;

            const double time = ImGui::GetTime();
            const char* wave = "pidora";
            const char* type = "haxxx";
            const double cycle = std::fmod(time, 5.0);
            int count = cycle < 2.5 ? static_cast<int>(cycle * 2.0 + 0.5) : 5 - static_cast<int>((cycle - 2.5) * 2.0 + 0.5);
            count = std::clamp(count, 0, 5);
            float brand_width = 0.0f;
            for (int i = 0; wave[i]; ++i)
            {
                const char glyph[] = { wave[i], '\0' };
                brand_width += s.brand_font->CalcTextSizeA(15.0f, FLT_MAX, 0.0f, glyph).x;
            }
            for (int i = 0; i < count; ++i)
            {
                const char glyph[] = { type[i], '\0' };
                brand_width += s.brand_font->CalcTextSizeA(15.0f, FLT_MAX, 0.0f, glyph).x;
            }
            const ImVec2 origin(std::round(s.window_pos.x + 261.0f - brand_width * 0.5f), std::round(s.window_pos.y - 7.0f));
            float x = origin.x;
            const ImDrawListFlags old_flags = c.draw->Flags;
            c.draw->Flags &= ~ImDrawListFlags_AntiAliasedLines;
            ImGui::PushFont(s.brand_font);
            for (int i = 0; wave[i]; ++i)
            {
                const float phase = static_cast<float>(time * 3.2 + i * 0.65);
                const float pulse = (std::sin(phase) + 1.0f) * 0.5f;
                const float lift = std::round(std::sin(phase) * 3.0f);
                const ImU32 col = IM_COL32(static_cast<int>(185 + pulse * 70), static_cast<int>(20 + pulse * 45), static_cast<int>(35 + pulse * 60), static_cast<int>(c.alpha * 255.0f));
                const char glyph[] = { wave[i], '\0' };
                const ImVec2 pos(std::round(x), std::round(origin.y + lift));
                c.draw->AddText(s.brand_font, 15.0f, ImVec2(pos.x - 1.0f, pos.y), IM_COL32(0, 0, 0, static_cast<int>(c.alpha * 255.0f)), glyph);
                c.draw->AddText(s.brand_font, 15.0f, ImVec2(pos.x + 1.0f, pos.y), IM_COL32(0, 0, 0, static_cast<int>(c.alpha * 255.0f)), glyph);
                c.draw->AddText(s.brand_font, 15.0f, ImVec2(pos.x, pos.y - 1.0f), IM_COL32(0, 0, 0, static_cast<int>(c.alpha * 255.0f)), glyph);
                c.draw->AddText(s.brand_font, 15.0f, ImVec2(pos.x, pos.y + 1.0f), IM_COL32(0, 0, 0, static_cast<int>(c.alpha * 255.0f)), glyph);
                c.draw->AddText(s.brand_font, 15.0f, pos, col, glyph);
                x += ImGui::CalcTextSize(glyph, nullptr, false, 15.0f).x;
            }
            char suffix[6]{};
            std::memcpy(suffix, type, count);
            for (int i = 0; i < count; ++i)
            {
                const float phase = static_cast<float>(time * 3.2 + (i + 6) * 0.65);
                const float pulse = (std::sin(phase) + 1.0f) * 0.5f;
                const float lift = std::round(std::sin(phase) * 3.0f);
                const char glyph[] = { suffix[i], '\0' };
                const ImU32 col = IM_COL32(static_cast<int>(170 + pulse * 70), static_cast<int>(170 + pulse * 70), static_cast<int>(170 + pulse * 70), static_cast<int>(c.alpha * 255.0f));
                const ImVec2 pos(std::round(x), std::round(origin.y + lift));
                c.draw->AddText(s.brand_font, 15.0f, ImVec2(pos.x - 1.0f, pos.y), IM_COL32(0, 0, 0, static_cast<int>(c.alpha * 255.0f)), glyph);
                c.draw->AddText(s.brand_font, 15.0f, ImVec2(pos.x + 1.0f, pos.y), IM_COL32(0, 0, 0, static_cast<int>(c.alpha * 255.0f)), glyph);
                c.draw->AddText(s.brand_font, 15.0f, ImVec2(pos.x, pos.y - 1.0f), IM_COL32(0, 0, 0, static_cast<int>(c.alpha * 255.0f)), glyph);
                c.draw->AddText(s.brand_font, 15.0f, ImVec2(pos.x, pos.y + 1.0f), IM_COL32(0, 0, 0, static_cast<int>(c.alpha * 255.0f)), glyph);
                c.draw->AddText(s.brand_font, 15.0f, pos, col, glyph);
                x += ImGui::CalcTextSize(glyph, nullptr, false, 15.0f).x;
            }
            ImGui::PopFont();
            c.draw->Flags = old_flags;
        }

        void DrawPreview(UiState& state, float alpha)
        {
            constexpr float scale = 0.76f;
            constexpr float width = 370.0f * scale;
            constexpr float height = 475.0f * scale;
            const float x = state.preview_pos.x;
            const float y = state.preview_pos.y;
            ImDrawList* draw = ImGui::GetBackgroundDrawList();
            draw->AddRectFilled(ImVec2(x, y), ImVec2(x + width, y + height), PackColor(outer_shadow_color, alpha));
            draw->AddRectFilled(ImVec2(x + 1, y + 1), ImVec2(x + width - 1, y + height - 1), PackColor(outer_outer_line_color, alpha));
            draw->AddRectFilled(ImVec2(x + 2, y + 2), ImVec2(x + width - 2, y + height - 2), PackColor(outer_between_color, alpha));
            draw->AddRectFilled(ImVec2(x + 5, y + 5), ImVec2(x + width - 5, y + height - 5), PackColor(outer_inner_line_color, alpha));
            draw->AddRectFilled(ImVec2(x + 6, y + 6), ImVec2(x + width - 6, y + height - 6), PackColor(outer_face_color, alpha));

            const float panel_x = x + 17.0f * scale;
            const float panel_y = y + 44.0f;
            const float panel_r = x + width - 17.0f * scale;
            const float panel_b = y + height - 17.0f * scale;
            draw->AddRectFilled(ImVec2(panel_x, panel_y), ImVec2(panel_r, panel_b), PackColor(panel_outer_line_color, alpha));
            draw->AddRectFilled(ImVec2(panel_x + 1, panel_y + 1), ImVec2(panel_r - 1, panel_b - 1), PackColor(panel_between_color, alpha));
            draw->AddRectFilled(ImVec2(panel_x + 2, panel_y + 2), ImVec2(panel_r - 2, panel_b - 2), PackColor(panel_inner_line_color, alpha));
            draw->AddRectFilled(ImVec2(panel_x + 3, panel_y + 3), ImVec2(panel_r - 3, panel_b - 3), PackColor(panel_inside_color, alpha));

            struct Vertex3 { float x, y, z; };
            struct Triangle3 { Vertex3 v[3]; };
            static std::vector<Triangle3> mesh;
            static bool mesh_loaded = false;
            if (!mesh_loaded)
            {
                mesh_loaded = true;
                const auto model_paths = FindPreviewModels();
                for (const auto& model_path : model_paths)
                {
                    constexpr int grid_x = 48;
                    constexpr int grid_y = 36;
                    constexpr int grid_z = 144;
                    FILE* file = nullptr;
                    if (_wfopen_s(&file, model_path.c_str(), L"rb") != 0 || !file)
                        continue;
                    std::fseek(file, 80, SEEK_SET);
                    unsigned int total = 0;
                    if (std::fread(&total, sizeof(total), 1, file) != 1 || total == 0)
                    {
                        std::fclose(file);
                        continue;
                    }

                    // First pass: get bounds without keeping the 2M-triangle scan in memory.
                    float min_x = FLT_MAX, min_y = FLT_MAX, min_z = FLT_MAX;
                    float max_x = -FLT_MAX, max_y = -FLT_MAX, max_z = -FLT_MAX;
                    unsigned char record[50];
                    for (unsigned int i = 0; i < total; ++i)
                    {
                        if (std::fread(record, sizeof(record), 1, file) != 1)
                            break;
                        Triangle3 tri{};
                        std::memcpy(&tri.v[0], record + 12, 12);
                        std::memcpy(&tri.v[1], record + 24, 12);
                        std::memcpy(&tri.v[2], record + 36, 12);
                        for (const Vertex3& vertex : tri.v)
                        {
                            min_x = std::min(min_x, vertex.x); max_x = std::max(max_x, vertex.x);
                            min_y = std::min(min_y, vertex.y); max_y = std::max(max_y, vertex.y);
                            min_z = std::min(min_z, vertex.z); max_z = std::max(max_z, vertex.z);
                        }
                    }
                    std::fclose(file);

                    const float size_x = std::max(max_x - min_x, 0.001f);
                    const float size_y = std::max(max_y - min_y, 0.001f);
                    const float size_z = std::max(max_z - min_z, 0.001f);
                    const auto cell_index = [&](int ix, int iy, int iz) {
                        return (static_cast<size_t>(iz) * grid_y + iy) * grid_x + ix;
                    };
                    const auto to_cell = [](float value, float minimum, float size, int count) {
                        return std::clamp(static_cast<int>((value - minimum) / size * count), 0, count - 1);
                    };

                    // Second pass: cluster the dense STL vertices into a low-poly mesh.
                    const size_t cell_count = static_cast<size_t>(grid_x) * grid_y * grid_z;
                    std::vector<Vertex3> sums(cell_count, Vertex3{});
                    std::vector<unsigned int> counts(cell_count, 0);
                    if (_wfopen_s(&file, model_path.c_str(), L"rb") == 0 && file)
                    {
                        std::fseek(file, 84, SEEK_SET);
                        for (unsigned int i = 0; i < total; ++i)
                        {
                            if (std::fread(record, sizeof(record), 1, file) != 1)
                                break;
                            Triangle3 tri{};
                            std::memcpy(&tri.v[0], record + 12, 12);
                            std::memcpy(&tri.v[1], record + 24, 12);
                            std::memcpy(&tri.v[2], record + 36, 12);
                            for (const Vertex3& vertex : tri.v)
                            {
                                const int ix = to_cell(vertex.x, min_x, size_x, grid_x);
                                const int iy = to_cell(vertex.y, min_y, size_y, grid_y);
                                const int iz = to_cell(vertex.z, min_z, size_z, grid_z);
                                const size_t index = cell_index(ix, iy, iz);
                                sums[index].x += vertex.x;
                                sums[index].y += vertex.y;
                                sums[index].z += vertex.z;
                                ++counts[index];
                            }
                        }
                        std::fclose(file);
                    }

                    std::vector<Vertex3> representatives(cell_count, Vertex3{});
                    for (size_t index = 0; index < cell_count; ++index)
                        if (counts[index] != 0)
                            representatives[index] = {
                                sums[index].x / counts[index],
                                sums[index].y / counts[index],
                                sums[index].z / counts[index]
                            };

                    if (_wfopen_s(&file, model_path.c_str(), L"rb") == 0 && file)
                    {
                        std::unordered_set<std::uint64_t> faces;
                        faces.reserve(150000);
                        std::fseek(file, 84, SEEK_SET);
                        for (unsigned int i = 0; i < total; ++i)
                        {
                            if (std::fread(record, sizeof(record), 1, file) != 1)
                                break;
                            Triangle3 tri{};
                            std::memcpy(&tri.v[0], record + 12, 12);
                            std::memcpy(&tri.v[1], record + 24, 12);
                            std::memcpy(&tri.v[2], record + 36, 12);
                            std::uint32_t ids[3]{};
                            for (int vertex = 0; vertex < 3; ++vertex)
                            {
                                const int ix = to_cell(tri.v[vertex].x, min_x, size_x, grid_x);
                                const int iy = to_cell(tri.v[vertex].y, min_y, size_y, grid_y);
                                const int iz = to_cell(tri.v[vertex].z, min_z, size_z, grid_z);
                                ids[vertex] = static_cast<std::uint32_t>(cell_index(ix, iy, iz));
                            }
                            if (ids[0] == ids[1] || ids[0] == ids[2] || ids[1] == ids[2])
                                continue;
                            std::uint32_t sorted[3] = { ids[0], ids[1], ids[2] };
                            std::sort(std::begin(sorted), std::end(sorted));
                            const std::uint64_t key = (static_cast<std::uint64_t>(sorted[0]) << 42)
                                | (static_cast<std::uint64_t>(sorted[1]) << 21)
                                | sorted[2];
                            if (!faces.insert(key).second)
                                continue;
                            const Triangle3 reduced = {{ representatives[ids[0]], representatives[ids[1]], representatives[ids[2]] }};
                            mesh.push_back(reduced);
                        }
                        std::fclose(file);
                    }
                }

                // Keep the preview useful on a clean checkout where the optional STL is absent.
                if (mesh.empty())
                {
                    const auto add_box = [&](float x0, float x1, float y0, float y1, float z0, float z1)
                    {
                        const Vertex3 v[8] = {
                            { x0, y0, z0 }, { x1, y0, z0 }, { x1, y1, z0 }, { x0, y1, z0 },
                            { x0, y0, z1 }, { x1, y0, z1 }, { x1, y1, z1 }, { x0, y1, z1 }
                        };
                        const int faces[6][4] = {
                            { 0, 1, 5, 4 }, { 1, 2, 6, 5 }, { 2, 3, 7, 6 },
                            { 3, 0, 4, 7 }, { 4, 5, 6, 7 }, { 3, 2, 1, 0 }
                        };
                        for (const auto& face : faces)
                        {
                            mesh.push_back({ { v[face[0]], v[face[1]], v[face[2]] } });
                            mesh.push_back({ { v[face[0]], v[face[2]], v[face[3]] } });
                        }
                    };

                    // Simple mannequin: head, torso, arms and legs.
                    add_box(-0.30f, 0.30f, -0.28f, 0.28f, 2.45f, 3.05f);
                    add_box(-0.46f, 0.46f, -0.22f, 0.22f, 0.95f, 2.45f);
                    add_box(-0.86f, -0.46f, -0.19f, 0.19f, 1.50f, 2.30f);
                    add_box(0.46f, 0.86f, -0.19f, 0.19f, 1.50f, 2.30f);
                    add_box(-0.98f, -0.86f, -0.17f, 0.17f, 0.95f, 1.50f);
                    add_box(0.86f, 0.98f, -0.17f, 0.17f, 0.95f, 1.50f);
                    add_box(-0.40f, -0.04f, -0.20f, 0.20f, 0.45f, 0.95f);
                    add_box(0.04f, 0.40f, -0.20f, 0.20f, 0.45f, 0.95f);
                    add_box(-0.36f, -0.05f, -0.18f, 0.18f, 0.0f, 0.45f);
                    add_box(0.05f, 0.36f, -0.18f, 0.18f, 0.0f, 0.45f);
                }
            }
            if (!mesh.empty())
            {
                static float min_x = 0, max_x = 0, min_y = 0, max_y = 0, min_z = 0, max_z = 0;
                static bool bounds_ready = false;
                if (!bounds_ready)
                {
                    min_x = min_y = min_z = FLT_MAX;
                    max_x = max_y = max_z = -FLT_MAX;
                    for (const Triangle3& tri : mesh)
                        for (const Vertex3& v : tri.v)
                        {
                            min_x = std::min(min_x, v.x); max_x = std::max(max_x, v.x);
                            min_y = std::min(min_y, v.y); max_y = std::max(max_y, v.y);
                            min_z = std::min(min_z, v.z); max_z = std::max(max_z, v.z);
                        }
                    bounds_ready = true;
                }
                const float cx = (min_x + max_x) * 0.5f;
                const float cy = (min_y + max_y) * 0.5f;
                const float cz = (min_z + max_z) * 0.5f;
                const float extent = std::max({ max_x - min_x, max_y - min_y, max_z - min_z, 0.001f });
                const float view_w = panel_r - panel_x - 12.0f;
                const float view_h = panel_b - panel_y - 12.0f;
                const float fit = std::min(view_w, view_h) * 0.95f * state.preview_zoom / extent;
                const float model_center_x = (panel_x + panel_r) * 0.5f;
                const float model_center_y = (panel_y + panel_b) * 0.5f;
                const double now = ImGui::GetTime();
                const bool rotate_automatically = !state.preview_rotation_paused && !state.preview_rotating && now - state.preview_last_rotate_time >= 2.0;
                const float angle = state.preview_yaw + 3.14159265f + (rotate_automatically ? static_cast<float>((now - state.preview_last_rotate_time - 2.0) * 0.3) : 0.0f);
                const float tilt = std::clamp(state.preview_pitch, -0.65f, 0.65f);
                struct Projected { ImVec2 p[3]; float depth; float shade; float specular; };
                std::vector<Projected> projected;
                projected.reserve(mesh.size());
                for (const Triangle3& tri : mesh)
                {
                    Projected out{};
                    const float ab_x = tri.v[1].x - tri.v[0].x;
                    const float ab_y = tri.v[1].y - tri.v[0].y;
                    const float ab_z = tri.v[1].z - tri.v[0].z;
                    const float ac_x = tri.v[2].x - tri.v[0].x;
                    const float ac_y = tri.v[2].y - tri.v[0].y;
                    const float ac_z = tri.v[2].z - tri.v[0].z;
                    const float nx = ab_y * ac_z - ab_z * ac_y;
                    const float ny = ab_z * ac_x - ab_x * ac_z;
                    const float nz = ab_x * ac_y - ab_y * ac_x;
                    const float normal_length = std::sqrt(nx * nx + ny * ny + nz * nz);
                    const float light_x = 0.35f;
                    const float light_y = -0.55f;
                    const float light_z = 0.76f;
                    const float view_x = 0.0f;
                    const float view_y = 0.0f;
                    const float view_z = 1.0f;
                    const float normal_x = normal_length > 0.0001f ? nx / normal_length : 0.0f;
                    const float normal_y = normal_length > 0.0001f ? ny / normal_length : 0.0f;
                    const float normal_z = normal_length > 0.0001f ? nz / normal_length : 0.0f;
                    const float diffuse = std::max(0.0f, normal_x * light_x + normal_y * light_y + normal_z * light_z);
                    const float half_x = light_x + view_x;
                    const float half_y = light_y + view_y;
                    const float half_z = light_z + view_z;
                    const float half_length = std::sqrt(half_x * half_x + half_y * half_y + half_z * half_z);
                    const float specular_base = half_length > 0.0001f ? std::max(0.0f, normal_x * half_x / half_length + normal_y * half_y / half_length + normal_z * half_z / half_length) : 0.0f;
                    out.shade = 0.32f + diffuse * 0.68f;
                    out.specular = std::pow(specular_base, 28.0f);
                    for (int i = 0; i < 3; ++i)
                    {
                        const float vx = tri.v[i].x - cx;
                        const float vy = tri.v[i].y - cy;
                        const float vz = tri.v[i].z - cz;
                        const float rx = vx * std::cos(angle) - vy * std::sin(angle);
                        const float depth = vx * std::sin(angle) + vy * std::cos(angle);
                        const float screen_y = vz * std::cos(tilt) - depth * std::sin(tilt);
                        out.p[i] = ImVec2(model_center_x + rx * fit, model_center_y - screen_y * fit);
                        out.depth += vz * std::sin(tilt) + depth * std::cos(tilt);
                    }
                    projected.push_back(out);
                }
                std::sort(projected.begin(), projected.end(), [](const Projected& a, const Projected& b) { return a.depth < b.depth; });
                const float bounds_padding_left = state.player_enemy_box ? (state.player_health ? 5.0f : 2.0f) : 0.0f;
                const float bounds_padding_right = state.player_enemy_box && state.player_flags && state.player_flags_mask ? 46.0f : state.player_enemy_box ? 2.0f : 0.0f;
                const float bounds_padding_top = state.player_enemy_box ? (state.player_name ? 18.0f : 2.0f) + (state.player_flags && (state.player_flags_mask & 1u) ? 10.0f : 0.0f) + (state.player_flags && (state.player_flags_mask & 2u) ? 10.0f : 0.0f) + (state.player_flags && (state.player_flags_mask & 4u) ? 10.0f : 0.0f) : 0.0f;
                const float bounds_padding_bottom = state.player_enemy_box ? (state.player_distance ? 18.0f : 2.0f) + (state.player_weapon && state.player_weapon_display_mask ? (state.player_weapon_display_mask == 3u ? 32.0f : 16.0f) : 0.0f) : 0.0f;
                IM_UNUSED(bounds_padding_left);
                IM_UNUSED(bounds_padding_right);
                IM_UNUSED(bounds_padding_top);
                IM_UNUSED(bounds_padding_bottom);
                draw->PushClipRect(ImVec2(panel_x + 4, panel_y + 4), ImVec2(panel_r - 4, panel_b - 4), true);
                ImVec2 bounds_min(panel_r, panel_b);
                ImVec2 bounds_max(panel_x, panel_y);
                for (const Projected& tri : projected)
                {
                    const float specular = tri.specular * 0.8f;
                    const int red = static_cast<int>(std::clamp(state.accent[0] * tri.shade + specular, 0.0f, 1.0f) * 255.0f);
                    const int green = static_cast<int>(std::clamp(state.accent[1] * tri.shade + specular, 0.0f, 1.0f) * 255.0f);
                    const int blue = static_cast<int>(std::clamp(state.accent[2] * tri.shade + specular, 0.0f, 1.0f) * 255.0f);
                    draw->AddTriangleFilled(tri.p[0], tri.p[1], tri.p[2], IM_COL32(red, green, blue, 255));
                    for (const ImVec2& point : tri.p)
                    {
                        bounds_min.x = std::min(bounds_min.x, point.x);
                        bounds_min.y = std::min(bounds_min.y, point.y);
                        bounds_max.x = std::max(bounds_max.x, point.x);
                        bounds_max.y = std::max(bounds_max.y, point.y);
                    }
                }
                struct EspAnchors
                {
                    ImVec2 top_center;
                    ImVec2 bottom_center;
                    ImVec2 left_center;
                    ImVec2 right_top;
                };
                const EspAnchors anchors{
                    ImVec2((bounds_min.x + bounds_max.x) * 0.5f, bounds_min.y),
                    ImVec2((bounds_min.x + bounds_max.x) * 0.5f, bounds_max.y),
                    ImVec2(bounds_min.x, (bounds_min.y + bounds_max.y) * 0.5f),
                    ImVec2(bounds_max.x, bounds_min.y)
                };
                if (false)
                {
                ImVec2* drag_offset = nullptr;
                if (state.player_name)
                    drag_offset = &state.preview_name_offset;
                if (state.preview_anchor_drag < 0 && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    const ImVec2 mouse = ImGui::GetIO().MousePos;
                    const char* name_text = "neegy[fagghotty]";
                    const ImVec2 name_size = state.brand_font ? state.brand_font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, name_text) : ImVec2(70.0f, 13.0f);
                    const ImVec2 name_pos(anchors.top_center.x - name_size.x * 0.5f + state.preview_name_offset.x, anchors.top_center.y - name_size.y - 1.0f + state.preview_name_offset.y);
                    const ImVec2 name_hit(name_pos.x - 2.0f, name_pos.y - 2.0f);
                    const ImVec2 health_hit(anchors.left_center.x - 8.0f + state.preview_health_offset.x, anchors.left_center.y - 35.0f + state.preview_health_offset.y);
                    const ImVec2 flags_hit(anchors.right_top.x + state.preview_flags_offset.x, anchors.right_top.y - 4.0f + state.preview_flags_offset.y);
                    const ImVec2 weapon_hit(anchors.bottom_center.x - 30.0f + state.preview_weapon_offset.x, anchors.bottom_center.y + state.preview_weapon_offset.y);
                    const ImVec2 distance_hit(anchors.bottom_center.x - 30.0f + state.preview_distance_offset.x, anchors.bottom_center.y + 14.0f + state.preview_distance_offset.y);
                    const auto inside = [&](ImVec2 min, ImVec2 max) { return mouse.x >= min.x && mouse.x <= max.x && mouse.y >= min.y && mouse.y <= max.y; };
                    if (state.player_name && inside(name_hit, ImVec2(name_pos.x + name_size.x + 2.0f, name_pos.y + name_size.y + 2.0f))) state.preview_anchor_drag = 0;
                    else if (state.player_health && inside(health_hit, ImVec2(health_hit.x + 8.0f, health_hit.y + 70.0f))) state.preview_anchor_drag = 1;
                    else if (state.player_flags && inside(flags_hit, ImVec2(flags_hit.x + 70.0f, flags_hit.y + 30.0f))) state.preview_anchor_drag = 2;
                    else if (state.player_weapon && inside(weapon_hit, ImVec2(weapon_hit.x + 70.0f, weapon_hit.y + 16.0f))) state.preview_anchor_drag = 3;
                    else if (state.player_distance && inside(distance_hit, ImVec2(distance_hit.x + 70.0f, distance_hit.y + 16.0f))) state.preview_anchor_drag = 4;
                    if (state.preview_anchor_drag >= 0)
                    {
                        static ImVec2* offsets[] = { &state.preview_name_offset, &state.preview_health_offset, &state.preview_flags_offset, &state.preview_weapon_offset, &state.preview_distance_offset };
                        drag_offset = offsets[state.preview_anchor_drag];
                        state.preview_anchor_drag_start = mouse;
                        state.preview_anchor_offset_start = *drag_offset;
                    }
                }
                if (state.preview_anchor_drag >= 0 && ImGui::IsMouseDown(ImGuiMouseButton_Left))
                {
                    static ImVec2* offsets[] = { &state.preview_name_offset, &state.preview_health_offset, &state.preview_flags_offset, &state.preview_weapon_offset, &state.preview_distance_offset };
                    drag_offset = offsets[state.preview_anchor_drag];
                    const ImVec2 delta(ImGui::GetIO().MousePos.x - state.preview_anchor_drag_start.x, ImGui::GetIO().MousePos.y - state.preview_anchor_drag_start.y);
                    const float box_width = bounds_max.x - bounds_min.x;
                    const float box_height = bounds_max.y - bounds_min.y;
                    const float x_limit = std::max(0.0f, box_width * 0.5f - 8.0f);
                    const float y_limit = std::max(0.0f, box_height * 0.5f - 8.0f);
                    ImVec2 next(std::round(state.preview_anchor_offset_start.x + delta.x), std::round(state.preview_anchor_offset_start.y + delta.y));
                    if (state.preview_anchor_drag == 0)
                    {
                        next.x = std::clamp(next.x, -x_limit, x_limit);
                        next.y = std::round(next.y);
                    }
                    else if (state.preview_anchor_drag == 1 || state.preview_anchor_drag == 2)
                    {
                        next.x = 0.0f;
                        next.y = std::clamp(next.y, -y_limit, y_limit);
                    }
                    else
                    {
                        next.x = std::clamp(next.x, -x_limit, x_limit);
                        next.y = std::clamp(next.y, -box_height - 24.0f, box_height + 24.0f);
                    }
                    *drag_offset = next;
                }
                if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) && state.preview_anchor_drag >= 0)
                {
                    const int dragged = state.preview_anchor_drag;
                    const ImVec2 mouse = ImGui::GetIO().MousePos;
                    const float box_w = bounds_max.x - bounds_min.x;
                    const float box_h = bounds_max.y - bounds_min.y;
                    const auto slot_pos = [&](int slot, float width, float height) {
                        const int side = slot / 3;
                        const int part = slot % 3;
                        const float t = (part + 1.0f) / 4.0f;
                        if (side == 0)
                            return ImVec2(bounds_min.x + box_w * t - width * 0.5f, bounds_min.y - height - 1.0f);
                        if (side == 1)
                            return ImVec2(bounds_max.x + 1.0f, bounds_min.y + box_h * t - height * 0.5f);
                        if (side == 2)
                            return ImVec2(bounds_min.x + box_w * t - width * 0.5f, bounds_max.y + 1.0f);
                        return ImVec2(bounds_min.x - width - 1.0f, bounds_min.y + box_h * t - height * 0.5f);
                    };
                    const auto occupied = [&](int slot, int except) {
                        const int slots[] = { state.preview_name_slot, state.preview_health_slot, state.preview_flags_slot, state.preview_weapon_slot, state.preview_distance_slot };
                        for (int i = 0; i < 5; ++i)
                            if (i != except && slots[i] == slot)
                                return true;
                        return false;
                    };
                    const float element_w[] = { 74.0f, 4.0f, 48.0f, 36.0f, 32.0f };
                    const float element_h[] = { 13.0f, box_h, 11.0f, 11.0f, 13.0f };
                    const ImVec2 base[] = {
                        ImVec2(anchors.top_center.x - element_w[0] * 0.5f, anchors.top_center.y - element_h[0] - 1.0f),
                        ImVec2(anchors.left_center.x - 3.0f, bounds_min.y),
                        ImVec2(anchors.right_top.x + 2.0f, anchors.right_top.y - 2.0f),
                        ImVec2(anchors.bottom_center.x - element_w[3] * 0.5f, anchors.bottom_center.y + 1.0f),
                        ImVec2(anchors.bottom_center.x - element_w[4] * 0.5f, anchors.bottom_center.y + 1.0f)
                    };
                    int best_slot = -1;
                    float best_dist = FLT_MAX;
                    for (int slot = 0; slot < 12; ++slot)
                    {
                        if (occupied(slot, dragged))
                            continue;
                        const ImVec2 pos = slot_pos(slot, element_w[dragged], element_h[dragged]);
                        const ImVec2 center(pos.x + element_w[dragged] * 0.5f, pos.y + element_h[dragged] * 0.5f);
                        const float dx = mouse.x - center.x;
                        const float dy = mouse.y - center.y;
                        const float dist = dx * dx + dy * dy;
                        if (dist < best_dist)
                        {
                            best_dist = dist;
                            best_slot = slot;
                        }
                    }
                    if (best_slot >= 0)
                    {
                        const ImVec2 pos = slot_pos(best_slot, element_w[dragged], element_h[dragged]);
                        const ImVec2 offset(pos.x - base[dragged].x, pos.y - base[dragged].y);
                        if (dragged == 0) { state.preview_name_slot = best_slot; state.preview_name_offset = offset; }
                        if (dragged == 1) { state.preview_health_slot = best_slot; state.preview_health_offset = offset; }
                        if (dragged == 2) { state.preview_flags_slot = best_slot; state.preview_flags_offset = offset; }
                        if (dragged == 3) { state.preview_weapon_slot = best_slot; state.preview_weapon_offset = offset; }
                        if (dragged == 4) { state.preview_distance_slot = best_slot; state.preview_distance_offset = offset; }
                    }
                    state.preview_anchor_drag = -1;
                    state.preview_last_rotate_time = ImGui::GetTime();
                    state.preview_rotating = false;
                }
                }
                const ImVec2 name_offset(0.0f, 0.0f);
                const ImVec2 health_offset(0.0f, 0.0f);
                const ImVec2 flags_offset(0.0f, 0.0f);
                const ImVec2 weapon_offset(0.0f, 0.0f);
                const ImVec2 distance_offset(0.0f, 0.0f);
                if (state.player_enemy_box)
                {
                    const ImU32 box_color = IM_COL32(static_cast<int>(state.player_enemy_box_color[0] * 255.0f), static_cast<int>(state.player_enemy_box_color[1] * 255.0f), static_cast<int>(state.player_enemy_box_color[2] * 255.0f), static_cast<int>(alpha * state.player_enemy_box_color[3] * 255.0f));
                    const ImU32 box_outline = IM_COL32(0, 0, 0, static_cast<int>(state.player_enemy_box_color[3] * 255.0f));
                    const ImVec2 outer_min(bounds_min.x - 1.0f, bounds_min.y - 1.0f);
                    const ImVec2 outer_max(bounds_max.x + 1.0f, bounds_max.y + 1.0f);
                    if (state.player_box_type == 0)
                    {
                        draw->AddRect(outer_min, outer_max, box_outline, 0.0f, 0, 1.0f);
                        draw->AddRect(bounds_min, bounds_max, box_color, 0.0f, 0, 1.0f);
                        draw->AddRect(ImVec2(bounds_min.x + 1.0f, bounds_min.y + 1.0f), ImVec2(bounds_max.x - 1.0f, bounds_max.y - 1.0f), box_outline, 0.0f, 0, 1.0f);
                    }
                    else
                    {
                        const float w = outer_max.x - outer_min.x;
                        const float h = outer_max.y - outer_min.y;
                        const float cw = std::max(3.0f, w * 0.28f);
                        const float ch = std::max(3.0f, h * 0.14f);
                        const ImVec2 corners[16] = {
                            outer_min, ImVec2(outer_min.x + cw, outer_min.y), outer_min, ImVec2(outer_min.x, outer_min.y + ch),
                            ImVec2(outer_max.x - cw, outer_min.y), ImVec2(outer_max.x, outer_min.y), ImVec2(outer_max.x, outer_min.y), ImVec2(outer_max.x, outer_min.y + ch),
                            ImVec2(outer_min.x, outer_max.y - ch), ImVec2(outer_min.x, outer_max.y), ImVec2(outer_min.x, outer_max.y), ImVec2(outer_min.x + cw, outer_max.y),
                            ImVec2(outer_max.x - cw, outer_max.y), outer_max, ImVec2(outer_max.x, outer_max.y - ch), outer_max
                        };
                        for (int i = 0; i < 16; i += 2)
                            draw->AddLine(corners[i], corners[i + 1], box_outline, 3.0f);
                        for (int i = 0; i < 16; i += 2)
                            draw->AddLine(corners[i], corners[i + 1], box_color, 1.0f);
                    }
                    if (state.player_health)
                    {
                        const float health = std::clamp(0.62f + std::sin(static_cast<float>(ImGui::GetTime()) * 1.4f) * 0.28f, 0.08f, 0.92f);
                         const float bar_x = anchors.left_center.x - 3.0f + health_offset.x;
                         const float bar_top = bounds_min.y + health_offset.y;
                         const float bar_bottom = bounds_max.y + health_offset.y;
                        draw->AddRect(ImVec2(bar_x - 1.0f, bar_top), ImVec2(bar_x + 3.0f, bar_bottom), IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.0f);
                        const float fill_top = bar_bottom - (bar_bottom - bar_top) * health;
                        const float fill_mid = fill_top + (bar_bottom - fill_top) * 0.45f;
                        draw->AddRectFilledMultiColor(ImVec2(bar_x, fill_top), ImVec2(bar_x + 2.0f, fill_mid), IM_COL32(255, 255, 255, 255), IM_COL32(255, 255, 255, 255), IM_COL32(0, 190, 255, 255), IM_COL32(0, 190, 255, 255));
                        draw->AddRectFilled(ImVec2(bar_x, fill_mid), ImVec2(bar_x + 2.0f, bar_bottom), IM_COL32(0, 190, 255, 255));
                        if (state.brand_font)
                        {
                            char health_text[8];
                            std::snprintf(health_text, sizeof(health_text), "%d", static_cast<int>(health * 100.0f));
                            const ImVec2 text_size = state.brand_font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, health_text);
                            const float text_y = fill_top - text_size.y * 0.5f;
                            const ImVec2 text_pos(bar_x - text_size.x - 2.0f, text_y);
                            const ImU32 outline = IM_COL32(0, 0, 0, 255);
                            draw->AddText(state.brand_font, 13.0f, ImVec2(text_pos.x - 1.0f, text_pos.y), outline, health_text);
                            draw->AddText(state.brand_font, 13.0f, ImVec2(text_pos.x + 1.0f, text_pos.y), outline, health_text);
                            draw->AddText(state.brand_font, 13.0f, ImVec2(text_pos.x, text_pos.y - 1.0f), outline, health_text);
                            draw->AddText(state.brand_font, 13.0f, ImVec2(text_pos.x, text_pos.y + 1.0f), outline, health_text);
                            draw->AddText(state.brand_font, 13.0f, text_pos, IM_COL32(255, 255, 255, 255), health_text);
                        }
                    }
                    if (state.player_flags && state.esp_font)
                    {
                     float flag_y = std::round(anchors.right_top.y - 2.0f + flags_offset.y);
                     const float flag_x = anchors.right_top.x + 2.0f + flags_offset.x;
                    const double flag_time = ImGui::GetTime();
                    if (state.player_flags_mask & 1u)
                    {
                        static float money = 228.0f;
                        money = 782.0f + std::sin(static_cast<float>(flag_time) * 0.45f) * 554.0f;
                        char money_text[32];
                        std::snprintf(money_text, sizeof(money_text), "$%.0f", money);
                         draw->AddText(state.esp_font, 11.0f, ImVec2(flag_x, flag_y), IM_COL32(255, 255, 255, 255), money_text);
                         flag_y += state.esp_font->CalcTextSizeA(11.0f, FLT_MAX, 0.0f, money_text).y + 1.0f;
                    }
                    if (state.player_flags_mask & 2u)
                    {
                        constexpr char name[] = "nigger";
                        const float cycle = std::fmod(static_cast<float>(flag_time), 3.0f);
                        const int count = cycle < 1.5f ? static_cast<int>(cycle * 4.0f + 0.5f) : 6 - static_cast<int>((cycle - 1.5f) * 4.0f + 0.5f);
                        char shown[sizeof(name)]{};
                        std::memcpy(shown, name, std::clamp(count, 0, 6));
                         draw->AddText(state.esp_font, 11.0f, ImVec2(flag_x, flag_y), IM_COL32(255, 255, 255, 255), shown);
                         flag_y += state.esp_font->CalcTextSizeA(11.0f, FLT_MAX, 0.0f, shown).y + 1.0f;
                    }
                    if (state.player_flags_mask & 4u)
                    {
                        draw->AddText(state.esp_font, 11.0f, ImVec2(flag_x, flag_y), IM_COL32(168, 180, 237, static_cast<int>(alpha * 255.0f)), "flexxx");
                    }
                    }
                    float stack_y = anchors.bottom_center.y + 1.0f;
                    if (state.player_distance && state.brand_font)
                    {
                         const float distance = (std::sin(static_cast<float>(ImGui::GetTime()) * 0.7f) + 1.0f) * 50.0f;
                         char distance_text[16];
                         std::snprintf(distance_text, sizeof(distance_text), "%.0fm", distance);
                         const ImVec2 text_size = state.brand_font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, distance_text);
                          const float distance_y = stack_y + distance_offset.y;
                          const ImVec2 text_pos(std::round(anchors.bottom_center.x - text_size.x * 0.5f + distance_offset.x), std::round(distance_y));
                        const ImU32 outline = IM_COL32(0, 0, 0, static_cast<int>(state.player_distance_color[3] * 255.0f));
                        draw->AddText(state.brand_font, 13.0f, ImVec2(text_pos.x - 1.0f, text_pos.y), outline, distance_text);
                        draw->AddText(state.brand_font, 13.0f, ImVec2(text_pos.x + 1.0f, text_pos.y), outline, distance_text);
                        draw->AddText(state.brand_font, 13.0f, ImVec2(text_pos.x, text_pos.y - 1.0f), outline, distance_text);
                        draw->AddText(state.brand_font, 13.0f, ImVec2(text_pos.x, text_pos.y + 1.0f), outline, distance_text);
                        const ImU32 distance_color = IM_COL32(static_cast<int>(state.player_distance_color[0] * 255.0f), static_cast<int>(state.player_distance_color[1] * 255.0f), static_cast<int>(state.player_distance_color[2] * 255.0f), static_cast<int>(alpha * state.player_distance_color[3] * 255.0f));
                         draw->AddText(state.brand_font, 13.0f, text_pos, distance_color, distance_text);
                          stack_y = distance_y + text_size.y + 1.0f;
                    }
                    if (state.player_weapon && state.esp_font && state.player_weapon_display_mask != 0)
                    {
                        constexpr float item_font_size = 11.0f;
                        const char* weapon_name = "AK-47";
                        const ImVec2 weapon_size = state.esp_font->CalcTextSizeA(item_font_size, FLT_MAX, 0.0f, weapon_name);
                         const float weapon_y = stack_y + weapon_offset.y;
                         draw->AddText(state.esp_font, item_font_size, ImVec2(std::round(anchors.bottom_center.x - weapon_size.x * 0.5f + weapon_offset.x), std::round(weapon_y)), IM_COL32(255, 255, 255, 255), weapon_name);
                    }
                    if (state.player_name && state.brand_font)
                    {
                     const char* name = "neegy[fagghotty]";
                     const ImVec2 name_size = state.brand_font->CalcTextSizeA(13.0f, FLT_MAX, 0.0f, name);
                       const ImVec2 name_pos(std::round(anchors.top_center.x - name_size.x * 0.5f + name_offset.x), std::round(anchors.top_center.y - name_size.y - 1.0f + name_offset.y));
                        const ImU32 name_color = IM_COL32(static_cast<int>(state.player_name_color[0] * 255.0f), static_cast<int>(state.player_name_color[1] * 255.0f), static_cast<int>(state.player_name_color[2] * 255.0f), static_cast<int>(alpha * state.player_name_color[3] * 255.0f));
                        draw->AddText(state.brand_font, 13.0f, name_pos, name_color, name);
                    }
                }
                draw->PopClipRect();
            }

            const float title_width = ImGui::CalcTextSize("esp preview").x;
            const float title_x = x + (width - title_width) * 0.5f;
            draw->AddText(ImVec2(title_x, y + 16.0f), PackColor(tab_text_color, alpha), "esp preview");
            const float accent_x = x + 16.0f * scale;
            const float accent_r = x + width - 16.0f * scale;
            const float accent_y = y + 35.0f;
            draw->AddRectFilledMultiColor(ImVec2(accent_x, accent_y), ImVec2(accent_r, accent_y + 5.0f), PackColor(checkbox_on_color, alpha), PackColor(checkbox_on_color, alpha), PackColor(checkbox_on_shade_color, alpha), PackColor(checkbox_on_shade_color, alpha));
            draw->AddRectFilledMultiColor(ImVec2(accent_x, accent_y + 5.0f), ImVec2(accent_r, accent_y + 6.0f), PackColor(tab_line_shadow_color, alpha), PackColor(tab_line_shadow_color, alpha), PackColor(tab_line_black_color, alpha), PackColor(tab_line_black_color, alpha));

            ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
            ImGui::SetNextWindowBgAlpha(0.0f);
            ImGui::Begin("esp_preview_move", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoNav);
            ImGui::SetCursorScreenPos(ImVec2(panel_x + 4, panel_y + 4));
            ImGui::InvisibleButton("model_rotate", ImVec2(panel_r - panel_x - 8, panel_b - panel_y - 8));
            if (ImGui::IsItemHovered() && ImGui::GetIO().MouseWheel != 0.0f)
                state.preview_zoom = std::clamp(state.preview_zoom + ImGui::GetIO().MouseWheel * 0.1f, 0.5f, 3.0f);
            if (ImGui::IsItemClicked())
            {
                const double now = ImGui::GetTime();
                if (!state.preview_rotation_paused)
                {
                    if (now - state.preview_last_rotate_time >= 2.0)
                        state.preview_yaw += static_cast<float>((now - state.preview_last_rotate_time - 2.0) * 0.3);
                    state.preview_last_rotate_time = now;
                    state.preview_rotation_paused = true;
                }
                else
                {
                    state.preview_last_rotate_time = now;
                    state.preview_rotation_paused = false;
                }
            }
            if (ImGui::IsItemActivated())
            {
                state.preview_rotating = !state.preview_rotation_paused;
                state.preview_rotate_start = ImGui::GetIO().MousePos;
                state.preview_yaw_start = state.preview_yaw;
                state.preview_pitch_start = state.preview_pitch;
            }
            if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
                state.preview_rotating = false;
            if (state.preview_rotating)
            {
                const ImVec2 delta(ImGui::GetIO().MousePos.x - state.preview_rotate_start.x, ImGui::GetIO().MousePos.y - state.preview_rotate_start.y);
                state.preview_yaw = state.preview_yaw_start + delta.x * 0.01f;
                state.preview_pitch = std::clamp(state.preview_pitch_start + delta.y * 0.01f, -0.65f, 0.85f);
                state.preview_last_rotate_time = ImGui::GetTime();
            }
            ImGui::SetCursorScreenPos(ImVec2(x, y));
            ImGui::InvisibleButton("preview_drag", ImVec2(width, 42.0f * scale));
            if (ImGui::IsItemActivated())
            {
                state.preview_dragging = true;
                state.preview_drag_offset = ImVec2(ImGui::GetIO().MousePos.x - x, ImGui::GetIO().MousePos.y - y);
            }
            if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
                state.preview_dragging = false;
            if (state.preview_dragging)
            {
                state.preview_pos.x = std::clamp(ImGui::GetIO().MousePos.x - state.preview_drag_offset.x, 0.0f, std::max(0.0f, ImGui::GetIO().DisplaySize.x - width));
                state.preview_pos.y = std::clamp(ImGui::GetIO().MousePos.y - state.preview_drag_offset.y, 0.0f, std::max(0.0f, ImGui::GetIO().DisplaySize.y - height));
            }
            ImGui::End();
        }

        void DrawLegit(Canvas& c, UiState& s)
        {
            c.BeginColumn(false);
            c.BeginGroup("aimbot");
            c.Check("enable", s.legit_enable);
            c.Check("aimbot", s.legit_aimbot);
            if (s.legit_aimbot)
            {
                c.Key("key", s.aim_key);
                c.Slider("fov", s.fov, 1, 50, "°");
                c.Slider("threshold", s.threshold, 0, 80, "°");
                c.Slider("retarget time", s.retarget, 0, 1500, "ms");
                c.Slider("speed", s.speed, 1, 100, "%");
                c.Label("hitbox");
                static const char* hitboxes[] = { "head", "chest", "pelvis", "legs", "arms" };
                for (int i = 0; i < 5; ++i)
                    c.Check(hitboxes[i], s.hitboxes[i]);
                c.Check("target friendlies", s.legit_friendlies);
                c.Check("auto fire", s.auto_fire);
            }
            c.Check("rcs", s.legit_rcs);
            if (s.legit_rcs)
            {
                c.Check("standalone", s.small_visualize);
                c.Slider("pitch amount", s.threshold, 0, 100, "%");
                c.Slider("yaw amount", s.speed, 0, 100, "%");
            }
            c.EndGroup();
            c.BeginGroup("backtracking");
            c.Check("enable", s.legit_backtrack);
            if (s.legit_backtrack)
            {
                c.Check(nullptr, s.small_visualize, true);
                c.Label("visualize color");
                c.Slider("range", s.backtrack_range, 1, 200, "ms");
            }
            c.EndGroup();
            c.BeginColumn(true);
            c.BeginGroup("misc");
            c.Check("triggerbot", s.legit_trigger);
            if (s.legit_trigger)
            {
                c.Key("key", s.trigger_key);
                c.Check("target friendlies", s.trigger_friendlies);
                c.Check("scope check", s.trigger_scope);
                c.Check("smoke check", s.trigger_smoke);
                c.Slider("delay", s.trigger_delay, 0, 2500, "ms");
            }
            c.EndGroup();
        }

        void DrawRage(Canvas& c, UiState& s)
        {
            c.BeginColumn(false);
            c.BeginGroup("aimbot");
            c.Check("enable", s.rage_enable);
            c.Key("key", s.aim_key);
            c.Check("silent", s.rage_silent);
            c.Check("auto fire", s.rage_auto_fire);
            static const std::vector<const char*> aim_modes = { "normal", "priority baim", "agressive baim" };
            static const std::vector<const char*> target = { "fov", "distance", "lag", "health", "cycle" };
            static int mode = 0, selection = 0, auto_mode = 0;
            if (s.legit_aimbot)
                c.Dropdown("", auto_mode, { "normal", "force send on shot", "force choke on shot", "avoid animation update" });
            c.Dropdown("mode", mode, aim_modes);
            c.Dropdown("target selection", selection, target);
            c.Slider("minimum damage", s.minimum_damage, 1, 100, "hp");
            c.Check("autowall", s.autowall);
            c.Check("save fps", s.save_fps);
            c.EndGroup();
            c.BeginGroup("accuracy");
            c.Check("no recoil", s.no_recoil);
            static int spread = 0, resolver = 0;
            if (!s.restrictions)
                c.Dropdown("no spread", spread, { "off", "pitch/ yaw", "roll" });
            c.Dropdown("resolver", resolver, { "off", "spread", "no spread", "anti boss fix" }, true, &s.resolver_mask);
            c.Slider("max misses", s.max_misses, 0, 4);
            c.Check("baim fakewalk", s.rage_baim_fakewalk);
            c.Check("hitchance", s.hitchance_enabled);
            if (s.hitchance_enabled)
                c.Slider("", s.hitchance, 0, 100, "%");
            c.EndGroup();
            c.BeginColumn(true);
            c.BeginGroup("anti hit");
            c.Check("fake lag", s.fake_lag);
            if (s.fake_lag)
                c.Dropdown("", mode, { "adaptive", "passive", "random" });
            c.Check("anti aim", s.anti_aim);
            c.EndGroup();
        }

        void DrawPlayers(Canvas& c, UiState& s)
        {
            c.BeginColumn(false);
            c.BeginGroup("esp");
            static int esp_mode = 0;
            static int flags_mode = 0;
            c.Dropdown("esp mode", esp_mode, { "off", "enemy only", "all" });
            c.Check("enemy box", s.player_enemy_box);
            c.ColorPicker("enemy_box_color", s.player_enemy_box_color);
            c.Check("draw team", s.player_friendly_box);
            c.Dropdown("box type", s.player_box_type, { "bounding", "cornered" });
            c.Check("distance", s.player_distance);
            c.ColorPicker("distance_color", s.player_distance_color);
            c.Check("name", s.player_name);
            c.ColorPicker("name_color", s.player_name_color);
            c.Check("health", s.player_health);
            c.Check("weapon", s.player_weapon);
            static int weapon_display = 0;
            c.Dropdown("weapon display", weapon_display, { "icon", "text" }, true, &s.player_weapon_display_mask);
            c.Check("ammo", s.player_ammo);
            c.Check("skeleton", s.player_skeleton);
            c.Check("dormant", s.player_dormant);
            c.Check("flags", s.player_flags);
            c.Dropdown("flags", flags_mode, { "money", "nigger", "flex" }, true, &s.player_flags_mask);
            c.EndGroup();
            c.BeginColumn(true);
            c.BeginGroup("other");
            c.Check("reveal on radar", s.player_reveal_radar);
            c.Check("enemy chams", s.player_enemy_chams);
            c.Check("enemy xqz", s.player_enemy_xqz);
            c.Check("enemy glow", s.player_enemy_glow);
            c.Check("friendly chams", s.player_friendly_chams);
            c.Check("friendly xqz", s.player_friendly_xqz);
            c.Check("friendly glow", s.player_friendly_glow);
            c.Slider("reflectivity", s.glow_reflectivity, 0, 100);
            c.Slider("translucency", s.glow_translucency, 0, 100);
            c.Slider("shine", s.glow_shine, 0, 100);
            c.EndGroup();
        }

        void DrawVisuals(Canvas& c, UiState& s)
        {
            c.BeginColumn(false);
            c.BeginGroup("esp");
            c.Check("enable", s.visuals_enable);
            c.Check("dropped weapons", s.visuals_dropped_weapons);
            c.Check("projectiles", s.visuals_projectiles);
            c.Check("bomb", s.visuals_bomb);
            c.Check("other entities", s.visuals_other_entities);
            c.EndGroup();
            c.BeginColumn(true);
            c.BeginGroup("effects");
            c.Check("no visual recoil", s.no_recoil);
            c.Check("no smoke", s.visuals_no_smoke);
            c.Check("no flash", s.visuals_no_flash);
            c.Check("no fog", s.visuals_no_fog);
            c.Check("no scope", s.visuals_no_scope);
            c.Check("no post processing", s.visuals_no_post_processing);
            static int render = 0;
            c.Dropdown("render mode", render, { "off", "full bright", "low quality", "night mode" });
            c.Slider("wall alpha", s.wall_alpha, 0, 100);
            c.Check("custom world color", s.visuals_custom_world_color);
            c.Slider("fov", s.view_fov, 0, 89, "°");
            if (s.view_fov > 0)
                c.Check("while scoped", s.legit_aimbot);
            c.EndGroup();
            c.BeginGroup("other");
            c.Check("spectator list", s.spectator_list);
            c.Check("grenade prediction", s.grenade_prediction);
            c.Check("recoil crosshair", s.recoil_crosshair);
            c.Check("shot tracers", s.shot_tracers);
            c.Dropdown("indicate flags", render, { "lower body yaw", "lag compensation", "nospread" });
            c.Key("third person", s.trigger_key);
            c.EndGroup();
        }

        void DrawMisc(Canvas& c, UiState& s)
        {
            c.BeginColumn(false);
            c.BeginGroup("movement");
            c.Check("auto bhop", s.auto_bhop);
            c.Check("auto strafe", s.auto_strafe);
            if (s.auto_strafe)
            {
                c.Key("circle strafe", s.aim_key);
                c.Key("snake strafe", s.trigger_key);
            }
            c.EndGroup();
            c.BeginColumn(true);
            c.BeginGroup("matchmaking");
            c.Check("auto accept competitive matches", s.auto_accept);
            c.Check("reveal competitive ranks", s.rank_reveal);
            c.Check("unlock inventory access", s.unlock_inventory);
            c.EndGroup();
            c.BeginGroup("misc");
            c.Check("hitmarker", s.hitmarker);
            c.Check("auto defuse", s.auto_defuse);
            if (s.auto_defuse)
                c.Key("key", s.trigger_key);
            c.Slider("preserve killfeed", s.keep_killfeed, 0, 300, "s");
            c.Slider("weapon volume", s.weapon_volume, 0, 100, "%");
            c.Slider("footstep volume", s.footstep_volume, 0, 100, "%");
            c.Slider("other volume", s.other_volume, 0, 100, "%");
            c.EndGroup();
        }

        void DrawConfig(Canvas& c, UiState& s)
        {
            c.BeginColumn(false);
            c.BeginGroup("menu");
            c.Label("accent");
            c.Slider("fade speed", s.fade_speed, 0, 10);
            c.Check("rainbow mode", s.rainbow);
            c.EndGroup();
            c.BeginColumn(true);
            c.BeginGroup("hack");
            c.Dropdown("restrictions", s.selected_restriction, { "none", "anti untrusted", "anti smac" });
            s.restrictions = s.selected_restriction != 0;
            c.Button("save", s.save_clicked);
            c.Button("load", s.load_clicked);
            c.Button("unload", s.unload_clicked);
            c.EndGroup();
        }
    }

        void Draw(UiState& state)
        {
        ImGuiIO& io = ImGui::GetIO();
        inspect_items.clear();
        ImGui::GetStyle().Colors[ImGuiCol_Text] = text_color;
        ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = background_color;
        static float alpha = 255.0f;
        const float fade = static_cast<float>(state.fade_speed > 0 ? state.fade_speed : 255);
        alpha = std::clamp(alpha + (state.menu_open ? fade : -fade), 0.0f, 255.0f);
        if (ImGui::IsKeyPressed(ImGuiKey_Insert, false))
            state.menu_open = !state.menu_open;
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(io.DisplaySize, ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.0f);
        ImGui::Begin("ui_canvas", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBackground);
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, PackColor(control_color, alpha / 255.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, PackColor(control_color, alpha / 255.0f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, PackColor(control_color, alpha / 255.0f));
        ImGui::PushStyleColor(ImGuiCol_CheckMark, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_SliderGrab, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
        ImGui::GetWindowDrawList()->Flags &= ~(ImDrawListFlags_AntiAliasedLines | ImDrawListFlags_AntiAliasedFill);
        ImDrawList* draw = ImGui::GetWindowDrawList();
        if (alpha <= 0)
        {
            ImGui::PopStyleVar(7);
            ImGui::PopStyleColor(9);
            ImGui::End();
            return;
        }
        
        Canvas c{ draw, state, alpha / 255.0f, 0, 0, 0, 0, 0.0f, state.window_pos, 1.0f, 1.0f, {} };
        float x = state.window_pos.x;
        float y = state.window_pos.y;
        c.state_pos = state.window_pos;
        if (!state.menu_open && alpha == 0.0f)
        {
            ImGui::PopStyleVar(7);
            ImGui::PopStyleColor(9);
            ImGui::End();
            return;
        }
        ImGui::SetCursorScreenPos(ImVec2(x, y));
        ImGui::InvisibleButton("move_window", ImVec2(522.0f, 8));
        if (ImGui::IsItemActivated())
        {
            state.dragging = true;
            state.drag_offset = ImVec2(io.MousePos.x - x, io.MousePos.y - y);
        }
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
            state.dragging = false;
        if (state.dragging)
        {
            state.window_pos.x = std::clamp(io.MousePos.x - state.drag_offset.x, 0.0f, std::max(0.0f, io.DisplaySize.x - 522.0f));
            state.window_pos.y = std::clamp(io.MousePos.y - state.drag_offset.y, 0.0f, std::max(0.0f, io.DisplaySize.y - 560.0f));
            x = state.window_pos.x;
            y = state.window_pos.y;
            c.state_pos = state.window_pos;
        }
        DrawWindowLayers(draw, ImVec2(x, y), ImVec2(x + 522.0f, y + 560.0f), theme, alpha / 255.0f);
        DrawBrand(c, state);
        const float cx = x + 20, cy = y + 44;
        DrawPanelLayers(draw, ImVec2(cx, cy), ImVec2(cx + InnerSize.x, cy + InnerSize.y), theme, alpha / 255.0f);
        DrawTabs(c, state);
        const ImVec2 content_min(cx + 3, cy + 8);
        const ImVec2 content_max(cx + InnerSize.x - 3, cy + InnerSize.y - 3);
        const ImVec2 mouse = io.MousePos;
        if (mouse.x >= content_min.x && mouse.x <= content_max.x && mouse.y >= content_min.y && mouse.y <= content_max.y)
            state.content_scroll = std::clamp(state.content_scroll - io.MouseWheel * 18.0f, 0.0f, 120.0f);
        draw->PushClipRect(ImVec2(cx + 3, cy + 8), ImVec2(cx + InnerSize.x - 3, cy + InnerSize.y - 3), true);
        switch (state.tab)
        {
        case Tab::Legit: DrawLegit(c, state); break;
        case Tab::Rage: DrawRage(c, state); break;
        case Tab::Players: DrawPlayers(c, state); break;
        case Tab::Visuals: DrawVisuals(c, state); break;
        case Tab::Misc: DrawMisc(c, state); break;
        case Tab::Config: DrawConfig(c, state); break;
        }
        draw->PopClipRect();
        draw->AddLine(ImVec2(io.MousePos.x + 1, io.MousePos.y), ImVec2(io.MousePos.x + 1, io.MousePos.y + 17), c.C(3, 6, 26, 215), 2);
        for (int i = 0; i < 11; ++i)
            draw->AddRectFilled(ImVec2(io.MousePos.x + 2 + i, io.MousePos.y + 1 + i), ImVec2(io.MousePos.x + 3 + i, io.MousePos.y + 2 + i), c.C(3, 6, 26, 215));
        draw->AddTriangleFilled(ImVec2(io.MousePos.x + 2, io.MousePos.y + 2), ImVec2(io.MousePos.x + 7, io.MousePos.y + 14), ImVec2(io.MousePos.x + 11, io.MousePos.y + 10), c.C(255, 255, 255));
        ImGui::PopStyleVar(7);
        ImGui::PopStyleColor(9);
        ImGui::End();
        DrawPreview(state, alpha / 255.0f);
    }
}
