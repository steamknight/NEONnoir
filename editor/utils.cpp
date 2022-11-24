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

    std::vector<rgb_color> read_bmp_palette(std::string_view const& path)
    {
        auto source = std::ifstream{ path.data(), std::ios::binary };
        if (!source)
        {
            // TODO Show error
        }

        auto header = bmp_header{};
        auto info = bmp_info_header{};
        source.read(reinterpret_cast<char*>(&header), sizeof(bmp_header));
        source.read(reinterpret_cast<char*>(&info), sizeof(bmp_info_header));

        if (header.format != 0x4D42) // "BM"
        {
            //throw std::runtime_error("Palette donor file is not a BMP file.");
        }
        if (info.bits_per_pixel != 8)
        {
            //throw std::runtime_error("Palette donor file is not an 8bit indexed BMP.");
        }

        // How many colors are there in the palette?
        auto color_count = info.palette_color_count > 0 ? info.palette_color_count : 256;

        auto palette = std::vector<rgb_color>{};
        palette.resize(color_count);
        source.read(reinterpret_cast<char*>(&palette[0]), sizeof(rgb_color) * color_count);

        return palette;
    }

    void write(std::ofstream& stream, uint16_t value)
    {
        auto data = force_to<char*>(&value);
        stream.write(&data[1], 1);
        stream.write(&data[0], 1);
    }

    void write(std::ofstream& stream, uint32_t value)
    {
        auto data = force_to<char*>(&value);
        stream.write(&data[3], 1);
        stream.write(&data[2], 1);
        stream.write(&data[1], 1);
        stream.write(&data[0], 1);
    }
}

#pragma warning(pop)