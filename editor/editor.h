#pragma once
#include <glfw/glfw3.h>
#include <TextEditor.h>
#include "glfw_utils.h"
#include <memory>
#include <functional>
#include <filesystem>

#include "palette_injector.h"
#include "image_converter.h"
#include "game_data.h"
#include "location_browser.h"
#include "scene_editor.h"
#include "script_editor.h"
#include "dialogue_editor.h"
#include "shapes_editor.h"
#include "shape_editor_tool.h"
#include "speaker_editor.h"
#include "string_table_editor.h"
#include "manifest_editor.h"

struct ImFont;

namespace NEONnoir
{
    class editor
    {
    public:
        struct settings
        {
            std::string title;
            i32 width{ 1600 };
            i32 height{ 900 };
        };

    public:
        explicit editor(settings const& settings);
        ~editor() noexcept;

        void run();
        void load_fonts() noexcept;

        void process_main_menu();

    private:
        void new_project();
        void load_project(std::filesystem::path const& file_path);
        void save_project();

        void initialize_editors();

        void export_neon_file();

    private:
        GLFWwindow_ptr _window;
        ImFont* _ui_font;
        ImFont* _monospaced_font;
        float _dpi_scale_x;
        float _dpi_scale_y;

        std::unique_ptr<palette_injector> _palette_injector;
        std::unique_ptr<image_converter> _image_converter;
        std::unique_ptr<shape_editor_tool> _shape_editor_tool;
        std::shared_ptr<game_data> _game_data;
        std::string _script;

        std::unique_ptr<location_browser> _location_browser{};
        std::unique_ptr<scene_editor> _scene_editor{};
        std::unique_ptr<script_editor> _script_editor{};
        std::unique_ptr<dialogue_editor> _dialogue_editor{};
        std::unique_ptr<shapes_editor> _shapes_editor{};
        std::unique_ptr<speaker_editor> _speaker_editor{};
        std::unique_ptr<string_table_editor> _string_table{};
        std::unique_ptr<manifest_editor> _manifest_editor{};

        bool _show_properties_popup{ false };
        bool _show_error_popup{ false };
        std::string _error_message{};
    };
};

