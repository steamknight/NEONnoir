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
        ImGui::Begin(ICON_MD_PALETTE " Palette Injector", &_is_open, ImGuiWindowFlags_NoCollapse);

        if (!_is_open)
        {
            ImGui::End();
            return false;
        }

        ImGui::SameLine(ImGui::GetWindowWidth() - 30);
        HelpMarker("Prepends one image's color palette into another, updating the image so that it still look as expected.");

        display_palette(_source_image, "No Palette", ICON_MD_FILE_OPEN " Open Palette...");
        ImGui::SameLine(palette_sz.x + 16);
        display_palette(_dest_image, "No Palette", ICON_MD_FILE_OPEN " Open Source BMP...");

        if (_source_image.color_palette.size() > 0 && _dest_image.color_palette.size() > 0)
        {
            ImGui::NewLine();
            ImGui::TextWrapped("Number of colors to inject:");

            ImGui::SetNextItemWidth(palette_sz.x);
            ImGui::SliderInt("##slider", &_inject_color_count, 0, 255, "%d", ImGuiSliderFlags_AlwaysClamp);
            update_palette();

            ImGui::SameLine();
            if (ImGui::Button(ICON_MD_SAVE_AS " Save Destination IFF...", { palette_sz.x, 0 }))
            {
                auto output_path = save_file_dialog("iff");
                if (output_path)
                {
                    process_image(output_path.value());
                    //ImGui::End();
                    // return false;
                }
            }
        }

        ImGui::End();
        return true;
    }

    void palette_injector::display_palette(MPG::simple_image& image, std::string_view const& placeholder_label, std::string_view const& open_button_label)
    {
        ImGui::BeginGroup();

        if (image.color_palette.size() == 0)
        {
            display_palette_placeholder(placeholder_label);
        }
        else
        {
            display_palette_colors(image.color_palette);
        }

        if (ImGui::Button(open_button_label.data(), { palette_sz.x, 0 }))
        {
            auto palette_path = open_file_dialog("iff;bmp");
            if (palette_path)
            {
                image = MPG::load_image(palette_path.value());
            }
        }

        ImGui::EndGroup();
    }

    void palette_injector::display_palette_colors(MPG::color_palette const& palette)
    {
        auto const origin = ImGui::GetCursorScreenPos();
        auto end = origin + palette_sz;

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

            draw_list->AddRectFilled(offset, { offset.x + color_sz, offset.y + color_sz }, IM_COL32(color.r, color.g, color.b, 255));
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

        auto previous_color_count = _dest_image.color_palette.size();
        if (_inject_color_count < _prev_inject_color_count)
        {
            for (auto i = _prev_inject_color_count - 1; i >= _inject_color_count; i--)
            {
                _dest_image.color_palette.erase(_dest_image.color_palette.begin() + i);
            }
        }
        else
        {
            for (auto i = _prev_inject_color_count; i < _inject_color_count; i++)
            {
                _dest_image.color_palette.insert(_dest_image.color_palette.begin() + i, _source_image.color_palette[i]);
            }
        }

        _dest_image.color_palette.resize(previous_color_count);

        _prev_inject_color_count = _inject_color_count;
    }

    void palette_injector::process_image(std::string_view const& output_path)
    {
        std::transform(
            _dest_image.pixel_data.cbegin(),
            _dest_image.pixel_data.cend(),
            _dest_image.pixel_data.begin(),
            [this](uint8_t pixel)
            {
                auto new_pixel = std::min(pixel + _inject_color_count, 255);
                return static_cast<uint8_t>(new_pixel);
            }
        );

        MPG::save_simple_ilbm(output_path, _dest_image);
    }
}
