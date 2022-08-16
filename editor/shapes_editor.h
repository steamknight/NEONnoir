#pragma once
#include <memory>
#include <optional>
#include <string>

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
        void display_editor(game_data_location& location, int32_t shape_start_id);
        void display_placeholder(bool have_data, std::optional<size_t> const& location_index) const noexcept;
        std::string generate_script(game_data_location& location, int32_t shape_start_id) const;

    private:
        std::optional<size_t> _selected_image;
        image_viewer _shape_image;
    };
}