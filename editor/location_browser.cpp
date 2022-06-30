#include "imgui.h"
#include "imgui_stdlib.h"
#include "IconsMaterialDesign.h"
#include "imgui_utils.h"

#include <filesystem>
#include <format>

#include "utils.h"
#include "location_browser.h"

namespace fs = std::filesystem;

namespace NEONnoir
{
    void location_browser::use(std::weak_ptr<game_data> game_data)
    {
        _game_data = game_data;
    }

    void location_browser::display()
    {
        auto locations_window = ImGui_window(ICON_MD_MAP " Locations");

        if (auto data = _game_data.lock())
        {
            display_editor(data);
        }
        else
        {
            display_placeholder();
        }
    }

    bool location_browser::is_scene_selected() const noexcept
    {
        return _selected_location.has_value() && _selected_scene.has_value();
    }

    void location_browser::display_placeholder() noexcept
    {
        auto const origin = ImGui::GetCursorScreenPos();
        auto const size = ImGui::GetContentRegionAvail();

        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        draw_list->AddRectFilled(origin, origin + size, IM_COL32(4, 16, 32, 255));
        SetCursorCenteredText(origin + (size / 2), "No Game File");
        ImGui::TextColored({ 16.f / 255.f, 64.f / 255.f, 128.f / 255.f, 1.f }, "No Game File");
    }

    void location_browser::display_editor(std::shared_ptr<game_data> data)
    {
        auto const origin = ImGui::GetCursorScreenPos();
        auto const size = ImGui::GetContentRegionAvail();

        // Add Location button
        if (ImGui::Button(ICON_MD_MAP " Add new location", { size.x, 0.f }))
        {
            auto name = std::string{ "Location " } + std::to_string(data->locations.size() + 1);
            data->locations.push_back({ name });
        }

        // Display each location's data
        for (auto idx = 0; idx < data->locations.size(); idx++)
        {
            display_location(data->locations[idx], idx);
        }
    }

    void location_browser::display_location(game_data_location& location, size_t location_index)
    {
        // For the header name, we don't want the name to have any part in the
        // generation of the id since it can change and mess everything up
        auto id = std::to_string(reinterpret_cast<uint64_t>(&location));
        if (!ImGui::CollapsingHeader(std::format("{}###Location{}", location.name, id).c_str()))
            return;

        // Use a table so we can have labels on the left rather than on the right
        if (ImGui::BeginTable(("LocationTable" + id).c_str(), 2, ImGuiTableFlags_SizingStretchProp))
        {
            display_location_name(location.name, id);
            display_backgrounds(location.backgrounds, location.background_textures, id);
            display_scenes(location.scenes, id, location_index);

            // End LocationTable
            ImGui::EndTable();
        }
    }

    void location_browser::display_location_name(std::string& name, std::string const& id)
    {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Name");

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText(("##" + id).c_str(), &name);
    }

    void location_browser::display_backgrounds(std::vector<std::string>& backgrounds, std::vector<GLtexture>& background_textures, std::string const& id)
    {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Backgrounds");

        // Add background button
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::Button((ICON_MD_ADD_PHOTO_ALTERNATE " Add##BG" + id).c_str(), { -FLT_MIN, 0 }))
        {
            if (auto file = open_file_dialog("bmp; Bitmap file"))
            {
                backgrounds.push_back(file.value().data());
                background_textures.push_back(load_texture(file.value()));
            }
        }

        // List backgrounds
        // Not using range-for because we need an index
        int32_t remove_index = -1;
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
    }

    void location_browser::display_scenes(std::vector<game_data_scene>& scenes, std::string const& id, size_t location_index)
    {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Scenes");

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::Button((ICON_MD_ADD_LOCATION_ALT " Add##Scene" + id).c_str(), { -FLT_MIN, 0 }))
        {
            auto name = std::format("Scene {}", scenes.size() + 1);
            scenes.emplace_back(name);
        }

        // List scenes
        // Not using range-for because we need an index
        int32_t remove_index = -1;
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
            scenes.erase(scenes.begin() + remove_index);
        }

    }
}