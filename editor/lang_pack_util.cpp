#include <fstream>
#include <format>
#include <vector>

#include "packfile.h"
#include "lang_pack_util.h"

namespace NEONnoir
{
    void lang_pack_to_csv(std::filesystem::path const& pack_path)
    {
        // Load the pack file
        auto pack_file = std::ifstream{ pack_path, std::ios::binary };
        if (!pack_file)
        {
            throw std::runtime_error{ std::format("Could not open pack file: '{}'", pack_path.string()) };
        }


    }

    void csv_to_lang_pack(std::filesystem::path const& /*csv_path*/)
    {

    }

}