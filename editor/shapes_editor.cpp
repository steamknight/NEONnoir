#include "imgui.h"
#include "imgui_stdlib.h"
#include "IconsMaterialDesign.h"
#include "imgui_utils.h"

#include <format>

#include "utils.h"
#include "shapes_editor.h"

namespace NEONnoir
{
    void NEONnoir::shapes_editor::display(std::weak_ptr<game_data> game_data, std::optional<size_t> const& location_index)
    {
        auto shapes_editor_window = ImGui_window(ICON_MD_PHOTO " Shapes Editor");

        auto data = game_data.lock();
        if (data && location_index)
        {
            display_editor(data->locations[location_index.value()]);
        }
        else
        {
            display_placeholder(!game_data.expired(), location_index);
        }
    }

    void shapes_editor::display_placeholder(bool have_data, std::optional<size_t> const& location_index) const noexcept
    {
        auto const origin = ImGui::GetCursorScreenPos();
        auto const size = ImGui::GetContentRegionAvail();

        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        draw_list->AddRectFilled(origin, origin + size, IM_COL32(4, 16, 32, 255));

        auto text = (!have_data) ? "No Game File" : "Select a location.";

        SetCursorCenteredText(origin + (size / 2), text);
        ImGui::TextColored({ 16.f / 255.f, 64.f / 255.f, 128.f / 255.f, 1.f }, text);
    }

    void shapes_editor::display_editor(game_data_location& location)
    {
        ImGui::Text("Shapes for Location: %s", location.name.c_str());

        if (auto table = imgui::table("main_shapes_editor", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable))
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            if (ImGui::Button(ICON_MD_ADD_PHOTO_ALTERNATE " Add source image"))
            {
                if (auto file = open_file_dialog("bmp; Bitmap file"))
                {
                    location.shapes.push_back({ file.value().data(), {} });
                }
            }

            for (auto const& shape : location.shapes)
            {
                if (ImGui::Button(shape.image_file.c_str()))
                {

                }

                ImGui::SameLine();

                if (DeleteButton(std::format("##ShapeContainderDelete{}", (size_t)&shape)))
                {

                }
            }
        }
    }
}
