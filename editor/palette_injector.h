#pragma once
#include <vector>
#include <string_view>
#include "bmp_header.h"

namespace NEONnoir
{
    class palette_injector
    {
    public:
        bool display();

    private:
        std::vector<rgb_color> read_palette(std::string_view const& path);
        void display_palette(std::vector<rgb_color>& palette, std::string_view const& placeholder_label, std::string_view const& open_button_label, bool read_bytes = false);
        void update_palette();

        void display_palette_placeholder(std::string_view const& title);
        void display_palette_colors(std::vector<rgb_color> const& palette);

        void process_image(std::string_view const& output_path);
        void read_image(std::string_view const& image_path);

    private:
        bool _is_dirty{ false };
        bool _is_open{ true };
        int32_t _inject_color_count{ 0 };
        int32_t _prev_inject_color_count{ 0 };
        std::vector<rgb_color> _source_palette;
        std::vector<rgb_color> _dest_palette;
        std::vector<uint8_t> _image_buffer;
    };
}

