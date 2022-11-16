#pragma once

#include <filesystem>

namespace NEONnoir
{
    void lang_pack_to_csv(std::filesystem::path const& pack_path);
    void csv_to_lang_pack(std::filesystem::path const& csv_path);
}