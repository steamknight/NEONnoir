#pragma once
#include <glfw/glfw3.h>
#include <TextEditor.h>
#include "glfw_utils.h"
#include <memory>
#include <functional>

#include "palette_injector.h"
#include "image_converter.h"
#include "game_data.h"
#include "location_browser.h"
#include "scene_editor.h"
#include "script_editor.h"
#include "dialogue_editor.h"
#include "shapes_editor.h"

struct ImFont;

namespace NEONnoir
{
    class editor
    {
    public:
        struct settings
        {
            std::string title;
            int32_t width{ 1600 };
            int32_t height{ 900 };
        };

    public:
        explicit editor(settings const& settings);
        ~editor() noexcept;

        void run();
        void load_fonts() noexcept;

        void process_main_menu();

    private:
        GLFWwindow_ptr _window;
        ImFont* _ui_font;
        ImFont* _monospaced_font;
        float _dpi_scale_x;
        float _dpi_scale_y;

        std::unique_ptr<palette_injector> _palette_injector;
        std::unique_ptr<image_converter> _image_converter;
        std::shared_ptr<game_data> _game_data;
        std::string _script;

        location_browser _location_browser{};
        scene_editor _scene_editor{};
        script_editor _script_editor{};
        dialogue_editor _dialogue_editor{};
        shapes_editor _shapes_editor{};

        bool _show_properties_popup;
    };
};

