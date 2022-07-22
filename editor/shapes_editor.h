#pragma once
#include <memory>
#include <optional>

#include "game_data.h"

namespace NEONnoir
{
    class shapes_editor
    {
    public:
        shapes_editor() = default;

        void display(std::weak_ptr<game_data> game_data, std::optional<size_t> const& location_index);

    private:
        void display_editor(game_data_location& location);
        void display_placeholder(bool have_data, std::optional<size_t> const& location_index) const noexcept;

    };
}
