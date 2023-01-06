#include <json.hpp>

#include <fstream>
#include <random>
#include "string_table.h"

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;
namespace fs = std::filesystem;

namespace NEONnoir
{
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
        string_entry, id, value, description
    )

    std::string string_table::create_string_entry(std::string const& value)
    {
        auto id = std::string{};
        do
        {
            id = generate_string_id(_string_id_length);
        } while (id_index_map.count(id) != 0);

        id_index_map[id] = string_entries.size();
        string_entries.push_back({ id, value, "" });

        return std::string{ id };
    }

    std::string& string_table::get_string(std::string const& string_id)
    {
        ensure_valid_id(string_id);
        auto index = id_index_map.at(string_id);

        return string_entries[index].value;
    }

    void string_table::remove_string(std::string const& string_id)
    {
        ensure_valid_id(string_id);
        auto index = id_index_map.at(string_id);

        id_index_map.erase(string_id);
        string_entries.erase(string_entries.begin() + index);
    }

    void string_table::move_up(std::string const& string_id)
    {
        ensure_valid_id(string_id);
        auto index = id_index_map.at(string_id);

        if (index != 0)
        {
            std::swap(string_entries[index], string_entries[index - 1]);
        }
    }

    void string_table::move_down(std::string const& string_id)
    {
        ensure_valid_id(string_id);
        auto index = id_index_map.at(string_id);

        if (index != string_entries.size() - 1)
        {
            std::swap(string_entries[index], string_entries[index + 1]);
        }
    }

    void string_table::serialize(fs::path const& file_path) const
    {
        auto savefile = std::ofstream{ file_path, std::ios::trunc };
        if (savefile)
        {
            auto root = ordered_json{};

            for (auto const& se : string_entries)
            {
                auto entry = ordered_json{};
                entry["id"] = se.id;
                entry["value"] = se.value;
                entry["description"] = se.description;

                root.push_back(entry);
            }

            savefile << root.dump(2);
            savefile.close();
        }
    }

    void string_table::deserialize(fs::path const& file_path)
    {
        if (!fs::exists(file_path))
        {
            throw std::runtime_error{ std::format("File '{}' does not exist.", file_path.string())};
        }

        auto savefile = std::ifstream{ file_path };
        if (savefile)
        {
            auto buffer = std::stringstream{};
            buffer << savefile.rdbuf();

            auto j = json::parse(buffer.str());
            string_entries = j.get<std::vector<string_entry>>();

            size_t count = 0;
            for (auto const& se : string_entries)
            {
                id_index_map[se.id] = count;
                count++;
            }

            savefile.close();
        }
    }

    void string_table::ensure_valid_id(std::string const& string_id) const
    {
        if (id_index_map.count(string_id) == 0)
        {
            throw std::runtime_error{ std::format("Could not find string id: '{}'", string_id) };
        }

        auto index = id_index_map.at(string_id);
        if (string_entries.size() <= index)
        {
            throw std::runtime_error{ std::format("String id '{}' with index {} is out bounds", string_id, index) };
        }
    }

    // https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
    std::string generate_string_id(std::string::size_type length) noexcept
    {
        static auto characters = std::string_view{ "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz" };
        thread_local std::mt19937 rng{ std::random_device{}() };
        thread_local std::uniform_int_distribution<std::string::size_type> dist{ 0, characters.size() - 2 };

        auto string_id = std::string{};
        while (length--)
        {
            string_id += characters[dist(rng)];
        }

        return string_id;
    }
}
