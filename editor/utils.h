#pragma once
#include <optional>
#include <string_view>
#include <format>
#include "bmp_header.h"


namespace NEONnoir
{
    std::optional<std::string_view> open_file_dialog(std::string_view const& filter);
    std::optional<std::string_view> save_file_dialog(std::string_view const& filter);

    template<char... T>
    size_t operator"" _z()
    {
        return static_cast<size_t>(T);
    }

    template<typename T, typename U>
    auto to(U value) -> T
    {
        return static_cast<T>(value);
    }


    std::vector<rgb_color> read_bmp_palette(std::string_view const& path);


#define make_id(label, value) std::format((label), (size_t)&value).c_str()

}