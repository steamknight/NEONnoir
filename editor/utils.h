#pragma once
#include <optional>
#include <string_view>
#include <format>

#include "types.h"

namespace NEONnoir
{
    std::optional<std::string_view> open_file_dialog(std::string_view const& filter);
    std::optional<std::string_view> save_file_dialog(std::string_view const& filter);

    void write(std::ofstream& stream, u16 value);
    void write(std::ofstream& stream, u32 value);

    std::string UTF8toISO8859_1(const char* in);

#define make_id(label, value) std::format((label), (size_t)&value).c_str()

}