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
}

#pragma warning(pop)