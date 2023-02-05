#include "imgui.h"
#include "imgui_stdlib.h"
#include "IconsMaterialDesign.h"
#include "imgui_utils.h"
#include "glfw_utils.h"

#include <fstream>
#include <sstream>

#include "script_editor.h"
#include "utils.h"
#include "assembler.h"

namespace NEONnoir
{
    void script_editor::display(ImFont* font)
    {
        auto script_window = ImGui_window(ICON_MD_CODE " NOIRscript");

        display_toolbar(_data->script_name);

        if (ImGui::BeginTable("Layout", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable))
        {
            ImGui::TableNextRow();

            display_editor(font);

            if (_text_editor.IsTextChanged())
            {
                extract_metadata();
            }

            ImGui::TableNextColumn();

            display_metadata(_flags, ICON_MD_FLAG " Flags");
            display_metadata(_consts, ICON_MD_TAG " Constants");
            display_metadata(_scripts, ICON_MD_FUNCTIONS " Scripts");

            ImGui::EndTable();
        }
    }

    void script_editor::load_script(std::string_view const& file)
    {
        auto script_file = std::ifstream{ file.data() };
        if (script_file)
        {
            auto script_buffer = std::stringstream{};
            script_buffer << script_file.rdbuf();

            _text_editor.SetText(script_buffer.str());

            _filename = file;
        }
    }

    void script_editor::save_script(std::string_view const& file)
    {
        auto script_file = std::ofstream{ file.data(), std::ios::trunc };
        if (script_file)
        {
            script_file << _text_editor.GetText();
        }
    }

    void script_editor::display_toolbar(std::string& script_name)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
        if (ImGui::SmallButton(ICON_MD_FILE_OPEN))
        {
            auto file = open_file_dialog("nscript");
            if (file)
            {
                load_script(file.value());
                script_name = file.value();
            }
        }
        ToolTip("Open NOIRscript...");

        ImGui::SameLine();

        if (ImGui::SmallButton(ICON_MD_SAVE))
        {
            auto file = save_file_dialog("nscript");
            if (file)
            {
                save_script(file.value());
                script_name = file.value();
            }
        }
        ToolTip("Save NOIRscript...");

        ImGui::SameLine();
        if (ImGui::SmallButton(ICON_MD_MEMORY))
        {
            try
            {
                compile();
            }
            catch (assembler_error&)
            {
                // Do nothing
            }
        }
        ToolTip("Compile");

        if (!_show_editor)
        {
            ImGui::SameLine();
            if (ImGui::SmallButton(ICON_MD_VISIBILITY))
            {
                _show_editor = true;
            }
            ToolTip("Show the editor");
        }

        if (_show_editor)
        {
            ImGui::SameLine();
            if (ImGui::SmallButton(ICON_MD_VISIBILITY_OFF))
            {
                _show_editor = false;
            }
            ToolTip("Hide the editor");
        }

        if (_filename != "")
        {
            ImGui::SameLine();
            if (ImGui::SmallButton(ICON_MD_REFRESH))
            {
                load_script(_filename);
            }
            ToolTip("Reload the script");
        }

        if (_has_error)
        {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, { 1.0f, .5f, 0.f, 1.f });
            ImGui::Text("Compile error on line %d", _error_line);
            ImGui::PopStyleColor(1);
        }

        ImGui::PopStyleColor(1);

    }

    assembler_result script_editor::compile()
    {
        try
        {
            auto massembler = assembler{ _text_editor.GetText() };
            auto result = massembler.assemble();
            _text_editor.SetErrorMarkers({});
            _has_error = false;
            return result;
        }
        catch (assembler_error& error)
        {
            auto error_markers = TextEditor::ErrorMarkers{};
            error_markers[to<int>(error.line_number)] = error.what();
            _text_editor.SetErrorMarkers(error_markers);
            _has_error = true;
            _error_line = error.line_number;
            throw;
        }
    }

    void script_editor::display_editor(ImFont* font)
    {
        ImGui::TableNextColumn();

        if (_show_editor)
        {
            // Render text editor
            ImGui::PushFont(font);
            _text_editor.SetLanguageDefinition(NEONscript_language_definition());
            _text_editor.Render(ICON_MD_MEMORY "NOIRscript", ImVec2(), true);

            ImGui::PopFont();
        }
    }

    void script_editor::display_metadata(std::vector<std::string>const & data, std::string_view const& label)
    {
        if (ImGui::BeginTable(label.data(), 1))
        {
            ImGui::TableSetupColumn(label.data(), 100);
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            for (auto const& value : data)
            {
                ImGui::TextUnformatted(value.c_str());
            }

            ImGui::EndTable();
        }
    }

    bool is_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
    bool is_digit(char c) { return c >= '0' && c <= '9'; }
    bool is_alphanumeric(char c) { return is_alpha(c) || is_digit(c); }

    std::string get_identifier(std::string const& source, size_t &index)
    {
        auto current = index;
        while (current < source.length() && is_alphanumeric(source[current]))
            current++;

        auto start = index;
        index = current + 1;
        return source.substr(start, current - start);
    }

    size_t skip_whitespace(std::string const& source, size_t index)
    {
        auto done = false;
        while (index < source.length() && !done)
        {
            switch (source[index])
            {
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                index++;
                break;
            case ';':
                while (source[index] != '\n' && index < source.length()) index++;
                break;
            default:
                done = true;
            }
        }

        return index;
    }

    size_t skip_number(std::string const& source, size_t index)
    {
        while (index < source.length() && is_digit(source[index]))
            index++;

        return index;
    }

    size_t skip_string(std::string const& source, size_t index)
    {
        if (source[index] == '\"')
        {
            index++;
            while (index < source.length() && source[index] != '\"')
                index++;
        }

        return index;
    }

    void script_editor::extract_metadata()
    {
        auto const& script = _text_editor.GetText();
        // auto start = 0_z;

        _flags.clear();
        _consts.clear();
        _scripts.clear();

        for (size_t current = 0; current < script.length(); current++)
        {
            current = skip_whitespace(script, current);

            if (script[current] != '.')
            {
                current++;
                continue;
            }

            current++;

            switch (script[current])
            {
            case 'F': // Check if .FLAGS
            {
                auto identifier = get_identifier(script, current);
                if (identifier.compare("FLAGS") == 0)
                {
                    while (true)
                    {
                        current = skip_whitespace(script, current);
                        if (is_alpha(script[current]))
                        {
                            auto flag = get_identifier(script, current);
                            _flags.push_back(flag);
                        }
                        else if (current >= script.length() || script[current] == '.')
                        {
                            current--;
                            break;
                        }
                        else
                        {
                            // Invalid character
                            break;
                        }
                    }
                }

                break;
            }
            case 'C': // Check if .CONST
            {
                auto identifier = get_identifier(script, current);
                if (identifier.compare("CONST") == 0)
                {
                    while (true)
                    {
                        current = skip_whitespace(script, current);
                        if (is_alpha(script[current]))
                        {
                            auto value = get_identifier(script, current);
                            _consts.push_back(value);

                            current = skip_whitespace(script, current);
                            current = skip_number(script, current);
                        }
                        else if (current >= script.length() || script[current] == '.')
                        {
                            current--;
                            break;
                        }
                        else
                        {
                            // Invalid character
                            break;
                        }
                    }
                }

                break;
            }
            case 'S': // Check if .SCRIPT
            {
                auto identifier = get_identifier(script, current);
                if (identifier.compare("SCRIPT") == 0)
                {
                    while (true)
                    {
                        current = skip_whitespace(script, current);
                        if (is_alpha(script[current]))
                        {
                            auto value = get_identifier(script, current);
                            _scripts.push_back(value);

                            //while (current < script.length() && script[current] != '.')
                            //{
                            //    current = skip_whitespace(script, current);
                            //    current++;
                            //}

                            //current--;
                            break;
                        }
                        else if (current >= script.length() || script[current] == '.')
                        {
                            current--;
                            break;
                        }
                        else
                        {
                            // Invalid character
                            break;
                        }
                    }
                }

                break;
            }
            case 'T': // completely skip the TEXT section
            {
                get_identifier(script, current);
                while (current < script.length() && script[current] != '.')
                {
                    current = skip_whitespace(script, current);
                    get_identifier(script, current);
                    current = skip_whitespace(script, current);
                    current = skip_string(script, current);
                }

                current--;
            }
            }
        }
    }

    TextEditor::LanguageDefinition const& script_editor::NEONscript_language_definition()
    {
        static bool inited = false;
        static TextEditor::LanguageDefinition langDef;
        if (!inited)
        {
            static const char* const keywords[] = {
                ".FLAGS", ".CONST", ".TEXT", ".SCRIPT"
            };

            for (auto& k : keywords)
                langDef.mKeywords.insert(k);

            static const char* const identifiers[] = {
                "noop", "clear", "set", "load", "store", "and", "or", "not", "jump", "jift", "jiff",
                "setbg", "draw", "hasi ", "addi", "remi", "goto", "map", "text", "gameover"
            };
            for (auto& k : identifiers)
            {
                TextEditor::Identifier id;
                id.mDeclaration = "Built-in function";
                langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
            }

            //langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("L?\\\"(\\\\.|[^\\\"])*\\\"", TextEditor::PaletteIndex::String));
            //langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("\\\'[^\\\']*\\\'", TextEditor::PaletteIndex::String));
            //langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?", TextEditor::PaletteIndex::Number));
            //langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?", TextEditor::PaletteIndex::Number));
            //langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("[+-]?[0-9]+[Uu]?[lL]?[lL]?", TextEditor::PaletteIndex::Number));
            //langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("[a-zA-Z_][a-zA-Z0-9_]*", TextEditor::PaletteIndex::Identifier));
            //langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", TextEditor::PaletteIndex::Punctuation));

            langDef.mSingleLineComment = ";";
            langDef.mCommentStart = "/*";
            langDef.mCommentEnd = "*/";

            langDef.mCaseSensitive = true;
            langDef.mAutoIndentation = false;

            langDef.mName = "NEONscript";

            inited = true;
        }
        return langDef;
    }
}
