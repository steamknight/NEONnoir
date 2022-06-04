#pragma once
#include <memory>
#include <optional>
#include "game_data.h"

namespace NEONnoir
{
    class location_browser
    {
    public:
        location_browser() = default;

        void use(std::weak_ptr<game_data> game_data);

        void display();
        bool is_scene_selected() const noexcept;

        std::optional<size_t> get_selected_location_index() { return _selected_location; }
        std::optional<size_t> get_selected_scene() { return _selected_scene; }

    private:
        void display_placeholder() noexcept;
        void display_editor(std::shared_ptr<game_data> data);
        void display_location(game_data_location& location, size_t location_index);
        void display_location_name(std::string& name, std::string const& id);
        void display_backgrounds(std::vector<std::string>& backgrounds, std::string const& id);
        void display_scenes(std::vector<game_data_scene>& scenes, std::string const& id, size_t location_index);

    private:
        std::weak_ptr<game_data> _game_data;
        std::optional<size_t> _selected_location;
        std::optional<size_t> _selected_scene;
    };
}

