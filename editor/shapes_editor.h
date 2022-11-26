#pragma once
#include <memory>
#include <optional>
#include <string>
#include <filesystem>

#include "game_data.h"
#include "image_viewer.h"

namespace NEONnoir
{
    class shapes_editor
    {
    public:
        shapes_editor() = default;

        void display(std::weak_ptr<game_data> game_data, std::optional<size_t> const& location_index);

    private:
        void display_editor(game_data_location& location, i32 shape_start_id);
        void display_placeholder(bool have_data) const noexcept;
        void save_shapes(std::filesystem::path const& shapes_file_path, game_data_location& location) const;

    private:
        std::optional<size_t> _selected_image{ std::nullopt };
        std::optional<size_t> _shape_container_to_delete{ std::nullopt };
        std::optional<size_t> _shape_to_delete{ std::nullopt };
        image_viewer _shape_image;
    };
}
