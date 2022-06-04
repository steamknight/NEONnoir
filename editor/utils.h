#pragma once
#include <optional>
#include <string_view>

namespace NEONnoir
{
    std::optional<std::string_view> open_file_dialog(std::string_view const& filter);
    std::optional<std::string_view> save_file_dialog(std::string_view const& filter);
}