#include "imgui.h"
#include "imgui_stdlib.h"
#include "IconsMaterialDesign.h"
#include "imgui_utils.h"

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

    }
}