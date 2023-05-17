#pragma once
#include <concepts>
#include <filesystem>
#include <format>
#include <string>
#include <unordered_map>
#include <vector>
#include "imgui.h"
#include "types.h"

namespace NEONnoir
{
    struct word
    {
        i32 begin_idx{ 0 };
        i32 end_idx{ -1 };
    };

    struct string_entry
    {
        std::string id;
        std::string value;
        std::string description;
        std::string value_latin1;
        std::vector<word> words_latin1;
    };

    class string_table
    {
    private:
        using string_entries = std::vector<string_entry>;
        using id_index_map = std::unordered_map<std::string, string_entries::size_type>;

    public:
        std::string create_string_entry(std::string const& value);
        [[nodiscard]] std::string& get_string(std::string const& string_id);
        [[nodiscard]] std::string& get_string_latin1(std::string const& string_id);
        [[nodiscard]] std::vector<word> const& get_words_latin1(std::string const& string_id);
        bool constains_string(std::string const& string_id);
        size_t get_string_index(std::string const& string_id);
        void remove_string(std::string const& string_id);
        void change_string_id(std::string const& old_id, std::string const& new_id);
        void move_up(std::string const& string_id);
        void move_down(std::string const& string_id);

        void serialize(std::filesystem::path const& file_path) const;
        void deserialize(std::filesystem::path const& file_path);

        void generate_po_file(std::filesystem::path const& file_path) const;

        static std::vector<word> calculate_words(std::string const& text);

    public:
        string_entries entries{};

    private:
        std::string::size_type const _string_id_length = 16;
        id_index_map _id_index_map{};
        ImVec2 _char_size;

    private:
        void ensure_valid_id(std::string const& string_id) const;

    };

    [[nodiscard]] std::string generate_string_id(std::string::size_type length) noexcept;
}

