#pragma once
#include "imgui.h"
#include "glfw_utils.h"
#include <vector>

#include "game_data.h"

namespace NEONnoir
{
    class image_viewer
    {
    public:
        image_viewer() = default;
        ~image_viewer() = default;

        void display(GLtexture const& texture) noexcept;
        void display(GLtexture const&, std::vector<shape>& regions) noexcept;
        void selected_region(i32 selected_region) { _selected_region_index = selected_region; }

    private:
        ImVec2 _last_mouse{ 0.f, 0.f };
        int _zoom{ 1 };

        bool _add_region_mode{ false };
        bool _add_region_dragging{ false };

        bool _show_autogrid_popup{ false };
        i32 _cell_width{ 1 };
        i32 _cell_height{ 1 };

        i32 _selected_region_index{ -1 };
        ImVec2 _add_region_p0{ -1, -1 };
    };
}

