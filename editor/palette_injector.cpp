#include "imgui.h"
#include "IconsMaterialDesign.h"
#include <nfd.h>
#include <fstream>

#include "palette_injector.h"
#include "imgui_utils.h"
#include "utils.h"

namespace NEONnoir
{
    auto constexpr color_sz = 16.f;
    auto constexpr colors_per_run = 16.f;
    auto constexpr color_run_width = color_sz * colors_per_run;
    auto constexpr colors_per_col = 256.f / colors_per_run;
    auto constexpr palette_sz = ImVec2{ color_run_width, color_run_width };

    bool palette_injector::display()
    {
        ImGui::SetNextWindowContentSize({ 520, 0 });
        ImGui::Begin("Palette Injector", &_is_open, ImGuiWindowFlags_NoCollapse);

        if (!_is_open)
        {
            ImGui::End();
            return false;
        }

        ImGui::SameLine(ImGui::GetWindowWidth() - 30);
        HelpMarker("Prepends one image's color palette into another, updating the image so that it still look as expected.");

        display_palette(_source_palette, "No Palette", ICON_MD_FILE_OPEN " Open Palette BMP...");
        ImGui::SameLine(palette_sz.x + 16);
        display_palette(_dest_palette, "No Palette", ICON_MD_FILE_OPEN " Open Source BMP...", true);

        if (_source_palette.size() > 0 && _dest_palette.size() > 0)
        {
            ImGui::NewLine();
            ImGui::TextWrapped("Number of colors to inject:");

            ImGui::SetNextItemWidth(palette_sz.x);
            ImGui::SliderInt("##slider", &_inject_color_count, 0, 255, "%d", ImGuiSliderFlags_AlwaysClamp);
            update_palette();

            ImGui::SameLine();
            if (ImGui::Button(ICON_MD_SAVE_AS " Save Destination BMP...", { palette_sz.x, 0 }))
            {
                auto output_path = save_file_dialog("bmp");
                if (output_path)
                {
                    process_image(output_path.value());
                    ImGui::End();
                    return false;
                }
            }
        }

        ImGui::End();
        return true;
    }

    std::vector<rgb_color> palette_injector::read_palette(std::string_view const& path)
    {
        auto source = std::ifstream{ path.data(), std::ios::binary };
        if (!source)
        {
            // TODO Show error
        }

        auto header = bmp_header{};
        auto info = bmp_info_header{};
        source.read(reinterpret_cast<char*>(&header), sizeof(bmp_header));
        source.read(reinterpret_cast<char*>(&info), sizeof(bmp_info_header));

        if (header.format != 0x4D42) // "BM"
        {
            //throw std::runtime_error("Palette donor file is not a BMP file.");
        }
        if (info.bits_per_pixel != 8)
        {
            //throw std::runtime_error("Palette donor file is not an 8bit indexed BMP.");
        }

        // How many colors are there in the palette?
        auto color_count = info.palette_color_count > 0 ? info.palette_color_count : 256;
        
        auto palette = std::vector<rgb_color>{};
        palette.resize(color_count);
        source.read(reinterpret_cast<char*>(&palette[0]), sizeof(rgb_color) * color_count);

        return palette;
    }

    void palette_injector::display_palette(std::vector<rgb_color>& palette, std::string_view const& placeholder_label, std::string_view const& open_button_label, bool read_bytes)
    {
        ImGui::BeginGroup();

        if (palette.size() == 0)
        {
            display_palette_placeholder(placeholder_label);
        }
        else
        {
            display_palette_colors(palette);
        }

        if (ImGui::Button(open_button_label.data(), { palette_sz.x, 0 }))
        {
            auto palette_path = open_file_dialog("bmp");
            if (palette_path)
            {
                palette = read_palette(palette_path.value());

                if (read_bytes)
                {
                    read_image(palette_path.value());
                }
            }
        }

        ImGui::EndGroup();
    }

    void palette_injector::display_palette_colors(std::vector<rgb_color> const& palette)
    {
        auto const origin = ImGui::GetCursorScreenPos();
        auto end = origin + palette_sz;

        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        ImVec2 color_p0 = origin;
        auto count = 0;
        for (auto const& color : palette)
        {
            auto offset = ImVec2
            {
                color_p0.x + ((fmodf(to<float>(count), colors_per_run)) * color_sz),
                color_p0.y + (floorf(count / colors_per_col) * color_sz)
            };

            draw_list->AddRectFilled(offset, { offset.x + color_sz, offset.y + color_sz }, IM_COL32(color.red, color.green, color.blue, 255));
            count++;

            if (count >= 256) break;
        }

        // Reset the cursor position
        ImGui::SetCursorScreenPos({ origin.x, origin.y + palette_sz.y + 8 });
    }

    void palette_injector::display_palette_placeholder(std::string_view const& title)
    {
        auto const origin = ImGui::GetCursorScreenPos();
        auto end = origin + palette_sz;

        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        draw_list->AddRectFilled(origin, end, IM_COL32(8, 32, 64, 255));
        draw_list->AddRect(origin, end, IM_COL32(16, 64, 128, 255));

        // Calculate the offsets to middle align the text
        auto text_sz = ImGui::CalcTextSize(title.data());
        auto center = origin + (palette_sz / 2);
        ImGui::SetCursorScreenPos(center - (text_sz / 2));
        ImGui::TextColored({ 16, 64, 128, 255 }, title.data());

        // Reset the cursor position
        ImGui::SetCursorScreenPos({ origin.x, origin.y + palette_sz.y + 8 });
    }

    void palette_injector::update_palette()
    {
        if (_prev_inject_color_count == _inject_color_count) return;

        if (_inject_color_count < _prev_inject_color_count)
        {
            for (auto i = _prev_inject_color_count - 1; i >= _inject_color_count; i--)
            {
                _dest_palette.erase(_dest_palette.begin() + i);
            }
        }
        else
        {
            for (auto i = _prev_inject_color_count; i < _inject_color_count; i++)
            {
                _dest_palette.insert(_dest_palette.begin() + i, _source_palette[i]);
            }
        }

        _prev_inject_color_count = _inject_color_count;
    }

    void palette_injector::process_image(std::string_view const& output_path)
    {
        // Shift the palette forward
        auto palette = reinterpret_cast<rgb_color*>(_image_buffer.data() + sizeof(bmp_header) + sizeof(bmp_info_header));
        auto source = palette + (_inject_color_count);
        auto remaining_colors = 256 - _inject_color_count;
        for (int i = remaining_colors - 1; i >= 0; i--)
        {
            *(source + i) = *(palette + i);
        }

        // Add the new palette at the beginning
        for (auto i = 0; i < _inject_color_count; i++)
        {
            *(palette + i) = _source_palette[i];
        }

        // Update image data
        auto header = reinterpret_cast<bmp_header*>(_image_buffer.data());
        auto info = reinterpret_cast<bmp_info_header*>(_image_buffer.data() + sizeof(bmp_header));
        auto image_data = _image_buffer.data() + header->image_offset;
        auto image_size = info->bmp_size;

        for (auto i = 0u; i < image_size; i++)
        {
            *(image_data + i) = std::min((uint8_t)(*(image_data + i) + (uint8_t)_inject_color_count), (uint8_t)255);
        }

        auto output_file = std::ofstream{ output_path.data(), std::ios::binary | std::ios::trunc };
        output_file.write(reinterpret_cast<char*>(_image_buffer.data()), _image_buffer.size());
        output_file.close();
    }

    void palette_injector::read_image(std::string_view const& image_path)
    {
        auto source = std::ifstream{ image_path.data(), std::ios::binary | std::ios::ate };
        if (!source)
        {
            // TODO throw std::runtime_error(std::string{ "Can't open source file." });
        }

        auto file_size = source.tellg();
        source.seekg(0);

        _image_buffer.resize(file_size);
        source.read(reinterpret_cast<char*>(_image_buffer.data()), file_size);
        source.close();
    }
}