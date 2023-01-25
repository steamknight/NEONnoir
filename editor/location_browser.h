#pragma once

#include <optional>
#include "editor_window_base.h"

namespace NEONnoir
{
    class location_browser : public editor_window_base
    {
    public:
        location_browser() = default;
        virtual ~location_browser() = default;

        bool is_scene_selected() const noexcept;

        std::optional<size_t> get_selected_location_index() { return _selected_location; }
        std::optional<size_t> get_selected_scene() { return _selected_scene; }

    private:
        std::string_view get_title_name() const noexcept override { return ICON_MD_MAP " Locations"; }
        void display_editor(std::shared_ptr<game_data> data) override;

        void display_location(game_data_location& location, size_t location_index, std::vector<speaker_info>& speakers, string_table& strings);
        void display_location_name(std::string& name, std::string const& id);
        void display_backgrounds(std::vector<std::string>& backgrounds, std::vector<GLtexture>& background_textures, std::string const& id);
        void display_scenes(std::vector<game_data_scene>& scenes, std::string const& id, size_t location_index, string_table& strings);
        void display_speakers(game_data_location& location, std::vector<speaker_info>& speakers);

    private:
        std::optional<size_t> _selected_location;
        std::optional<size_t> _selected_scene;
    };
}

