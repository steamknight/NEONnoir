#include "imgui.h"
#include "imgui_stdlib.h"
#include "IconsMaterialDesign.h"
#include "imgui_utils.h"

#include <filesystem>
#include <format>

#include "utils.h"
#include "location_browser.h"
#include "shapes_editor.h"

namespace fs = std::filesystem;

namespace NEONnoir
{
    bool location_browser::is_scene_selected() const noexcept
    {
        return _selected_location.has_value() && _selected_scene.has_value();
    }

    void location_browser::display_editor()
    {
        auto const size = ImGui::GetContentRegionAvail();

        // Add Location button
        if (ImGui::Button(ICON_MD_MAP " Add new location", { size.x, 0.f }))
        {
            auto name = std::string{ "Location " } + std::to_string(_data->locations.size() + 1);
            _data->locations.push_back({ name });
        }

        // Display each location's data
        for (auto idx = 0; idx < _data->locations.size(); idx++)
        {
            display_location(_data->locations[idx], idx);
        }
    }

    void location_browser::display_location(game_data_location& location, size_t location_index)
    {
        // For the header name, we don't want the name to have any part in the
        // generation of the id since it can change and mess everything up
        auto id = std::to_string(force_to<u64>(&location));
        if (!ImGui::CollapsingHeader(std::format("{}###Location{}", location.name, id).c_str()))
            return;

        // Use a table so we can have labels on the left rather than on the right
        ImGui::PushID(to<void*>(&location));
        ImGui::Indent();
        if (auto table = imgui::table("##location_table", 1, ImGuiTableFlags_SizingStretchProp))
        {
            display_location_name(location.name, id);
            display_backgrounds(location.backgrounds, location.background_textures, id);
            display_scenes(location.scenes, id, location_index);
            display_speakers(location);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::Button(std::format("Shapes Editor##{}", (size_t)&location).c_str(), {-FLT_MIN, 0}))
            {
                _selected_location = std::make_optional<size_t>(location_index);
            }
        }

        ImGui::Unindent();
        ImGui::PopID();
    }

    void location_browser::display_location_name(std::string& name, std::string const& id)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Name");
        ImGui::SameLine();

        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText(("##" + id).c_str(), &name);
    }

    void location_browser::display_backgrounds(std::vector<std::string>& backgrounds, std::vector<GLtexture>& background_textures, std::string const& id)
    {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        if (ImGui::CollapsingHeader("Backgrounds"))
        {
            ImGui::Indent();
            // Add background button
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::Button((ICON_MD_ADD_PHOTO_ALTERNATE " Add##BG" + id).c_str(), { -FLT_MIN, 0 }))
            {
                if (auto file = open_file_dialog("iff;bmp"))
                {
                    backgrounds.push_back(file.value().data());
                    auto background = MPG::load_image(file.value());
                    background_textures.push_back(load_texture(background));
                }
            }

            // List backgrounds
            // Not using range-for because we need an index
            i32 remove_index = -1;
            for (auto idx = 0; idx < backgrounds.size(); idx++)
            {
                auto& bg = backgrounds[idx];

                if (DeleteButton(std::format("##BGDeletebutton{}", idx).c_str()))
                {
                    // Mark for deletion
                    remove_index = idx;
                }
                ImGui::SameLine();
                auto bg_name = fs::path{ bg }.stem().string();
                if (ImGui::Button(bg_name.c_str(), ImVec2{ -FLT_MIN, 0 }))
                {
                    if (auto file = open_file_dialog("bmp"))
                    {
                        auto filename = fs::path{ file.value() }.filename();
                        bg = filename.string();
                    }
                };
            }

            // Check for deleted backgrouns
            if (remove_index >= 0)
            {
                backgrounds.erase(backgrounds.begin() + remove_index);
            }

            ImGui::Unindent();
        }
    }

    void location_browser::display_scenes(std::vector<game_data_scene>& scenes, std::string const& id, size_t location_index)
    {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();

        if (!ImGui::CollapsingHeader("Scenes"))
            return;

        ImGui::Indent();
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::Button((ICON_MD_ADD_LOCATION_ALT " Add##Scene" + id).c_str(), { -FLT_MIN, 0 }))
        {
            auto name = std::format("Scene {}", scenes.size() + 1);
            scenes.emplace_back(name);
        }

        // List scenes
        // Not using range-for because we need an index
        i32 remove_index = -1;
        for (auto idx = 0; idx < scenes.size(); idx++)
        {
            auto& scene = scenes[idx];

            if (DeleteButton(std::format("##SceneDeletebutton{}", idx).c_str()))
            {
                // Mark for deletion
                remove_index = idx;
            }

            ImGui::SameLine();
            if (ImGui::Button(std::format(ICON_MD_DESCRIPTION "##SceneEditButton{}", idx).c_str()))
            {
                _selected_location = std::make_optional<size_t>(location_index);
                _selected_scene = std::make_optional<size_t>(idx);
            }

            ImGui::SameLine();
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputTextWithHint(std::format("##Scene{}", idx).c_str(), scene.name.c_str(), &scene.name);
        }

        // Check for deleted backgrouns
        if (remove_index >= 0)
        {
            auto const& scene = scenes[remove_index];
            if (!scene.description_id.empty())
            {
                for (auto const& desc_id : scene.description_id)
                {
                    if (!desc_id.empty())
                    {
                        _data->strings.remove_string(desc_id);
                    }
                }
            }

            for (auto const& region : scene.regions)
            {
                if (!region.description_id.empty())
                {
                    _data->strings.remove_string(region.description_id);
                }
            }
            scenes.erase(scenes.begin() + remove_index);
        }

        ImGui::Unindent();
    }

    void location_browser::display_speakers(game_data_location& location)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        if (ImGui::CollapsingHeader("Speakers"))
        {
            ImGui::Indent();
            auto& values = get_speaker_list(_data->speakers);

            auto width = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x) / 2.f;

            for (size_t idx = 0; idx < 8; idx++)
            {
                auto& selected_value = location.speakers[idx];

                // Sanity check in case something's been deleted
                selected_value = (selected_value < values.size() - 1) ? selected_value : 0xFFFF;

                selected_value++;
                ImGui::PushID((void*)&selected_value);

                ImGui::SetNextItemWidth(width);
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

                if (idx % 2 == 0)
                    ImGui::SameLine();
            }

            ImGui::Unindent();
        }
    }
}
