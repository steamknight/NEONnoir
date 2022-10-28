#pragma once
#include <TextEditor.h>

#include <vector>
#include <string>

#include "game_data.h"
#include "assembler.h"

struct ImFont;

namespace NEONnoir
{
    class script_editor
    {
    public:
        script_editor() = default;

        void display(std::weak_ptr<game_data> game_data, ImFont* font);
        void load_script(std::string_view const& file);
        void save_script(std::string_view const& file);

        assembler_result compile();

    private:
        void display_toolbar(std::string& script_name);
        void display_editor(ImFont* font);
        void display_metadata(std::vector<std::string>const& data, std::string_view const& label);
        void extract_metadata();

        TextEditor::LanguageDefinition const& NEONscript_language_definition();

    private:
        TextEditor _text_editor;
        std::vector<std::string> _scripts;
        std::vector<std::string> _flags;
        std::vector<std::string> _consts;
        bytecode _bytecode;
        bool _has_error{ false };
        bool _show_editor{ false };
        size_t _error_line{0};
        std::string _filename{};
    };
}

