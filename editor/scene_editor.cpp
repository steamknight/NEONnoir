#include "imgui.h"
#include "imgui_stdlib.h"
#include "IconsMaterialDesign.h"
#include "imgui_utils.h"
#include "glfw_utils.h"

#include <format>

#include "scene_editor.h"

namespace NEONnoir
{
    void scene_editor::use(std::weak_ptr<game_data> game_data, std::optional<size_t> const& location_index, std::optional<size_t> const& scene_index)
    {
        _game_data = game_data;
        _location_index = location_index;
        _scene_index = scene_index;
    }

    void scene_editor::display()
    {
        auto locations_window = ImGui_window("Scene");

        auto data = _game_data.lock();
        if (data && _location_index.has_value() && _scene_index.has_value())
        {
            display_editor(data);
        }
        else
        {
            display_placeholder();
        }
    }

    void scene_editor::display_placeholder() const
    {
        auto const origin = ImGui::GetCursorScreenPos();
        auto const size = ImGui::GetContentRegionAvail();

        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        draw_list->AddRectFilled(origin, origin + size, IM_COL32(4, 16, 32, 255));

        auto text = (_game_data.expired()) ? "No Game File" : "Select a scene to edit.";

        SetCursorCenteredText(origin + (size / 2), text);
        ImGui::TextColored({ 16.f / 255.f, 64.f / 255.f, 128.f / 255.f, 1.f }, text);
    }

    void scene_editor::display_editor(std::shared_ptr<game_data>& data)
    {
        if (ImGui::BeginTable("Layout", 2, ImGuiTableFlags_SizingFixedFit))
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            display_scene_properties(data);

            auto& location = data->locations[_location_index.value()];
            auto& scene = location.scenes[_scene_index.value()];

            ImGui::TableNextColumn();
            display_scene(scene, location.background_textures);

            ImGui::EndTable();
        }
    }

    void scene_editor::display_scene_properties(std::shared_ptr<game_data>& data)
    {
        if (ImGui::BeginChild("SceneProps", { 256, 0 }, true))
        {
            auto& scene = data->locations[_location_index.value()].scenes[_scene_index.value()];

            if (ImGui::BeginTable("PropsLayout", 2, ImGuiTableFlags_SizingStretchProp))
            {
                display_prop_name(scene.name);
                display_prop_background(scene, data->locations[_location_index.value()].backgrounds);

                ImGui::EndTable();
            }
        }
        
        ImGui::EndChild();
    }

    void scene_editor::display_prop_name(std::string& name)
    {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Name");

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText(std::format("##{}", (uint64_t)&name).c_str(), &name);
    }

    void scene_editor::display_prop_background(game_data_scene& scene, std::vector<std::string> const& backgrounds)
    {
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Background");

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo("##BackgroundCombo", backgrounds[scene.image_id].c_str()))
        {
            for (uint16_t index = 0; index < backgrounds.size(); index++)
            {
                auto const is_selected = scene.image_id == index;
                if (ImGui::Selectable(backgrounds[index].c_str(), is_selected))
                {
                    scene.image_id = index;
                }

                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }
    }

    void scene_editor::display_scene(game_data_scene& scene, std::vector<GLtexture> const& background_textures)
    {
        auto button_size = ImVec2{ 128, 0 };
        if (ImGui::Button(ICON_MD_CROP " Add Region", button_size))
        {
            _add_region_mode = true;
        }

        ImGui::SameLine();
        if (ImGui::Button(ICON_MD_ZOOM_IN " Zoom In", button_size))
        {
            _zoom++; 
        }

        ImGui::SameLine();
        if (ImGui::Button(ICON_MD_ZOOM_OUT " Zoom Out", button_size) && _zoom > 1)
        {
            _zoom--;
        };

        ImGuiIO& io = ImGui::GetIO(); (void)io;

        static ImVec2 last_mouse{ 0, 0 };
        ImGui::Text(ICON_MD_NEAR_ME " %.0f, %.0f ", last_mouse.x, last_mouse.y);

        auto texture = background_textures[scene.image_id];
        ImGui::Image((void*)(intptr_t)texture.texture_id, ImVec2(texture.width * _zoom, texture.height * _zoom));

        auto image_min = ImGui::GetItemRectMin();
        if (ImGui::IsItemHovered())
        {
            last_mouse = (io.MousePos - image_min) / _zoom;
        }

        auto draw_list = ImGui::GetWindowDrawList();

        if (_add_region_mode)
        {
            if (io.MouseDown[0] && last_mouse.x > 0 && last_mouse.y > 0)
            {
                if (_add_region_p0.x < 0 && _add_region_p0.y < 0)
                {
                    _add_region_p0 = io.MousePos;
                    _add_region_dragging = true;
                }

            }
            if (io.MouseReleased[0] && _add_region_dragging)
            {
                auto p0 = (_add_region_p0 - image_min) / _zoom;
                auto p1 = (io.MousePos - image_min) / _zoom;

                auto p_min = ImVec2
                {
                    std::min(p0.x, p1.x),
                    std::min(p0.y, p1.y)
                };

                auto size = ImVec2
                {
                    std::fabsf(p1.x - p0.x),
                    std::fabsf(p1.y - p0.y)
                };

                // Sort the points so we don't get wonky regions
                auto region = game_data_region{
                    static_cast<uint16_t>(p_min.x), static_cast<uint16_t>(p_min.y),
                    static_cast<uint16_t>(size.x), static_cast<uint16_t>(size.y),
                    USHRT_MAX, "", ""
                };

                scene.regions.push_back(region);
                _add_region_mode = false;
                _add_region_dragging = false;
                _add_region_p0 = { -1, -1 };
            }

            if (_add_region_dragging)
            {
                draw_list->AddRectFilled(_add_region_p0, io.MousePos, IM_COL32(255, 165, 0, 64));
                draw_list->AddRect(_add_region_p0, io.MousePos, IM_COL32(255, 165, 0, 255), 0.f, 0, 2.f);
            }
        }

        for (auto const& region : scene.regions)
        {
            auto p0 = ImVec2{ static_cast<float>(region.x), static_cast<float>(region.y) } * _zoom + image_min;
            auto p1 = ImVec2{ static_cast<float>(region.x + region.width), static_cast<float>(region.y + region.height) } * _zoom + image_min;

            draw_list->AddRectFilled(p0, p1, IM_COL32(13, 129, 255, 64));
            draw_list->AddRect(p0, p1, IM_COL32(13, 129, 255, 255), 0.f, 0, 2.f);
        }
    }
}