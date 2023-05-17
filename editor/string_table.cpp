#include <json.hpp>

#include <fstream>
#include <random>
#include <set>
#include "string_table.h"

#include "utils.h"

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
        } while (_id_index_map.count(id) != 0);

        _id_index_map[id] = entries.size();
        entries.push_back({ id, value, "" });

        return std::string{ id };
    }

    bool string_table::constains_string(std::string const& string_id)
    {
        return _id_index_map.count(string_id) > 0;
    }

    size_t string_table::get_string_index(std::string const& string_id)
    {
        ensure_valid_id(string_id);
        return _id_index_map.at(string_id);
    }

    std::string& string_table::get_string(std::string const& string_id)
    {
        ensure_valid_id(string_id);
        auto index = _id_index_map.at(string_id);

        return entries[index].value;
    }

    std::string& string_table::get_string_latin1(std::string const& string_id)
    {
        ensure_valid_id(string_id);
        auto index = _id_index_map.at(string_id);

        entries[index].value_latin1 = UTF8toISO8859_1(entries[index].value.c_str());
        return entries[index].value_latin1;
    }

    std::vector<word> const& string_table::get_words_latin1(std::string const& string_id)
    {
        ensure_valid_id(string_id);
        auto index = _id_index_map.at(string_id);
        
        entries[index].words_latin1 = calculate_words(entries[index].value_latin1);
        return entries[index].words_latin1;
    }

    void string_table::remove_string(std::string const& string_id)
    {
        ensure_valid_id(string_id);
        auto index = _id_index_map.at(string_id);

        _id_index_map.erase(string_id);
        entries.erase(entries.begin() + index);

        std::for_each(entries.begin() + index, entries.end(),
            [&](string_entry& entry)
            {
                _id_index_map[entry.id]--;
            });
    }

    void string_table::change_string_id(std::string const& old_id, std::string const& new_id)
    {
        ensure_valid_id(old_id);

        auto index = _id_index_map.at(old_id);
        _id_index_map[new_id] = index;
        _id_index_map.erase(old_id);

        entries[index].id = new_id;
    }

    void string_table::move_up(std::string const& string_id)
    {
        ensure_valid_id(string_id);
        auto index = _id_index_map.at(string_id);

        if (index != 0)
        {
            std::swap(entries[index], entries[index - 1]);
        }
    }

    void string_table::move_down(std::string const& string_id)
    {
        ensure_valid_id(string_id);
        auto index = _id_index_map.at(string_id);

        if (index != entries.size() - 1)
        {
            std::swap(entries[index], entries[index + 1]);
        }
    }

    void string_table::serialize(fs::path const& file_path) const
    {
        auto savefile = std::ofstream{ file_path, std::ios::trunc };
        if (savefile)
        {
            auto root = ordered_json{};

            for (auto const& se : entries)
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
            entries = j.get<std::vector<string_entry>>();

            size_t count = 0;
            for (auto const& se : entries)
            {
                _id_index_map[se.id] = count;
                count++;
            }

            savefile.close();
        }
    }

    std::string escape(const char* src, const std::set<char> escapee, const char marker)
    {
        std::string r;
        while (char c = *src++)
        {
            if (escapee.find(c) != escapee.end())
                r += marker;
            r += c; // to get the desired behavior, replace this line with: r += c == '\n' ? 'n' : c;
        }
        return r;
    }

    void string_table::generate_po_file(std::filesystem::path const& file_path) const
    {
        auto savefile = std::ofstream{ file_path, std::ios::trunc };
        if (savefile)
        {
            auto write_multiline = [&savefile](std::string const& text)
            {
                auto escaped = escape(text.c_str(), { '\"'}, '\\');

                auto stream = std::stringstream{ escaped };
                auto line = std::string{};
                while (std::getline(stream, line, '\n'))
                {
                    savefile << "\"" << line << "\"" << std::endl;
                }

                if (text.empty()) savefile << "\"\"" << std::endl;
            };

            auto unique = std::set<std::string>{};
            auto count = 0;

            for (auto const& se : entries)
            {
                savefile << "#: " << se.id << std::endl;

                savefile << "msgctxt ";
                if (unique.contains(se.description))
                {
                    write_multiline(std::format("{}{}", count, se.description));
                    count++;
                }
                else
                {
                    write_multiline(se.description);
                }
                unique.insert(se.description);

                savefile << "msgid ";
                write_multiline(se.value);

                savefile << "msgstr \"\"" << std::endl;

                savefile << std::endl;
            }

            savefile.close();
        }
    }

    void string_table::ensure_valid_id(std::string const& string_id) const
    {
        if (_id_index_map.count(string_id) == 0)
        {
            throw std::runtime_error{ std::format("Could not find string id: '{}'", string_id) };
        }

        auto index = _id_index_map.at(string_id);
        if (entries.size() <= index)
        {
            throw std::runtime_error{ std::format("String id '{}' with index {} is out bounds", string_id, index) };
        }
    }

    std::vector<word> string_table::calculate_words(std::string const& text)
    {
        auto word_list = std::vector<word>{};

        i32 begin_idx = 0;
        i32 current = 0;

        word_list.clear();
        while (begin_idx < text.size())
        {
            current = begin_idx;
            while (current < text.size() && text[current] != ' ' && text[current] != '\n')
            {
                current++;
            }

            word_list.push_back({ begin_idx, current - 1 });

            if (text[current] == '\n')
            {
                word_list.push_back({});
            }

            begin_idx = current + 1;
        }

        return word_list;
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
