#include "imgui.h"
#include "imgui_stdlib.h"
#include "IconsMaterialDesign.h"
#include "imgui_utils.h"

#include <format>

#include "shapes_editor.h"

namespace NEONnoir
{
    void NEONnoir::shapes_editor::display(std::weak_ptr<game_data> game_data)
    {
        auto shapes_editor_window = ImGui_window(ICON_MD_PHOTO " Shapes Editor");

        if (auto data = game_data.lock())
        {
            display_editor(data);
        }
        else
        {
            display_placeholder();
        }
    }

    void shapes_editor::display_placeholder() const noexcept
    {
        auto const origin = ImGui::GetCursorScreenPos();
        auto const size = ImGui::GetContentRegionAvail();

        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        draw_list->AddRectFilled(origin, origin + size, IM_COL32(4, 16, 32, 255));
        SetCursorCenteredText(origin + (size / 2), "No Game File");
        ImGui::TextColored({ 16.f / 255.f, 64.f / 255.f, 128.f / 255.f, 1.f }, "No Game File");
    }

    void shapes_editor::display_editor(std::shared_ptr<game_data> data)
    {
    }
}
