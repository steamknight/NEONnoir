// NEONlangpack.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <filesystem>
#include <fstream>
#include <format>
#include "json.hpp"
#include "string_table.h"

namespace fs = std::filesystem;

using u16 = uint16_t;
using u32 = uint32_t;

template<typename T, typename U>
auto to(U value) -> T
{
    return static_cast<T>(value);
}

constexpr char magic[4] = { 'N', 'O', 'I', 'R' };
constexpr char string_header[4] = { 'S', 'T', 'R', 'G' };
constexpr char words_header[4] = { 'W', 'O', 'R', 'D' };
struct neon_word
{
    u16 start_idx{ 0xFFFF };
    u16 end_idx{ 0xFFFF };
};

struct neon_word_list
{
    std::vector<neon_word> words;
    u16 word_count;
};

int serialize_neon_loc(
    std::filesystem::path file_path, 
    std::vector<std::string> const& string_table, 
    std::vector<neon_word_list> const& words_table);

void write(std::ofstream& stream, u16 value);
void write(std::ofstream& stream, u32 value);

int show_usage()
{
    std::cout << "Usage:" << std::endl
        << "NEONlangpack <language_json> <language_code>" << std::endl
        << std::endl
        << "\tlanguage_json: the json file containing all the strings." << std::endl
        << "\tlanguage_code: a two letter language code. Currently, only en, it, and de are supported." << std::endl
        << std::endl;
    return 1;
}

int main(int argc, char** argv)
{
    std::cout << fs::current_path() << std::endl;

    if (argc != 3)
    {
        return show_usage();
    }

    if (!fs::exists(argv[1]))
    {
        std::cout << "ERROR: file \"" << argv[0] << "\" not found." << std::endl << std::endl;
        show_usage();

        return 2;
    }

    auto strings = NEONnoir::string_table{};
    strings.deserialize(argv[1]);

    // Gather the strings
    auto loc_strings = std::vector<std::string>{};
    for (auto const& entry : strings.entries)
    {
        loc_strings.push_back(strings.get_string_latin1(entry.id));
    }

    // Calculate the words
    std::vector<neon_word_list> words_table;
    for (auto const& text : loc_strings)
    {
        auto words = NEONnoir::string_table::calculate_words(text);

        auto word_list = neon_word_list{};
        word_list.word_count = to<u16>(words.size());
        word_list.words.reserve(word_list.word_count);

        std::ranges::transform(words, std::back_inserter(word_list.words), [](auto& word) -> neon_word { return { to<u16>(word.begin_idx), to<u16>(word.end_idx) }; });

        words_table.push_back(word_list);
    }

    return serialize_neon_loc(std::format({ "{}.noir" }, argv[2]), loc_strings, words_table);
}

int serialize_neon_loc(std::filesystem::path file_path, std::vector<std::string> const& string_table, std::vector<neon_word_list> const& words_table)
{
    auto locpack = std::ofstream{ file_path, std::ios::binary | std::ios::trunc };

    if (!locpack)
    {
        std::cout << std::format("Could not open/create '{}'", file_path.string()) << std::endl;
        return 3;
    }

    // Write the pack header
    u16 version = 1;                             // Eventually this will be updated
    u16 language = 1;                            // Eventually this will be a language code
    locpack.write(magic, 4);
    write(locpack, version);
    write(locpack, language);

    // Write the string table "header"
    locpack.write(string_header, 4);
    write(locpack, to<u32>(string_table.size()));

    auto string_size = 0u;
    for (auto const& entry : string_table)
    {
        string_size += to<u32>(entry.size());
        string_size += 4;
    }
    write(locpack, string_size);

    for (auto const& entry : string_table)
    {
        write(locpack, to<u32>(entry.size()));
        locpack.write(entry.data(), entry.size());
    }

    // Write words header
    locpack.write(words_header, 4);
    write(locpack, to<u32>(words_table.size()));

    auto words_size = 0u;
    for (auto const& entry : words_table)
    {
        words_size += 4; // size of word list
        words_size += to<u32>(entry.words.size() * sizeof(neon_word));
    }
    write(locpack, words_size);

    for (auto const& entry : words_table)
    {
        write(locpack, to<u32>(entry.words.size()));
        for (auto const& word : entry.words)
        {
            write(locpack, word.start_idx);
            write(locpack, word.end_idx);
        }
    }

    return 0;
}

void write(std::ofstream& stream, u16 value)
{
    auto data = reinterpret_cast<char*>(&value);
    stream.write(&data[1], 1);
    stream.write(&data[0], 1);
}

void write(std::ofstream& stream, u32 value)
{
    auto data = reinterpret_cast<char*>(&value);
    stream.write(&data[3], 1);
    stream.write(&data[2], 1);
    stream.write(&data[1], 1);
    stream.write(&data[0], 1);
}

namespace NEONnoir
{
    std::string UTF8toISO8859_1(const char* in) {
        // Based on https://stackoverflow.com/questions/53269432/convert-from-utf-8-to-iso8859-15-in-c
        std::string out;
        if (in == NULL)
            return out;

        unsigned int codepoint{ 0 };
        while (*in != 0) {
            unsigned char ch = static_cast<unsigned char>(*in);
            if (ch <= 0x7f)
                codepoint = ch;
            else if (ch <= 0xbf)
                codepoint = (codepoint << 6) | (ch & 0x3f);
            else if (ch <= 0xdf)
                codepoint = ch & 0x1f;
            else if (ch <= 0xef)
                codepoint = ch & 0x0f;
            else
                codepoint = ch & 0x07;
            ++in;
            if (((*in & 0xc0) != 0x80) && (codepoint <= 0x10ffff)) {
                if (codepoint <= 255) {
                    out.append(1, static_cast<char>(codepoint));
                }
                else {
                    out.append("?");
                }
            }
        }
        return out;
    }
}