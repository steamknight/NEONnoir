#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_utils.h"
#include "glfw_utils.h"

#include <fstream>

#include "types.h"
#include "speaker_editor.h"
#include "no_person.h"

namespace fs = std::filesystem;

namespace NEONnoir
{
    /*static*/ MPG::simple_image speaker_editor::_no_person = MPG::simple_image
    {
        100, 100, 3,
        no_person_color_palette,
        no_person_pixel_data
    };

    void speaker_editor::display_editor(std::shared_ptr<game_data> data)
    {
        display_toolbar(data->speakers);

        size_t count = 0;
        auto index_to_delete = std::optional<size_t>{ std::nullopt };

        for (auto& speaker : data->speakers)
        {
            if (display_speaker(speaker))
            {
                ImGui::SameLine();

                auto remaining_width = ImGui::GetContentRegionAvail().x;

                if (remaining_width < speaker.image_texture.width * _zoom)
                {
                    ImGui::NewLine();
                }
            }
            else
            {
                index_to_delete = count;
            }

            count++;
        }

        if (index_to_delete)
        {
            data->speakers.erase(data->speakers.begin() + index_to_delete.value());
            _speaker_list_with_empty = std::nullopt;
        }
    }

    void speaker_editor::display_toolbar(std::vector<speaker_info>& speakers) noexcept
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));

        if (ImGui::Button(ICON_MD_PERSON_ADD))
        {
            speakers.push_back({ "New Speaker", "", _no_person, load_texture(_no_person)});
            _speaker_list_with_empty = std::nullopt;
        }

        ToolTip("Add new speaker");
        ImGui::SameLine();

        if (ImGui::SmallButton(ICON_MD_ZOOM_IN))
        {
            _zoom++;
        }
        ToolTip("Zoom In");
        ImGui::SameLine();

        if (ImGui::SmallButton(ICON_MD_ZOOM_OUT) && _zoom > 1)
        {
            _zoom--;
        }
        ToolTip("Zoom Out");
        ImGui::SameLine();

        ImGui::Text("x%d", _zoom);
        ImGui::SameLine();

        if (ImGui::Button(ICON_MD_FILE_DOWNLOAD))
        {
            auto filename = save_file_dialog("shapes");
            if (filename)
            {
                save_shapes(filename.value(), speakers);
            }
        }
        ToolTip("Export BLITZ Basic Shapes");
        ImGui::SameLine();

        if (ImGui::Button(ICON_MD_SWITCH_ACCOUNT))
        {
            auto filename = save_file_dialog("mpsh");
            if (filename)
            {
                save_mpsh_shapes(filename.value(), speakers);
            }
        }
        ToolTip("Export MPSH shape collection");
        ImGui::SameLine();

        ImGui::SetNextItemWidth(200);
        ImGui::SliderInt("##speaker_bit_depth", &_bit_depth, 1, 8, "Bit depth: %d");

        ImGui::PopStyleColor();
    }

    bool speaker_editor::display_speaker(speaker_info& speaker) noexcept
    {
        auto const width = to<float>(speaker.image_texture.width * _zoom);

        ImGui::BeginGroup();
        ImGui::PushID((void*)&speaker);

        if (ImGui::ImageButton((void*)(intptr_t)speaker.image_texture.texture_id, ImVec2(width, (float)(speaker.image_texture.height * _zoom))))
        {
            auto file = open_file_dialog("iff;bmp");
            if (file)
            {
                speaker.image = MPG::load_image(file.value());
                
                free_texture(speaker.image_texture);
                speaker.image_texture = load_texture(speaker.image);

                speaker.image_path = fs::relative(fs::path{ file.value().data() }, fs::current_path()).string();
            }
        }

        // The button is going to have padding, so we should account for it on both left and right
        auto const new_width = width + (2.f * ImGui::GetStyle().FramePadding.x);

        ImGui::SetNextItemWidth(new_width);
        ImGui::InputText("##", &speaker.name);


        auto should_keep = !DeleteButton("##", "", ImVec2{ new_width , 0 });

        ImGui::PopID();
        ImGui::EndGroup();

        return should_keep;
    }

    void speaker_editor::save_shapes(std::filesystem::path const& shapes_file_path, std::vector<speaker_info>& speakers) const
    {
        auto all_shapes = std::vector<MPG::simple_image>{};
        for (auto const& speaker : speakers)
        {
            auto export_shape = MPG::crop_palette(speaker.image, to<u8>(_bit_depth), 0);
            all_shapes.push_back(export_shape);
        }

        MPG::save_blitz_shapes(shapes_file_path, all_shapes);
    }

    void speaker_editor::save_mpsh_shapes(std::filesystem::path const& shapes_file_path, std::vector<speaker_info>& speakers) const
    {
        auto all_shapes = std::vector<MPG::blitz_shapes>{};
        for (auto const& speaker : speakers)
        {
            auto cropped = MPG::crop_palette(speaker.image, to<u8>(_bit_depth), 0);
            auto shape = MPG::image_to_blitz_shapes(cropped);

            all_shapes.push_back(shape);
        }

        if (auto impish_file = std::ofstream{ shapes_file_path, std::ios::binary | std::ios::trunc })
        {
            auto offset = 0u;

            // Write header
            char magic[] = { 'M', 'P', 'S', 'H' };
            impish_file.write(magic, 4);                            // Magic number
            write(impish_file, 1u);                                 // Version. Always 1 for now
            write(impish_file, to<u32>(speakers.size()));      // Number of shapes
            offset += (sizeof(u32) * 3);

            // Push the offset past where the manifest will go
            // The manifest is 2 u32s (offset and size) per entry
            offset += (sizeof(u32) * 2 * to<u32>(speakers.size()));

            // Write the manifest
            for (auto const& shape : all_shapes)
            {
                write(impish_file, offset);

                auto size = to<u32>(shape.get_size());
                write(impish_file, size);

                offset += size;
            }

            // Write all the shapes
            for (auto const& shape : all_shapes)
            {
                // Write the shape header
                write(impish_file, shape.width);
                write(impish_file, shape.height);
                write(impish_file, shape.bit_depth);
                write(impish_file, shape.ebwidth);
                write(impish_file, shape.blitsize);

                // Handle is in the top left. Perhaps I can add support for moving them later
                write(impish_file, shape.handle_x);     // x
                write(impish_file, shape.handle_y);     // y

                // Data and cookie pointers. They seem to always be nonsense values in the shapes files created by Blitz
                write(impish_file, shape.data_ptr);     // data
                write(impish_file, shape.cookie_ptr);     // cookie

                write(impish_file, shape.onebpmem);
                write(impish_file, shape.onebpmemx);
                write(impish_file, shape.allbpmem);
                write(impish_file, shape.allbpmemx);

                write(impish_file, shape.padding);     // padding

                // Write out the shape's bitplanes
                if (shape.data.size() > 0)
                {
                    impish_file.write(force_to<char const*>(&shape.data[0]), shape.data.size());
                }
            }
        }
    }
}