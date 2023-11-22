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
            display_location(_data->locations[idx], _data->manifest.assets.backgrounds, idx);
        }
    }

    void location_browser::display_location(game_data_location& location, std::vector<game_asset>& assets, size_t location_index)
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
            display_backgrounds(location.backgrounds, assets, id);
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

    void location_browser::display_backgrounds(std::vector<u16>& backgrounds, std::vector<game_asset>& assets, std::string const& id)
    {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        if (ImGui::CollapsingHeader("Backgrounds"))
        {
            ImGui::Indent();
            // Add background button
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::Button((ICON_MD_ADD_PHOTO_ALTERNATE " Add new image##BG" + id).c_str(), { -FLT_MIN, 0 }))
            {
                if (auto file = open_file_dialog("iff;bmp"))
                {
                    auto path = std::filesystem::path{ file.value().data()};
                    
                    backgrounds.push_back(to<u16>(assets.size()));

                    auto background = MPG::load_image(path);
                    assets.push_back(
                        { 
                            path.stem().string(),
                            path.relative_path(),
                            load_texture(background),
                        });

                }
            }
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::Button((ICON_MD_ADD_PHOTO_ALTERNATE " Add existing image##BG" + id).c_str(), { -FLT_MIN, 0 }))
            {
                backgrounds.push_back(0);
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
                auto bg_name = assets[bg].name;

                ImGui::PushID((void*)&backgrounds[idx]);
                if (ImGui::BeginCombo("##combo", bg_name.c_str()))
                {
                    for (u16 n = 0; n < assets.size(); n++)
                    {
                        auto const is_selected = bg == n;
                        if (ImGui::Selectable(assets[n].name.c_str(), is_selected))
                        {
                            bg = n;
                        }

                        if (is_selected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }

                    ImGui::EndCombo();
                }

                ImGui::SameLine();
                ImGui::Button("##preview", ImVec2{ -FLT_MIN, 0 });
                if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();
                    auto texture = assets[bg].texture;
                    ImGui::Image((void*)(intptr_t)texture.texture_id, ImVec2((float)(texture.width), (float)(texture.height)));
                    ImGui::EndTooltip();
                }

                ImGui::PopID();
            }

            // Check for deleted backgrounds
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
                selected_value = (selected_value < values.size() - 1) ? selected_value : NO_INDEX;

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
