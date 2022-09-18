#pragma once

#include <optional>
#include "simple_image.h"
#include "glfw_utils.h"

namespace NEONnoir
{
    class image_converter
    {
    public:
        bool display();

    private:
        void display_image(std::optional<GLtexture>& image);

    private:
        MPG::simple_image _source_image;
        MPG::simple_image _dest_image;

        std::optional<GLtexture> _source_texture{ std::nullopt };
        std::optional<GLtexture> _dest_texture{ std::nullopt };

        int32_t _export_bit_depth{ 0 };
    };
}