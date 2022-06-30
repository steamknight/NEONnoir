#pragma once
#include <filesystem>
#include <memory>
#include <vector>

#include "game_data.h"
#include "assembler.h"

namespace NEONnoir
{
    class packer_error : public std::runtime_error
    {
    public:
        packer_error(std::string const& error) :
            runtime_error(error) {}
    };

#pragma pack(push, 1)
    struct neon_header
    {
        uint8_t magic[4]{ 'N', 'E', 'O', 'N' };
        uint16_t major_version{ 1 };
        uint16_t minor_version{ 0 };
    };

    struct neon_string
    {
        uint16_t size{ 0 };

        // Array of chars
    };

    struct neon_location
    {
        uint16_t name_id;

        uint16_t first_bg_id;
        uint16_t last_bg_id;

        uint16_t first_scene_id;
        uint16_t last_scene_id;
    };

    struct neon_scene
    {
        uint16_t name_id;
        uint16_t background_id;

        uint16_t first_region_id;
        uint16_t last_region_id;
    };

    struct neon_region
    {
        uint16_t x1, y1;
        uint16_t x2, y2;
        uint16_t pointer_id;
        uint16_t description_id;
        uint16_t script_offset;
    };

    struct string_table
    {
        uint16_t string_count;
    };
#pragma pack(pop)

    struct neon_packfile
    {
        neon_header header;
        std::vector<neon_location> locations;
        std::vector<neon_scene> scenes;
        std::vector<neon_region> regions;
        std::vector<std::string> string_table;
    };

    void serialize_to_neon_pak(std::filesystem::path file_path, std::shared_ptr<game_data> const& data, assembler_result const& result);

}
