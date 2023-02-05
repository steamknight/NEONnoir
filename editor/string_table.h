#pragma once
#include <concepts>
#include <filesystem>
#include <format>
#include <string>
#include <unordered_map>
#include <vector>

namespace NEONnoir
{
    struct string_entry
    {
        std::string id;
        std::string value;
        std::string description;
    };

    class string_table
    {
    private:
        //using string_entries = std::vector<string_entry>;
        //using id_index_map = std::unordered_map<std::std::string, std::vector<string_entry>::size_type>;

    public:
        std::string create_string_entry(std::string const& value);
        [[nodiscard]] std::string& get_string(std::string const& string_id);
        void remove_string(std::string const& string_id);
        void change_string_id(std::string const& old_id, std::string const& new_id);
        void move_up(std::string const& string_id);
        void move_down(std::string const& string_id);

        void serialize(std::filesystem::path const& file_path) const;
        void deserialize(std::filesystem::path const& file_path);

    public:
        std::vector<string_entry> string_entries{};

    private:
        std::string::size_type const _string_id_length = 16;
        std::unordered_map<std::string, std::vector<string_entry>::size_type> id_index_map{};

    private:
        void ensure_valid_id(std::string const& string_id) const;

    };

    [[nodiscard]] std::string generate_string_id(std::string::size_type length) noexcept;
}

