#pragma once
#include <memory>
#include <optional>
#include <string>
#include <filesystem>

#include "simple_image.h"
#include "image_viewer.h"
#include "game_data.h"

namespace NEONnoir
{
    class shape_editor_tool
    {
    public:
        shape_editor_tool() = default;

        bool display();

        void display_toolbar();

    private:
        void load_shapes(std::filesystem::path const& shapes_file_path);
        void save_shapes(std::filesystem::path const& shapes_file_path) const;

    private:
        std::optional<size_t> _selected_image{ std::nullopt };
        std::optional<size_t> _shape_container_to_delete{ std::nullopt };
        std::optional<size_t> _shape_to_delete{ std::nullopt };
        image_viewer _shape_image;

        std::vector<shape_container> _shape_containers{};
        std::vector<GLtexture> _shape_textures{};

        std::filesystem::path _filename{};

        bool _is_open{ true };
        int32_t _export_bit_depth{ 5 };
    };
}