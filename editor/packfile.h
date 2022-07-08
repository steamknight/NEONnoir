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

    constexpr char locations_header[4] = { 'L', 'O', 'C', 'S' };
    struct neon_locations_header
    {
        char name[4] = { 'L', 'O', 'C', 'S' };
        uint16_t dialogue_count;
    };

    struct neon_location
    {
        uint16_t name_id;

        uint16_t first_bg_id;
        uint16_t last_bg_id;

        uint16_t first_scene_id;
        uint16_t last_scene_id;
    };

    constexpr char scenes_header[4] = { 'S', 'C', 'N', 'S' };
    struct neon_scenes_header
    {
        char name[4] = { 'S', 'C', 'N', 'S' };
        uint16_t scene_count;
    };

    struct neon_scene
    {
        uint16_t name_id;
        uint16_t background_id;

        uint16_t first_region_id;
        uint16_t last_region_id;
    };

    struct neon_regions_header
    {
        char name[4] = { 'R', 'G', 'N', 'S' };
        uint16_t region_count;
    };

    constexpr char regions_header[4] = { 'R', 'G', 'N', 'S' };
    struct neon_region
    {
        uint16_t x1, y1;
        uint16_t x2, y2;
        uint16_t pointer_id;
        uint16_t description_id;
        uint16_t script_offset;
    };

    struct neon_dialogues_header
    {
        char name[4] = { 'D', 'L', 'G', 'S' };
        uint16_t dialogue_count;
    };

    constexpr char dialogues_header[4] = {'D', 'L', 'G', 'S'};

    struct neon_dialogue
    {
        uint16_t first_page_id;
        uint16_t page_count;
        uint16_t speaker_name;
        uint16_t speaker_image;
    };

    constexpr char pages_header [4] = { 'P', 'A', 'G', 'E' };
    struct neon_pages_header
    {
        char name[4] = { 'P', 'A', 'G', 'E' };
        uint16_t page_count;
    };

    struct neon_page
    {
        uint16_t text_id;
        uint16_t page_id;
        uint16_t first_choice_id;
        uint16_t choice_count;
        char enabled;
        char padding;
    };

    constexpr char choices_header[4] = { 'C', 'H', 'C', 'E' };
    struct neon_choices_header
    {
        char name[4] = { 'C', 'H', 'C', 'E' };
        uint16_t choice_count;
    };

    struct neon_choice
    {
        uint16_t text_id;
        uint16_t flag;
        uint16_t page_id;
        uint16_t script_offset;
        char enabled;
        char padding;
    };

    constexpr char string_header[4] = { 'S', 'T', 'R', 'G' };
    struct string_table
    {
        uint16_t string_count;
    };

    constexpr char bytecode_header[4] = { 'B', 'Y', 'T', 'E' };
#pragma pack(pop)

    struct neon_packfile
    {
        neon_header header;
        std::vector<neon_location> locations;
        std::vector<neon_scene> scenes;
        std::vector<neon_region> regions;
        std::vector<neon_dialogue> dialogues;
        std::vector<neon_page> pages;
        std::vector<neon_choice> choices;
        std::vector<std::string> string_table;
    };

    void serialize_to_neon_pak(std::filesystem::path file_path, std::shared_ptr<game_data> const& data, assembler_result const& result);

}
