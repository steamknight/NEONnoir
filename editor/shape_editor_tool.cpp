#include "imgui.h"
#include "imgui_stdlib.h"
#include "IconsMaterialDesign.h"
#include "imgui_utils.h"

#include <filesystem>
#include <fstream>
#include <format>
#include <sstream>

#include "utils.h"
#include "shape_editor_tool.h"

namespace NEONnoir
{
    bool shape_editor_tool::display()
    {
        auto shapes_editor_window = ImGui_window(ICON_MD_CROP " Shapes Editor Tool", true, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);

        if (shapes_editor_window.is_closing())
        {
            return false;
        }

        display_toolbar();

        if (!_filename.empty())
        {
            ImGui::SameLine();

            ImGui::Text("Shapes: %s", _filename.c_str());
        }

        if (auto table = imgui::table("main_shapes_editor_tool", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable))
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::SliderInt("##slider", &_export_bit_depth, 1, 8, "Output bit-depth: %d");

            if (ImGui::Button(ICON_MD_ADD_PHOTO_ALTERNATE " Add source image", { -FLT_MIN, 0.f }))
            {
                if (auto file = open_file_dialog("bmp;iff"))
                {
                    auto container = shape_container{ file.value().data() };
                    container.image = MPG::load_image(file.value());
                    _shape_containers.push_back(container);
                    _shape_textures.push_back(load_texture(container.image));
                }
            }

            auto count = 0u;
            auto shape_id = 0;
            for (auto& container : _shape_containers)
            {
                if (DeleteButton(std::format("##ShapeContainderDelete{}", (size_t)&container)))
                {
                    _shape_container_to_delete = count;
                }

                ImGui::SameLine();

                ImGui::SameLine();
                auto image_name = std::filesystem::path{ container.image_file }.stem().string();
                if (ImGui::Button(image_name.c_str(), { -FLT_MIN, 0.f }))
                {
                    _selected_image = count;
                }

                auto content_width = ImGui::GetColumnWidth() - 16; // leave room for a scroll bar
                auto spacing = ImGui::GetStyle().ItemSpacing.x;
                auto item_width = (content_width - (1 * spacing)) / 2.f;

                if (_selected_image == count)
                {
                    for (auto i = 0; i < _selected_image; i++)
                    {
                        shape_id += to<i32>(_shape_containers[i].shapes.size());
                    }

                    auto region_count = 0;
                    ImGui::BeginChild("regions");
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
                    ImGui::EndChild();
                }
                count++;
            }

            if (_shape_container_to_delete)
            {
                _shape_containers.erase(_shape_containers.begin() + _shape_container_to_delete.value());
                _shape_container_to_delete = std::nullopt;
                _selected_image = std::nullopt;
            }

            if (_shape_to_delete && _selected_image)
            {
                auto& container = _shape_containers[_selected_image.value()].shapes;
                container.erase(container.begin() + _shape_to_delete.value());

                _shape_to_delete = std::nullopt;
            }

            ImGui::TableNextColumn();
            if (_selected_image.has_value())
            {
                auto& texture = _shape_textures[_selected_image.value()];
                _shape_image.display(texture, _shape_containers[_selected_image.value()].shapes);
            }
        }

        return true;
    }

    void shape_editor_tool::display_toolbar()
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));

        if (ImGui::SmallButton(ICON_MD_FILE_OPEN))
        {
        }
        ToolTip("Load Shapes JSON");
        ImGui::SameLine();

        if (ImGui::SmallButton(ICON_MD_SAVE))
        {
        }
        ToolTip("Save Shapes JSON");
        ImGui::SameLine();

        if (ImGui::SmallButton(ICON_MD_FILE_DOWNLOAD))
        {
            auto filename = save_file_dialog("shapes");
            if (filename)
            {
                save_shapes(filename.value());
            }
        }
        ToolTip("Export BLITZ Basic Shapes");

        ImGui::PopStyleColor();
    }

    void shape_editor_tool::save_shapes(std::filesystem::path const& shapes_file_path) const
    {
        auto all_shapes = std::vector<MPG::simple_image>{};
        for (auto const& shape_container : _shape_containers)
        {
            for (auto const& shape : shape_container.shapes)
            {
                auto export_shape = MPG::crop(shape_container.image, shape.x, shape.y, shape.width, shape.height);
                export_shape = MPG::crop_palette(export_shape, to<u8>(_export_bit_depth), 0);
                all_shapes.push_back(export_shape);
            }
        }

        MPG::save_blitz_shapes(shapes_file_path, all_shapes);
    }
}
