#include "imgui.h"
#include "imgui_stdlib.h"
#include "IconsMaterialDesign.h"
#include "imgui_utils.h"
#include "glfw_utils.h"

#include <format>
#include <filesystem>

#include "scene_editor.h"

namespace fs = std::filesystem;

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
        auto data = _game_data.lock();
        if (data && _location_index.has_value() && _scene_index.has_value())
        {
            auto const& location = data->locations[_location_index.value()];
            auto const& scene = location.scenes[_scene_index.value()];
            auto locations_window = ImGui_window(std::format("{}: {}###Scene", location.name, scene.name));
            display_editor(data);
        }
        else
        {
            auto locations_window = ImGui_window(ICON_MD_PLACE " Scene");
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
                display_prop_string("Name", scene.name);
                display_prop_background(scene, data->locations[_location_index.value()].backgrounds);
                ImGui::EndTable();
            }
            display_prop_regions(scene);
        }
        
        ImGui::EndChild();
    }

    void scene_editor::display_prop_string(std::string_view const& label, std::string& value)
    {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label.data());

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText(std::format("##{}", (uint64_t)&value).c_str(), &value);
    }

    void scene_editor::display_prop_enum(std::string_view const& label, std::vector<std::string> const& values, uint16_t & selected_value)
    {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label.data());

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);

        ImGui::SliderInt(std::format("##{}", (uint64_t)&selected_value).c_str(), (int*)&selected_value, 0, values.size() - 1, values[selected_value].c_str());
    }


    void scene_editor::display_prop_list(std::string_view const& label, std::vector<std::string>const& values, std::string& selected)
    {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label.data());

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
 
        if (ImGui::BeginCombo("##ScriptCombo", "[NONE]"))
        {
            for (uint16_t index = 0; index < values.size(); index++)
            {
                auto const is_selected = values[index] == selected;

                if (ImGui::Selectable(values[index].c_str(), is_selected))
                {
                    selected = values[index];
                }

                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }
    }

    void scene_editor::display_prop_background(game_data_scene& scene, std::vector<std::string> const& backgrounds)
    {
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Background");

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        
        auto bg_path = fs::path{ backgrounds[scene.image_id] };
        if (ImGui::BeginCombo("##BackgroundCombo", bg_path.stem().string().c_str()))
        {
            for (uint16_t index = 0; index < backgrounds.size(); index++)
            {
                auto const is_selected = scene.image_id == index;
                bg_path = fs::path{ backgrounds[index] };
                if (ImGui::Selectable(bg_path.stem().string().c_str(), is_selected))
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

    void scene_editor::display_prop_regions(game_data_scene& scene)
    {
        ImGui::NewLine();

        auto const origin = ImGui::GetCursorScreenPos();
        auto content_center = ImGui::GetColumnWidth() / 2.f;
        SetCursorCenteredText(origin + ImVec2{content_center, 0}, "Regions");
        ImGui::TextUnformatted("Regions");

        if (ImGui::BeginTable("RegionsLayout", 2, ImGuiTableFlags_SizingStretchProp))
        {
            _selected_region_index = -1;
            auto region_to_delete = -1;
            for (auto index = 0; index < scene.regions.size(); index++)
            {
                auto& region = scene.regions[index];
                ImGui::TableNextRow();

                ImGui::BeginGroup();
                
                ImGui::TableNextColumn();
                ImGui::Text("Region %d", index);
                ImGui::TableNextColumn();

                display_prop_region_scalar("X", region.x);
                display_prop_region_scalar("Y", region.y);
                display_prop_region_scalar("Width", region.width);
                display_prop_region_scalar("Height", region.height);
                display_prop_string("Hover Text", region.description);
                display_prop_enum("Mouse Pointer", pointer_types, region.pointer_id);
                display_prop_string("Script Name", region.script);

                ImGui::TableNextColumn(); ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (DeleteButton(std::format("{}", (uint64_t)&region).c_str(), " Delete"))
                {
                    region_to_delete = index;
                }

                ImGui::NewLine();
                
                ImGui::EndGroup();

                if (ImGui::IsItemHovered())
                {
                    _selected_region_index = index;
                }
            }
            ImGui::EndTable();

            if (region_to_delete >= 0)
            {
                scene.regions.erase(scene.regions.begin() + region_to_delete);
            }
        }
    }

    void scene_editor::display_prop_region_scalar(std::string_view const& label, uint16_t& value)
    {
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label.data());

        ImGui::TableNextColumn();
        size_t id = reinterpret_cast<size_t>(&value);
        uint16_t const step_size = 1;
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputScalar(std::format("##{}", id).c_str(), ImGuiDataType_U16, &value, &step_size, nullptr, "%u");
    }

    void scene_editor::display_scene(game_data_scene& scene, std::vector<GLtexture> const& background_textures)
    {
        display_scene_toolbar();

        ImGuiIO& io = ImGui::GetIO(); (void)io;

        auto texture = background_textures[scene.image_id];
        ImGui::Image((void*)(intptr_t)texture.texture_id, ImVec2(texture.width * _zoom, texture.height * _zoom));

        auto image_min = ImGui::GetItemRectMin();
        if (ImGui::IsItemHovered())
        {
            _last_mouse = (io.MousePos - image_min) / _zoom;
        }

        auto draw_list = ImGui::GetWindowDrawList();

        if (_add_region_mode)
        {
            if (io.MouseDown[0] && _last_mouse.x > 0 && _last_mouse.y > 0)
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
                    USHRT_MAX, static_cast<uint16_t>(0), "", ""
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

        for (auto index = 0; index < scene.regions.size(); index++)
        {
            auto const& region = scene.regions[index];

            auto p0 = ImVec2{ static_cast<float>(region.x), static_cast<float>(region.y) } * _zoom + image_min;
            auto p1 = ImVec2{ static_cast<float>(region.x + region.width), static_cast<float>(region.y + region.height) } * _zoom + image_min;

            if (_selected_region_index == index)
            {
                draw_list->AddRectFilled(p0, p1, IM_COL32(255, 165, 0, 64));
                draw_list->AddRect(p0, p1, IM_COL32(255, 165, 0, 255), 0.f, 0, 2.f);
            }
            else
            {
                draw_list->AddRectFilled(p0, p1, IM_COL32(13, 129, 255, 64));
                draw_list->AddRect(p0, p1, IM_COL32(13, 129, 255, 255), 0.f, 0, 2.f);
            }
        }
    }

    void scene_editor::display_scene_toolbar() noexcept
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
        ImGui::Text(ICON_MD_NEAR_ME " %.0f, %.0f\t" ICON_MD_ZOOM_IN " %d00%%", _last_mouse.x, _last_mouse.y, _zoom);
    }
}