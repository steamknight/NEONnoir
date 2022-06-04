#pragma once
#include <memory>
#include <optional>
#include "game_data.h"

namespace NEONnoir
{
    class scene_editor
    {
    public:
        scene_editor() = default;

        void use(std::weak_ptr<game_data> game_data, std::optional<size_t> const& location_index, std::optional<size_t> const&scene_index);
        void display();

    private:
        void display_placeholder() const;
        void display_editor(std::shared_ptr<game_data>& data);

    private:
        std::weak_ptr<game_data> _game_data;
        std::optional<size_t> _location_index;
        std::optional<size_t> _scene_index;
    };
}

