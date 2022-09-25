#pragma once
#include <vector>
#include <string_view>
#include "simple_image.h"

namespace NEONnoir
{
    class palette_injector
    {
    public:
        bool display();

    private:
        void display_palette(MPG::simple_image& image, std::string_view const& placeholder_label, std::string_view const& open_button_label);
        void update_palette();

        void display_palette_placeholder(std::string_view const& title);
        void display_palette_colors(MPG::color_palette const& palette);

        void process_image(std::string_view const& output_path);

    private:
        bool _is_dirty{ false };
        bool _is_open{ true };
        int32_t _inject_color_count{ 0 };
        int32_t _prev_inject_color_count{ 0 };
        MPG::simple_image _source_image;
        MPG::simple_image _dest_image;
    };
}

