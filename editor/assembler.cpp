#include "assembler.h"
#include <iostream>
#include <iomanip>
#include <fstream>

#include "utils.h"

namespace NEONnoir
{
    assembler::assembler(std::string const& source) : _source(source)
    {
        _opcodes = named_values<opcode_spec>
        {
            // Name           Opcode               Type1                   Type2                   Type3
            { "noop",       { (int16_t)0x00,   {                                                                          }}},
            { "clear",      { (int16_t)0x01,   {                                                                          }}},
            { "set",        { (int16_t)0x02,   {                                                                          }}},
            { "load",       { (int16_t)0x03,   {   ParamType::FLAG                                                        }}},
            { "store",      { (int16_t)0x04,   {   ParamType::FLAG                                                        }}},
            { "and" ,       { (int16_t)0x10,   {   ParamType::FLAG                                                        }}},
            { "or",         { (int16_t)0x11,   {   ParamType::FLAG                                                        }}},
            { "not",        { (int16_t)0x12,   {                                                                          }}},
            { "jump",       { (int16_t)0x20,   {   ParamType::LABEL                                                       }}},
            { "jift",       { (int16_t)0x21,   {   ParamType::LABEL                                                       }}},
            { "jiff",       { (int16_t)0x22,   {   ParamType::LABEL                                                       }}},
            { "end",        { (int16_t)0x2F,   {                                                                          }}},
            { "setbg",      { (int16_t)0x30,   {   ParamType::CONST                                                       }}},
            { "draw",       { (int16_t)0x40,   {   ParamType::CONST,       ParamType::NUMBER,      ParamType::NUMBER      }}},
            { "dlg",        { (int16_t)0x50,   {   ParamType::CONST                                                       }}},
            { "choff",      { (int16_t)0x51,   {   ParamType::CONST                                                       }}},
            { "chon",       { (int16_t)0x52,   {   ParamType::CONST                                                       }}},
            { "pageoff",    { (int16_t)0x53,   {   ParamType::CONST                                                       }}},
            { "pageon",     { (int16_t)0x54,   {   ParamType::CONST                                                       }}},
            { "dlgimg",     { (int16_t)0x55,   {   ParamType::CONST                                                       }}},
            { "dlgname",    { (int16_t)0x5F,   {   ParamType::CONST                                                       }}},
            { "dlgend",     { (int16_t)0x5F,   {                                                                          }}},
            { "hasi",       { (int16_t)0x60,   {   ParamType::CONST                                                       }}},
            { "addi",       { (int16_t)0x61,   {   ParamType::CONST                                                       }}},
            { "remi",       { (int16_t)0x62,   {   ParamType::CONST                                                       }}},
            { "mod",        { (int16_t)0x70,   {   ParamType::TEXT,        ParamType::NUMBER                              }}},
            { "modpause",   { (int16_t)0x71,   {   ParamType::CONST                                                       }}},
            { "modplay",    { (int16_t)0x72,   {   ParamType::CONST                                                       }}},
            { "modkill",    { (int16_t)0x73,   {   ParamType::CONST                                                       }}},
            { "goto",       { (int16_t)0x80,   {   ParamType::CONST                                                       }}},
            { "map",        { (int16_t)0x81,   {                                                                          }}},
            { "goloc",      { (int16_t)0x83,   {   ParamType::CONST,       ParamType::CONST                               }}},
            { "quit",       { (int16_t)0x8E,   {                                                                          }}},
            { "gameover",   { (int16_t)0x8F,   {   ParamType::TEXT                                                        }}},
            { "text",       { (int16_t)0x90,   {   ParamType::TEXT                                                        }}},
            { "txtstart",   { (int16_t)0x91,   {                                                                          }}},
            { "txtend",     { (int16_t)0x92,   {   ParamType::NUMBER,      ParamType::NUMBER                              }}},
            { "txtput",     { (int16_t)0x93,   {   ParamType::TEXT,        ParamType::NUMBER                              }}},
            { "txtnl",      { (int16_t)0x94,   {                                                                          }}},
        };
    }

    assembler_result assembler::assemble()
    {
        parse_assembly();
        update_jump_references();

        return { _bytecode, _scripts, _strings, _flags };
    }

    void assembler::parse_assembly()
    {
        std::cout << "Pass 1: Parsing source..." << std::endl;
        while (true)
        {
            if (is_at_end())
                break;

            skip_whitepace();

            // All sections start with a .
            _scan_start = _scan_current;
            if (peek() == '.')
            {
                advance();
                switch (peek())
                {
                case 'F':
                    parse_flags();
                    break;

                case 'C':
                    parse_constants();
                    break;

                case 'T':
                    parse_text();
                    break;

                case 'S':
                    parse_script();
                    break;

                default:
                    break;
                }
            }
            else
            {
                throw assembler_error("Unexepected character.", _scan_line);
            }
        }
    }

    void assembler::update_jump_references()
    {
        std::cout << "Pass 2: Updating jump references..." << std::endl;
        for (auto const& placeholder : _placeholders)
        {
            if (_bytecode[placeholder.index] != 0x7FFF)
            {
                throw assembler_error("Jump location incorrect.", 0);
            }

            if (!_labels.count(placeholder.name))
            {
                throw assembler_error("Missing label: " + placeholder.name, 0);
            }

            _bytecode[placeholder.index] = to<int32_t>(_labels[placeholder.name] - placeholder.index + 1);
        }
    }

    void assembler::skip_whitepace()
    {
        while (true)
        {
            auto c = peek();
            switch (c)
            {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;

            case '\n':
                _scan_line++;
                advance();
                break;

            case ';':
                while (peek() != '\n' && !is_at_end())
                    advance();
                break;

            default:
                return;
            }
        }
    }

    bool assembler::check_word(size_t token_start, size_t token_length, const char* rest)
    {
        return (_scan_current - _scan_start == token_start + token_length && _source.compare(_scan_start, token_length, rest) == 0);
    }

    void assembler::parse_flags()
    {
        auto name = identifier();

        if (name.compare("FLAGS") != 0)
        {
            throw assembler_error("Unknown section declaration", _scan_line);
            return;
        }

        // Loop until we get to another section
        while (true)
        {
            skip_whitepace();
            _scan_start = _scan_current;
            if (is_alpha(peek()))
            {
                auto name = identifier();
                if (_flags.count(name))
                {
                    throw assembler_error("Duplicate flag definition", _scan_line);
                }

                _flags[name] = static_cast<int16_t>(_flags.size());
            }
            else if (is_at_end() || peek() == '.')
            {
                return;
            }
            else
            {
                throw assembler_error("Invalid character", _scan_line);
            }

        }
    }

    void assembler::parse_constants()
    {
        auto name = identifier();

        if (name.compare("CONST") != 0)
        {
            throw assembler_error("Unknown section declaration", _scan_line);
            return;
        }

        // Loop until we get to another section
        while (true)
        {
            skip_whitepace();
            _scan_start = _scan_current;
            if (is_alpha(peek()))
            {
                auto name = identifier();
                if (_constants.count(name))
                {
                    throw assembler_error("Duplicate const definition", _scan_line);
                }

                skip_whitepace();
                auto value = number();
                _constants.insert({ name, {static_cast<int16_t>(_constants.size()), value} });
            }
            else if (is_at_end() || peek() == '.')
            {
                return;
            }
            else
            {
                throw assembler_error("Invalid character", _scan_line);
            }

        }
    }

    void assembler::parse_text()
    {
        auto name = identifier();

        if (name.compare("TEXT") != 0)
        {
            throw assembler_error("Unknown section declaration", _scan_line);
            return;
        }

        // Loop until we get to another section
        while (true)
        {
            skip_whitepace();
            _scan_start = _scan_current;
            if (is_alpha(peek()))
            {
                auto name = identifier();
                if (_strings.count(name))
                {
                    throw assembler_error("Duplicate text definition", _scan_line);
                }

                skip_whitepace();
                auto value = string_literal();
                _strings.insert({ name, {static_cast<int16_t>(_strings.size()), value} });
            }
            else if (is_at_end() || peek() == '.')
            {
                return;
            }
            else
            {
                throw assembler_error("Invalid character", _scan_line);
            }
        }
    }

    void assembler::parse_script()
    {
        auto name = identifier();

        if (name.compare("SCRIPT") != 0)
        {
            throw assembler_error("Unknown section declaration", _scan_line);
            return;
        }

        // Get the name
        skip_whitepace();
        auto this_script = script{ static_cast<int16_t>(_scripts.size()), static_cast<int16_t>(_bytecode.size()), 0 };
        auto script_name = identifier();

        if (_scripts.count(script_name))
        {
            throw assembler_error("A script with this name already exists: " + script_name, _scan_line);
        }

        // Loop until we get to another section
        while (true)
        {
            skip_whitepace();
            _scan_start = _scan_current;
            if (is_alpha(peek()))
            {
                // Get the opcode
                auto name = identifier();
                if (!_opcodes.count(name))
                {
                    throw assembler_error("Unknown opcode: " + name, _scan_line);
                }

                _bytecode.push_back(_opcodes[name].opcode);

                // Look for each param
                skip_whitepace();
                for (auto type : _opcodes[name].params)
                {
                    skip_whitepace();
                    switch (type)
                    {
                    case ParamType::CONST:
                    {
                        auto flag = get_param('#');

                        if (!_constants.count(flag))
                        {
                            throw assembler_error("Undefined constant: #" + flag, _scan_line);
                        }

                        _bytecode.push_back(_constants[flag].value);
                        break;
                    }

                    case ParamType::FLAG:
                    {
                        auto flag = get_param('!');

                        if (!_flags.count(flag))
                        {
                            throw assembler_error("Undefined flag id: !" + flag, _scan_line);
                        }

                        _bytecode.push_back(_flags[flag]);
                        break;
                    }

                    case ParamType::LABEL:
                    {
                        auto label = get_param('@');

                        _placeholders.push_back({ _bytecode.size(), label });

                        // Place a temporary value
                        _bytecode.push_back(static_cast<int32_t>(0x7FFF));

                        break;
                    }

                    case ParamType::NUMBER:
                    {
                        _bytecode.push_back(number());
                        break;
                    }

                    case ParamType::TEXT:
                    {
                        auto string_id = get_param('$');

                        if (!_strings.count(string_id))
                        {
                            throw assembler_error("Undefined text id: $" + string_id, _scan_line);
                        }

                        _bytecode.push_back(_strings[string_id].id);

                        break;
                    }

                    case ParamType::BOOLEAN:
                    {
                        _bytecode.push_back(boolean() ? -1 : 0);
                        break;
                    }

                    default:
                        break;
                    }
                }
            }
            else if (peek() == '@')
            {
                advance();
                auto label = identifier();

                if (_labels.count(label))
                {
                    throw assembler_error("Duplicate label: @" + label, _scan_line);
                }

                _labels.insert({ label, to<int32_t>(_bytecode.size())});
            }
            else if (is_at_end() || peek() == '.')
            {
                this_script.length = static_cast<int16_t>(_bytecode.size() - this_script.start);
                _scripts.insert({ script_name, this_script });
                return;
            }
            else
            {
                throw assembler_error("Invalid character", _scan_line);
            }
        }
    }

    std::string assembler::identifier()
    {
        _scan_start = _scan_current;
        while (is_alphanumeric(peek()))
            advance();

        return _source.substr(_scan_start, _scan_current - _scan_start);
    }

    int16_t assembler::number()
    {
        _scan_start = _scan_current;
        while (is_digit(peek()))
            advance();

        auto val = _source.substr(_scan_start, _scan_current - _scan_start);
        return static_cast<int16_t>(std::stoi(val));
    }

    std::string assembler::string_literal()
    {
        _scan_start = _scan_current;
        if (peek() != '"')
        {
            throw assembler_error("Unexpected character, expecting a '\"'.", _scan_line);
        }
        // Consume the opening quote
        advance();

        while (peek() != '"' && peek() != '\n' && !is_at_end())
        {
            advance();
        }

        if (peek() == '\n' || is_at_end())
        {
            throw assembler_error("Unterminated string.", _scan_line);
        }

        // Consume the closing quote
        advance();

        // Skip the quotes
        return _source.substr(_scan_start + 1, _scan_current - _scan_start - 2);
    }

    bool assembler::boolean()
    {
        if (check_word(0, 4, "TRUE"))
            return true;
        if (check_word(0, 4, "FALSE"))
            return false;

        throw assembler_error("Not TRUE or FALSE.", _scan_line);
    }

    std::string assembler::get_param(char prefix)
    {
        if (peek() == prefix)
        {
            advance();
            return identifier();
        }
        else
        {
            auto message = std::string{ "Missing id, did you forget the " };
            message.push_back(prefix);
            throw assembler_error(message, _scan_line);
        }
    }

    void assembler::print_stats()
    {
        std::cout << "Number of flags tracked: " << _flags.size() << std::endl;
        std::cout << "Number of constants:     " << _constants.size() << std::endl;
        std::cout << "Number of strings:       " << _strings.size() << std::endl;
        std::cout << "Number of scripts:       " << _scripts.size() << std::endl;

        for (auto const& kvp : _scripts)
        {
            std::cout << "  "
                << std::setw(30) << kvp.first
                << "  ID:"
                << std::setw(5) << kvp.second.id
                << "  Offset:"
                << std::setw(5) << kvp.second.start
                << "  Size"
                << std::setw(5) << kvp.second.length
                << std::endl;
        }
    }

    void assembler::save_files(std::filesystem::path const& path)
    {
        // Save bytecode
        std::filesystem::path bytecode_filename{ path };
        bytecode_filename.replace_extension(".nsx");

        auto bytecode_file = std::ofstream(bytecode_filename, std::ios::binary);
        // header
        bytecode_file.write("NS01", 4);
        auto num_scripts = to_big_endian(static_cast<int16_t>(_scripts.size()));
        bytecode_file.write((char*)&num_scripts, 2);

        for (auto kvp : _scripts)
        {
            auto start = to_big_endian(kvp.second.start);
            auto length = to_big_endian(kvp.second.length);
            bytecode_file.write((char*)&start, 2);
            bytecode_file.write((char*)&length, 2);
        }

        for (auto word : _bytecode)
        {
            auto transformed = to_big_endian(word);
            bytecode_file.write((char*)&transformed, 2);
        }

        bytecode_file.close();

        // Save the strings
        std::filesystem::path strings_filename{ path };
        strings_filename.replace_extension(".ns.txt");

        auto strings_file = std::ofstream(strings_filename);
        for (auto const& text : _strings)
        {
            strings_file << text.second.value;
            strings_file << '\n';
        }

        strings_file.close();

        // Save the string to a bb2 file
        std::filesystem::path strings_bb2_filename{ path };
        strings_bb2_filename.remove_filename() /= "NN_string_table_asc.bb2";

        auto strings_bb2_file = std::ofstream(strings_bb2_filename);
        strings_bb2_file
            << "; Autogenerated from string resources in " << path.filename() << "\n"
            << "; Do not modify." << "\n"
            << "Dim NN_string_table$(" << _strings.size() << ")\n";

        for (auto const& text : _strings)
        {
            strings_bb2_file
                << "NN_string_table$(" << text.second.id << ") = " << text.second.value << "\n";
        }

        strings_bb2_file.close();

        // Write out manifest csv
        std::filesystem::path manifest_filename{ path };
        manifest_filename.replace_extension(".manifest.csv");

        auto manifest_file = std::ofstream(manifest_filename);
        manifest_file << "Script Name,ID,Start,Length\n";

        for (auto const& kvp : _scripts)
        {
            manifest_file
                << kvp.first << ","
                << kvp.second.id << ","
                << kvp.second.start << ","
                << kvp.second.length << "\n";
        }

        manifest_file.close();
    }
}