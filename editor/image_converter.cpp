#include "imgui.h"
#include "IconsMaterialDesign.h"

#include <nfd.h>
#include <fstream>

#include "imgui_utils.h"
#include "utils.h"

#include "image_converter.h"

namespace NEONnoir
{
    auto const button_size = ImVec2{ -FLT_MIN, 0 };
    bool image_converter::display()
    {
        auto image_converter_window = ImGui_window(ICON_MD_IMAGE " Image Converter", true, ImGuiWindowFlags_NoCollapse);

        if (image_converter_window.is_closing())
        {
            return false;
        }

        ImGui::SameLine(ImGui::GetWindowWidth() - 30);
        HelpMarker("Converts between indexed BMPs and ILBMs.");

        if (ImGui::Button(ICON_MD_FILE_OPEN " Load image...", button_size))
        {
            auto source_image_path = open_file_dialog("bmp;iff");
            if (source_image_path)
            {
                switch (MPG::determine_image_format(source_image_path.value()))
                {
                case MPG::simple_image_format::ilbm:
                    _source_image = MPG::load_simple_ilbm(source_image_path.value());
                    break;

                case MPG::simple_image_format::bitmap:
                    _source_image = MPG::load_simple_bitmap(source_image_path.value());
                    break;

                default:
                    break;
                }

                if (_source_image.bit_depth > 8)
                    throw std::runtime_error("Only indexed images are supported.");

                _export_bit_depth = _source_image.bit_depth;

                // Convert the image to a usable texture
                if (_source_texture)
                {
                    free_texture(_source_texture.value());
                }
                _source_texture = load_texture(_source_image);
                //_source_texture = load_texture(source_image_path.value());
            }
        }

        if (_source_texture)
        {
            ImGui::Text("Source image");
            //display_image(_source_texture);
            _image_viewer.display(_source_texture.value());

            ImGui::NewLine();

            //ImGui::Combo("Output bit-depth", &_export_bit_depth, " 1-bit\0 2-bit\0 3-bit\0 4-bit\0 5-bit \0 6-bit\0 7-bit\0 8-bit\0\0");
            ImGui::SetNextItemWidth(button_size.x);
            ImGui::SliderInt("##slider", &_export_bit_depth, 1, 8, "Output bit-depth: %d");
            if (ImGui::Button("Export ILBM...", button_size))
            {
                auto dest_image_path = save_file_dialog("iff");
                if (dest_image_path)
                {
                    _dest_image = MPG::simple_image{ _source_image };
                    if (_export_bit_depth > _source_image.bit_depth)
                    {
                        _dest_image.color_palette.resize(static_cast<size_t>(1) << _export_bit_depth);
                    }
                    else if (_export_bit_depth < _source_image.bit_depth)
                    {
                        _dest_image = MPG::crop_palette(_dest_image, to<uint8_t>(_export_bit_depth), 0);
                    }

                    _dest_image.bit_depth = _export_bit_depth;

                    MPG::save_simple_ilbm(dest_image_path.value(), _dest_image);
                }
            }
        }

        return true;
    }

    void image_converter::display_image(std::optional<GLtexture>& texture)
    {
        if (!texture)
            return;

        auto& value = texture.value();
        ImGui::Image(value, ImVec2((float)(value.width), (float)(value.height)));
    }
}
