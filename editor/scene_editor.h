#pragma once
#include <memory>
#include <optional>
#include "editor_window_base.h"
#include "game_data.h"
#include "imgui.h"

namespace NEONnoir
{
    class scene_editor : public editor_window_base
    {
    public:
        scene_editor(std::shared_ptr<game_data> data) : editor_window_base(data) { };
        virtual ~scene_editor() = default;

        void display(game_data_location* location, game_data_scene* scene);

    private:
        void display_editor() override;
        void display_scene_properties();

        void display_prop_string(std::string_view const& label, std::string& name);
        void display_prop_string_entry(std::string_view const& label, std::string& name);
        void display_prop_multistring(std::string_view const& label, std::vector<std::string>& values);
        void display_prop_int(std::string_view const& label, u16& value);
        void display_prop_background(std::vector<std::string> const& backgrounds);
        void display_prop_regions(std::vector<std::string> const& exit_regions);
        void display_prop_region_scalar(std::string_view const& label, u16& value);
        void display_prop_combo(std::string_view const& label, std::vector<std::string> const& values, u16& selected_value);
        void display_prop_enum(std::string_view const& label, std::vector<std::string> const& values, u16& selected_value);
        void display_prop_list(std::string_view const& label, std::vector<std::string>const& values, std::string& selected);

        void display_scene();
        void display_scene_toolbar() noexcept;

    private:
        game_data_location* _location{ nullptr };
        game_data_scene* _scene{ nullptr };

        i32 _zoom{ 1 };

        bool _add_region_mode{ false };
        bool _add_region_dragging{ false };
        i32 _selected_region_index{ -1 };
        ImVec2 _add_region_p0{ -1, -1 };
        ImVec2 _last_mouse{ 0, 0 };

        const std::vector<std::string> pointer_types{ "Normal", "Examine", "Use", "Exit" };
    };
}

