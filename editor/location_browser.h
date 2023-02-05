#pragma once

#include <optional>
#include "editor_window_base.h"

namespace NEONnoir
{
    class location_browser : public editor_window_base
    {
    public:
        location_browser(std::shared_ptr<game_data> data) : editor_window_base(data) { };
        virtual ~location_browser() = default;

        bool is_scene_selected() const noexcept;

        game_data_location* get_selected_location() { return _selected_location ? &_data->locations[_selected_location.value()] : nullptr; }
        game_data_scene* get_selected_scene() { return get_selected_location() && _selected_scene ? &get_selected_location()->scenes[_selected_scene.value()] : nullptr; }

    private:
        std::string_view get_title_name() const noexcept override { return ICON_MD_MAP " Locations"; }
        void display_editor() override;

        void display_location(game_data_location& location, size_t location_index );
        void display_location_name(std::string& name, std::string const& id);
        void display_backgrounds(std::vector<std::string>& backgrounds, std::vector<GLtexture>& background_textures, std::string const& id);
        void display_scenes(std::vector<game_data_scene>& scenes, std::string const& id, size_t location_index);
        void display_speakers(game_data_location& location);

    private:
        std::optional<size_t> _selected_location;
        std::optional<size_t> _selected_scene;
    };
}

