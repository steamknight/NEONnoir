#pragma warning(push)
#pragma warning(disable:26812)

#include <nfd.h>
#include <fstream>
#include <vector>
#include "utils.h"

namespace NEONnoir
{
    std::optional<std::string_view> open_file_dialog(std::string_view const& filter)
    {
        nfdchar_t* path = nullptr;
        if (NFD_OpenDialog(filter.data(), nullptr, &path) == NFD_OKAY)
        {
            return std::string_view{ path };
        }

        return {};
    }

    std::optional<std::string_view> save_file_dialog(std::string_view const& filter)
    {
        nfdchar_t* path = nullptr;
        if (NFD_SaveDialog(filter.data(), nullptr, &path) == NFD_OKAY)
        {
            return std::string_view{ path };
        }

        return {};
    }

    void write(std::ofstream& stream, u16 value)
    {
        auto data = force_to<char*>(&value);
        stream.write(&data[1], 1);
        stream.write(&data[0], 1);
    }

    void write(std::ofstream& stream, u32 value)
    {
        auto data = force_to<char*>(&value);
        stream.write(&data[3], 1);
        stream.write(&data[2], 1);
        stream.write(&data[1], 1);
        stream.write(&data[0], 1);
    }
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

#pragma warning(pop)