#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_utils.h"
#include "glfw_utils.h"

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
        }
    }

    void speaker_editor::display_toolbar(std::vector<speaker_info>& speakers) noexcept
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));

        if (ImGui::Button(ICON_MD_PERSON_ADD))
        {
            speakers.push_back({ "New Speaker", "", _no_person, load_texture(_no_person)});
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

        ImGui::SetNextItemWidth(200);
        ImGui::SliderInt("##speaker_bit_depth", &_bit_depth, 1, 8, "Bit depth: %d");

        ImGui::PopStyleColor();
    }

    bool speaker_editor::display_speaker(speaker_info& speaker) noexcept
    {
        auto const width = static_cast<float>(speaker.image_texture.width * _zoom);

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
            auto export_shape = MPG::crop_palette(speaker.image, _bit_depth, 0);
            all_shapes.push_back(export_shape);
        }

        MPG::save_blitz_shapes(shapes_file_path, all_shapes);
    }
}