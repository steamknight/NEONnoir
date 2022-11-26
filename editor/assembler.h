#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <filesystem>
#include <optional>
#include "types.h"

namespace NEONnoir
{
    constexpr i16 to_big_endian(i16 value)
    {
        return ((value & 0x00FF) << 8) | ((value & 0xFF00) >> 8);
    }

    class assembler_error : public std::runtime_error
    {
    public:
        assembler_error(std::string const& error, size_t line_number) :
            runtime_error(error + std::string(" - Line: ") + std::to_string(line_number)), line_number(line_number) {}
        size_t line_number;
    };


    template <typename Value>
    struct definition
    {
        i16 id = 0;
        Value value = 0;
    };

    using numeric_constant = definition<i16>;
    using string_constant = definition<std::string>;

    template <typename T>
    using named_values = std::unordered_map<std::string, T>;

    struct value_placeholder
    {
        size_t index = 0;
        std::string name;
    };

    enum class ParamType
    {
        FLAG,
        CONST,
        TEXT,
        LABEL,
        NUMBER,
        BOOLEAN
    };

    struct opcode_spec
    {
        i32 opcode = 0;
        std::vector<ParamType> params;
    };

    struct script
    {
        i16 id = 0;
        i16 start = 0;
        i16 length = 0;
    };

    using bytecode = std::vector<i16>;

    struct assembler_result
    {
        bytecode bytecode;
        named_values<script> scripts_meta;
        named_values<string_constant> strings;
        named_values<i16> flags;
        named_values<numeric_constant> constants;
    };

    class assembler
    {
    public:
        assembler(std::string const& source);

        assembler_result assemble();

        void print_stats();

        void save_files(std::filesystem::path const& path);

    private:
        inline bool is_at_end() { return _source[_scan_current] == '\0'; }
        inline bool is_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
        inline bool is_digit(char c) { return c >= '0' && c <= '9'; }
        inline bool is_alphanumeric(char c) { return is_alpha(c) || is_digit(c); }

        inline char advance() { return _source[_scan_current++]; }
        inline char peek() { return _source[_scan_current]; }
        void skip_whitepace();
        bool check_word(size_t token_start, size_t token_length, const char* rest);

        void parse_flags();
        void parse_constants();
        void parse_text();
        void parse_script();

        std::string identifier();
        i16 number();
        bool boolean();
        std::string string_literal();
        std::string get_param(char prefix);

        void parse_assembly();
        void update_jump_references();

    private:
        named_values<i16> _flags;
        named_values<numeric_constant> _constants;
        named_values<string_constant> _strings;
        named_values<opcode_spec> _opcodes;
        named_values<i16> _labels;
        named_values<script> _scripts;

        std::vector<value_placeholder> _placeholders;
        bytecode _bytecode;

        std::string _source;

        size_t _scan_start = 0;
        size_t _scan_current = 0;
        size_t _scan_line = 1;
    };
}
