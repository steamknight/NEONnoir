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

        void display_prop_name(std::string& name);
        void display_prop_background(game_data_scene& scene, std::vector<std::string> const& backgrounds);

        void display_scene(game_data_scene& scene, std::vector<GLtexture> const& background_textures);

    private:
        std::weak_ptr<game_data> _game_data;
        std::optional<size_t> _location_index;
        std::optional<size_t> _scene_index;

        int32_t _zoom{ 1 };

        bool _add_region_mode{ false };
        bool _add_region_dragging{ false };
        ImVec2 _add_region_p0{ -1, -1 };
    };
}

