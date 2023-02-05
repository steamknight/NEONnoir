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
    void scene_editor::display(game_data_location* location, game_data_scene* scene)
    {
        if (location != nullptr && scene != nullptr)
        {
            _location = location;
            _scene = scene;
            auto locations_window = ImGui_window(std::format("{}: {}###Scene", location->name, scene->name));
            display_editor();
        }
    }

    void scene_editor::display_editor()
    {
        if (ImGui::BeginTable("Layout", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable))
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            display_scene_properties();

            ImGui::TableNextColumn();
            display_scene();

            ImGui::EndTable();
        }
    }

    void scene_editor::display_scene_properties()
    {
        if (ImGui::BeginChild("SceneProps", { 0, 0 }, true))
        {
            if (ImGui::BeginTable("PropsLayout", 2, ImGuiTableFlags_SizingStretchProp))
            {
                display_prop_string("Name", _scene->name);
                display_prop_multistring("Description", _scene->description_id);
                display_prop_background(_location->backgrounds);
                display_prop_string("On Enter", _scene->on_enter);
                display_prop_string("On Exit", _scene->on_exit);
                display_prop_int("Music ID", _scene->music_id);
                ImGui::EndTable();
            }

            auto exit_regions = std::vector<std::string>{ "None" };
            for (auto const& scene : _location->scenes)
            {
                exit_regions.push_back(scene.name);
            }

            display_prop_regions(exit_regions);
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
        ImGui::InputText(std::format("##{}", (u64)&value).c_str(), &value);
    }


    void scene_editor::display_prop_string_entry(std::string_view const& label, std::string& string_id)
    {
        if (string_id.empty())
        {
            string_id = _data->strings.create_string_entry("");
        }

        display_prop_string(label, _data->strings.get_string(string_id));
    }

    void scene_editor::display_prop_multistring(std::string_view const& label, std::vector<std::string>& string_ids)
    {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label.data());

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);

        if (ImGui::Button("Add description"))
        {
            string_ids.push_back({});
        }

        auto delete_index = std::optional<size_t>{ std::nullopt };
        auto count = 0;
        for (auto& string_id : string_ids)
        {
            ImGui::PushID(to<void*>(&string_id));
            if (DeleteButton("##delete"))
            {
                delete_index = count;
            }
            ImGui::SameLine();
            ImGui::SetNextItemWidth(-FLT_MIN);

            if (string_id.empty())
            {
                string_id = _data->strings.create_string_entry("");
            }
            ImGui::InputTextMultiline("##", &_data->strings.get_string(string_id), {0, 60});
            ImGui::PopID();
            
            count++;
        }

        if (delete_index)
        {
            auto const& text_id = string_ids[delete_index.value()];
            if (!text_id.empty())
            {
                _data->strings.remove_string(text_id);
            }

            string_ids.erase(string_ids.begin() + delete_index.value());
        }
    }

    void scene_editor::display_prop_int(std::string_view const& label, u16& value)
    {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label.data());

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        int v = value;
        ImGui::InputInt(std::format("##{}", (u64)&value).c_str(), &v);
        value = to<u16>(v);
    }

    void scene_editor::display_prop_combo(std::string_view const& label, std::vector<std::string> const& values, u16& selected_value)
    {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label.data());

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);

        selected_value++;
        ImGui::PushID((void*)&selected_value);
        if (ImGui::BeginCombo("##", values[selected_value].c_str()))
        {
            for (u16 n = 0; n < values.size(); n++)
            {
                auto const is_selected = selected_value == n;
                if (ImGui::Selectable(values[n].c_str(), is_selected))
                {
                    selected_value = n;
                }

                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopID();

        selected_value--;
    }

    void scene_editor::display_prop_enum(std::string_view const& label, std::vector<std::string> const& values, u16 & selected_value)
    {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label.data());

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);

        ImGui::SliderInt(std::format("##{}", (u64)&selected_value).c_str(), (int*)&selected_value, 0, (int)(values.size() - 1), values[selected_value].c_str());
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
            for (u16 index = 0; index < values.size(); index++)
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

    void scene_editor::display_prop_background(std::vector<std::string> const& backgrounds)
    {
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Background");

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        
        auto bg_path = fs::path{ backgrounds[_scene->image_id] };
        if (ImGui::BeginCombo("##BackgroundCombo", bg_path.stem().string().c_str()))
        {
            for (u16 index = 0; index < backgrounds.size(); index++)
            {
                auto const is_selected = _scene->image_id == index;
                bg_path = fs::path{ backgrounds[index] };
                if (ImGui::Selectable(bg_path.stem().string().c_str(), is_selected))
                {
                    _scene->image_id = index;
                }

                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }
    }

    void scene_editor::display_prop_regions(std::vector<std::string> const& exit_regions)
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
            for (auto index = 0; index < _scene->regions.size(); index++)
            {
                auto& region = _scene->regions[index];
                ImGui::TableNextRow();

                ImGui::BeginGroup();
                
                ImGui::TableNextColumn();
                ImGui::Text("Region %d", index);
                ImGui::TableNextColumn();

                display_prop_region_scalar("X", region.x);
                display_prop_region_scalar("Y", region.y);
                display_prop_region_scalar("Width", region.width);
                display_prop_region_scalar("Height", region.height);
                display_prop_string_entry("Hover Text", region.description_id);
                display_prop_enum("Mouse Pointer", pointer_types, region.pointer_id);
                display_prop_combo("Exit To Scene", exit_regions, region.goto_scene);
                display_prop_string("Script Name", region.script);

                ImGui::TableNextColumn(); ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                if (DeleteButton(std::format("{}", (u64)&region).c_str(), " Delete"))
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
                auto const& region = _scene->regions[region_to_delete];
                if (!region.description_id.empty())
                {
                    _data->strings.remove_string(region.description_id);
                }

                _scene->regions.erase(_scene->regions.begin() + region_to_delete);
            }
        }
    }

    void scene_editor::display_prop_region_scalar(std::string_view const& label, u16& value)
    {
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label.data());

        ImGui::TableNextColumn();
        size_t id = force_to<size_t>(&value);
        u16 const step_size = 1;
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputScalar(std::format("##{}", id).c_str(), ImGuiDataType_U16, &value, &step_size, nullptr, "%u");
    }

    void scene_editor::display_scene()
    {
        display_scene_toolbar();

        ImGuiIO& io = ImGui::GetIO(); (void)io;

        auto texture = _location->background_textures[_scene->image_id];
        ImGui::Image((void*)(intptr_t)texture.texture_id, ImVec2((float)(texture.width * _zoom), (float)(texture.height * _zoom)));

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
                    to<u16>(p_min.x), to<u16>(p_min.y),
                    to<u16>(size.x), to<u16>(size.y),
                    USHRT_MAX, to<u16>(0), 0xFFFF, "", ""
                };

                _scene->regions.push_back(region);
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

        for (auto index = 0; index < _scene->regions.size(); index++)
        {
            auto const& region = _scene->regions[index];

            auto p0 = ImVec2{ to<float>(region.x), to<float>(region.y) } * _zoom + image_min;
            auto p1 = ImVec2{ to<float>(region.x + region.width), to<float>(region.y + region.height) } * _zoom + image_min;

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