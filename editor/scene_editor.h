#pragma once
#include <memory>
#include <optional>
#include "game_data.h"
#include "imgui.h"

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
        void display_scene_properties(std::shared_ptr<game_data>& data);

        void display_prop_string(std::string_view const& label, std::string& name);
        void display_prop_background(game_data_scene& scene, std::vector<std::string> const& backgrounds);
        void display_prop_regions(game_data_scene& scene);
        void display_prop_region_scalar(std::string_view const& label, uint16_t& value);
        void display_prop_enum(std::string_view const& label, std::vector<std::string> const& values, uint16_t& selected_value);
        void display_prop_list(std::string_view const& label, std::vector<std::string>const& values, std::string& selected);

        void display_scene(game_data_scene& scene, std::vector<GLtexture> const& background_textures);
        void display_scene_toolbar() noexcept;

    private:
        std::weak_ptr<game_data> _game_data;
        std::optional<size_t> _location_index;
        std::optional<size_t> _scene_index;

        int32_t _zoom{ 1 };

        bool _add_region_mode{ false };
        bool _add_region_dragging{ false };
        int32_t _selected_region_index{ -1 };
        ImVec2 _add_region_p0{ -1, -1 };
        ImVec2 _last_mouse{ 0, 0 };

        const std::vector<std::string> pointer_types{ "Normal", "Examine", "Use", "Exit" };
    };
}

