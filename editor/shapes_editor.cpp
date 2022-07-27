#include "imgui.h"
#include "imgui_stdlib.h"
#include "IconsMaterialDesign.h"
#include "imgui_utils.h"

#include <filesystem>
#include <fstream>
#include <format>
#include <sstream>


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
            display_editor(data->locations[location_index.value()], data->shape_start_id);
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

    std::string shapes_editor::generate_script(game_data_location& location, int32_t shape_start_id) const
    {
        auto script = std::stringstream{};

        script << "; Shape generator\n\nBitMap 0, 320, 200, 5\n";

        auto shape_id = shape_start_id;
        for (auto const& container : location.shapes)
        {
            auto start_id = shape_id;
            auto filename = std::filesystem::path{ container.image_file }.stem().string();
            script << "\nLoadBitmap 0, \"" << filename << ".iff\"\n";

            for (auto const& shape : container.shapes)
            {
                script << "GetaShape " << shape_id << ", " << shape.x << ", " << shape.y << ", " << shape.width << ", " << shape.height << "\n";
                shape_id++;
            }

        }

        auto filename = location.name;
        std::replace(filename.begin(), filename.end(), ' ', '_');

        script << "\nSaveShapes " << shape_start_id << ", " << shape_id - 1 << " \""  << filename << ".shapes\"\n\n";

        return script.str();
    }

    void shapes_editor::display_editor(game_data_location& location, int32_t shape_start_id)
    {
        ImGui::Text("Shapes for Location: %s", location.name.c_str());

        if (auto table = imgui::table("main_shapes_editor", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable))
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            if (ImGui::Button(ICON_MD_ADD_PHOTO_ALTERNATE " Add source image", { -FLT_MIN, 0.f }))
            {
                if (auto file = open_file_dialog("bmp; Bitmap file"))
                {
                    auto container = shape_container{ file.value().data() };
                    container.palette = read_bmp_palette(file.value());
                    location.shapes.push_back(container);
                    location.shapes_textures.push_back(load_texture(file.value()));
                }
            }

            if (ImGui::Button(ICON_MD_SAVE_AS " Save Script", { -FLT_MIN, 0.f }))
            {
                auto filename = save_file_dialog("*.bb2");
                if (filename)
                {
                    auto script_file = std::ofstream{ filename.value().data(), std::ios::trunc};
                    auto script = generate_script(location, shape_start_id);
                    script_file << script;
                    script_file.close();
                }
            }

            auto count = 0u;
            auto shape_id = shape_start_id;
            for (auto& container : location.shapes)
            {

                if (DeleteButton(std::format("##ShapeContainderDelete{}", (size_t)&container)))
                {

                }

                ImGui::SameLine();

                ImGui::Checkbox(make_id("##{}", container.has_palette), &container.has_palette);
                ToolTip("Has own palette?");

                ImGui::SameLine();
                auto image_name = std::filesystem::path{ container.image_file }.stem().string();
                if (ImGui::Button(image_name.c_str(), {-FLT_MIN, 0.f}))
                {
                    _selected_image = count;
                }

                auto content_width = ImGui::GetColumnWidth();
                auto spacing = ImGui::GetStyle().ItemSpacing.x;
                auto item_width = (content_width - (1 * spacing)) / 2.f;

                if (_selected_image == count)
                {
                    for (auto i = 0; i < _selected_image; i++)
                    {
                        shape_id += to<int32_t>(location.shapes[i].shapes.size());
                    }

                    auto region_count = 0;
                    for (auto& shape : container.shapes)
                    {
                        ImGui::BeginGroup();

                        ImGui::Text("Shape %d", shape_id + region_count);
                        auto avail = ImGui::GetContentRegionAvail();
                        ImGui::SameLine(avail.x - ImGui::CalcTextSize(ICON_MD_DELETE).x - (1 * spacing));
                        if (DeleteButton(make_id("##{}", shape)))
                        {

                        }

                        uint16_t const step_size = 1;
                        ImGui::SetNextItemWidth(item_width);
                        ImGui::InputScalar(std::format("##{}", (size_t)&shape.x).c_str(), ImGuiDataType_U16, &shape.x, &step_size, nullptr, "%u");
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(item_width);
                        ImGui::InputScalar(std::format("##{}", (size_t)&shape.y).c_str(), ImGuiDataType_U16, &shape.y, &step_size, nullptr, "%u");
                        //                    ImGui::SameLine();
                        ImGui::SetNextItemWidth(item_width);
                        ImGui::InputScalar(std::format("##{}", (size_t)&shape.width).c_str(), ImGuiDataType_U16, &shape.width, &step_size, nullptr, "%u");
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(item_width);
                        ImGui::InputScalar(std::format("##{}", (size_t)&shape.height).c_str(), ImGuiDataType_U16, &shape.height, &step_size, nullptr, "%u");

                        ImGui::EndGroup();

                        if (ImGui::IsItemHovered())
                        {
                            _shape_image.selected_region(region_count);
                        }

                        region_count++;
                    }
                }
                count++;
            }

            ImGui::TableNextColumn();
            if (_selected_image.has_value())
            {
                auto& texture = location.shapes_textures[_selected_image.value()];
                _shape_image.display(texture, location.shapes[_selected_image.value()].shapes);
            }


        }
    }
}
