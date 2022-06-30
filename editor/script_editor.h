#pragma once
#include <TextEditor.h>

#include <vector>
#include <string>

#include "assembler.h"

struct ImFont;

namespace NEONnoir
{
    class script_editor
    {
    public:
        script_editor() = default;

        void display(std::string& script, ImFont* font);

        assembler_result compile();

    private:
        void display_toolbar();
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
        bool _has_error;
        size_t _error_line;
    };
}

