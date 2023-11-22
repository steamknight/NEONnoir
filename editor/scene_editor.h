#pragma once
#include <memory>
#include <optional>
#include "editor_window_base.h"
#include "game_data.h"
#include "imgui.h"
#include "glfw_utils.h"

namespace NEONnoir
{
    class scene_editor : public editor_window_base
    {
    public:
        scene_editor(std::shared_ptr<game_data> data, GLFWwindow* window)
            : editor_window_base(data, window)
            , _crosshair_cursor(glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR)){ };
        virtual ~scene_editor() = default;

        void display(game_data_location* location, game_data_scene* scene, asset_collection* assets);

    private:
        void display_editor() override;
        void display_scene_properties();

        void display_prop_string(std::string_view const& label, std::string& name);
        void display_prop_string_entry(std::string_view const& label, std::string& name);
        void display_prop_multistring(std::string_view const& label, std::string& value);
        void display_prop_multistring_entry(std::string_view const& label, std::string& string_id);
        void display_prop_descriptions(std::string_view const& label, std::vector<std::string>& values);
        void display_prop_int(std::string_view const& label, u16& value);
        void display_prop_checkbox(std::string_view const& label, bool& value);
        void display_prop_background();
        void display_prop_music();
        void display_prop_regions(std::vector<std::string> const& exit_regions);
        void display_prop_region_scalar(std::string_view const& label, u16& value);
        void display_prop_region_scalar(std::string_view const& label, float& value);
        void display_prop_text_regions();
        void display_prop_combo(std::string_view const& label, std::vector<std::string> const& values, u16& selected_value);
        void display_prop_enum(std::string_view const& label, std::vector<std::string> const& values, u16& selected_value);
        void display_prop_list(std::string_view const& label, std::vector<std::string>const& values, std::string& selected);

        void display_label(std::string_view const& label);

        void display_scene();
        void print_line(std::string_view const& line, ImDrawList* draw_list, ImVec2& p0, ImVec2& p1);
        void display_scene_toolbar() noexcept;

    private:
        game_data_location* _location{ nullptr };
        game_data_scene* _scene{ nullptr };
        asset_collection* _assets{ nullptr };

        i32 _zoom{ 1 };

        bool _add_region_mode{ false };
        bool _add_region_dragging{ false };
        bool _add_text_mode{ false };
        i32 _selected_region_index{ -1 };
        i32 _selected_text_region_index{ -1 };
        ImVec2 _add_region_p0{ -1, -1 };
        ImVec2 _last_mouse{ 0, 0 };

        GLFWcursor_ptr _crosshair_cursor{ nullptr };

        const std::vector<std::string> pointer_types{ "Normal", "Examine", "Use", "Exit" };
    };
}

