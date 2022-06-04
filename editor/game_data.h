#pragma once
#include <vector>
#include <string>
#include "glfw_utils.h"

namespace NEONnoir
{
    enum class game_data_chunk_type : uint32_t
    {
        location = 1,
        bytecode,
        strings,
        items
    };

#pragma pack(push, 1)
    struct game_data_header
    {
        uint8_t magic[4]{ 'N', 'E', 'O', 'N' };
        uint16_t major_version{ 1 };
        uint16_t minor_version{ 0 };

        uint16_t location_count{ 0 };
        uint16_t item_count{ 0 };
        uint16_t bitecode_size{ 0 };
        uint16_t string_count{ 0 };
       
        uint32_t locations_offset{ 0 };
        uint32_t items_offset{ 0 };
        uint32_t bytecode_offset{ 0 };
        uint32_t strings_offset{ 0 };
    };
#pragma pack(pop)

    struct game_data_region
    {
        uint16_t x, y;
        uint16_t width, height;
        uint16_t shape_id;
        std::string description;
        std::string script;
    };

    struct game_data_scene
    {
        std::string name;
        uint16_t image_id;
        uint16_t view_x, view_y;
        uint16_t view_width, view_height;
        uint16_t offset_x, offset_y;
        std::vector<game_data_region> regions;
    };

    struct game_data_location
    {
        std::string name;
        std::vector<std::string> backgrounds;
        std::vector<GLtexture> background_textures;
        std::vector<game_data_scene> scenes;
    };

    class game_data
    {
    public:
        std::vector<game_data_location> locations;
    private:
    };
}

