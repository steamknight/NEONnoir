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
            display_placeholder(!game_data.expired());
        }
    }

    void shapes_editor::display_placeholder(bool have_data) const noexcept
    {
        auto const origin = ImGui::GetCursorScreenPos();
        auto const size = ImGui::GetContentRegionAvail();

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        draw_list->AddRectFilled(origin, origin + size, IM_COL32(4, 16, 32, 255));

        auto text = (!have_data) ? "No Game File" : "Select a location.";

        SetCursorCenteredText(origin + (size / 2), text);
        ImGui::TextColored({ 16.f / 255.f, 64.f / 255.f, 128.f / 255.f, 1.f }, text);
    }

    void shapes_editor::save_shapes(std::filesystem::path const& shapes_file_path, game_data_location& location) const
    {
        auto all_shapes = std::vector<MPG::simple_image>{};
        for (auto const& shape_container : location.shapes)
        {
            for (auto const& shape : shape_container.shapes)
            {
                all_shapes.push_back(MPG::crop(shape_container.image, shape.x, shape.y, shape.width, shape.height));
            }
        }

        MPG::save_blitz_shapes(shapes_file_path, all_shapes);
    }

    void shapes_editor::display_editor(game_data_location& location, i32 shape_start_id)
    {
        ImGui::Text("Shapes for Location: %s", location.name.c_str());

        if (auto table = imgui::table("main_shapes_editor", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable))
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            if (ImGui::Button(ICON_MD_ADD_PHOTO_ALTERNATE " Add source image", { -FLT_MIN, 0.f }))
            {
                if (auto file = open_file_dialog("iff"))
                {
                    auto container = shape_container{ file.value().data() };
                    container.image = MPG::load_image(file.value());
                    location.shapes.push_back(container);
                    location.shapes_textures.push_back(load_texture(container.image));
                }
            }

            if (ImGui::Button(ICON_MD_SAVE_AS " Export Shapes", { -FLT_MIN, 0.f }))
            {
                auto filename = save_file_dialog("shapes");
                if (filename)
                {
                    save_shapes(filename.value(), location);
                }
            }

            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::InputText(make_id("##shapefile{}", location.shapes_file), &location.shapes_file);
            ToolTip("Name of the shapes file.");

            auto count = 0u;
            auto shape_id = shape_start_id;
            for (auto& container : location.shapes)
            {
                if (DeleteButton(std::format("##ShapeContainderDelete{}", (size_t)&container)))
                {
                    _shape_container_to_delete = count;
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
                        shape_id += to<i32>(location.shapes[i].shapes.size());
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
                            _shape_to_delete = region_count;
                        }

                        u16 const step_size = 1;
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

            if (_shape_container_to_delete)
            {
                location.shapes.erase(location.shapes.begin() + _shape_container_to_delete.value());
                _shape_container_to_delete = std::nullopt;
                _selected_image = std::nullopt;
            }

            if (_shape_to_delete && _selected_image && _selected_image.value() < location.shapes.size())
            {
                auto& container = location.shapes[_selected_image.value()].shapes;
                container.erase(container.begin() + _shape_to_delete.value());

                _shape_to_delete = std::nullopt;
            }

            ImGui::TableNextColumn();
            if (_selected_image.has_value() && _selected_image.value() < location.shapes_textures.size())
            {
                auto& texture = location.shapes_textures[_selected_image.value()];
                _shape_image.display(texture, location.shapes[_selected_image.value()].shapes);
            }


        }
    }
}
