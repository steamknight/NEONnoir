#pragma once
#include <filesystem>
#include <memory>
#include <vector>

#include "game_data.h"
#include "assembler.h"
#include "utils.h"

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
        uint16_t name_id{ 0xFFFF };

        uint16_t first_bg_id{ 0xFFFF };
        uint16_t last_bg_id{ 0xFFFF };

        uint16_t first_scene_id{ 0xFFFF };
        uint16_t last_scene_id{ 0xFFFF };

        uint16_t first_shape_id{ 0xFFFF };
        uint16_t last_shape_id{ 0xFFFF };
        uint16_t shapes_file{ 0xFFFF };
    };

    constexpr char scenes_header[4] = { 'S', 'C', 'N', 'S' };
    struct neon_scenes_header
    {
        char name[4] = { 'S', 'C', 'N', 'S' };
        uint16_t scene_count;
    };

    struct neon_scene
    {
        uint16_t name_id{ 0xFFFF };
        uint16_t first_desc_id{ 0xFFFF };
        uint16_t last_desc_id{ 0xFFFF };
        uint16_t on_enter{ 0xFFFF };
        uint16_t on_exit{ 0xFFFF };
        uint16_t background_id{ 0xFFFF };

        uint16_t first_region_id{ 0xFFFF };
        uint16_t last_region_id{ 0xFFFF };

        uint16_t music_id{ 0xFFFF };
    };

    struct neon_regions_header
    {
        char name[4] = { 'R', 'G', 'N', 'S' };
        uint16_t region_count{ 0xFFFF };
    };

    constexpr char regions_header[4] = { 'R', 'G', 'N', 'S' };
    struct neon_region
    {
        uint16_t x1, y1;
        uint16_t x2, y2;
        uint16_t pointer_id{ 0xFFFF };
        uint16_t description_id{ 0xFFFF };
        uint16_t script_offset{ 0xFFFF };
    };

    struct neon_dialogues_header
    {
        char name[4] = { 'D', 'L', 'G', 'S' };
        uint16_t dialogue_count;
    };

    constexpr char dialogues_header[4] = {'D', 'L', 'G', 'S'};

    struct neon_dialogue
    {
        uint16_t first_page_id{ 0xFFFF };
        uint16_t page_count{ 0xFFFF };
        uint16_t speaker_name{ 0xFFFF };
        uint16_t speaker_image{ 0xFFFF };
    };

    constexpr char pages_header [4] = { 'P', 'A', 'G', 'E' };
    struct neon_pages_header
    {
        char name[4] = { 'P', 'A', 'G', 'E' };
        uint16_t page_count;
    };

    struct neon_page
    {
        uint16_t text_id{ 0xFFFF };
        uint16_t set_flag{ 0xFFFF };
        uint16_t clear_flag{ 0xFFFF };
        uint16_t check_flag{ 0xFFFF };
        uint16_t page_id{ 0xFFFF };
        uint16_t first_choice_id{ 0xFFFF };
        uint16_t choice_count{ 0xFFFF };
        char enabled{ -1 };
        char self_disable{ 0 };
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
        uint16_t set_flag{ 0xFFFF };
        uint16_t clear_flag{ 0xFFFF };
        uint16_t check_flag{ 0xFFFF };
        uint16_t page_id{ 0xFFFF };
        uint16_t script_offset{ 0xFFFF };
        char enabled{ -1 };
        char self_disable{ 0 };
    };

    constexpr char string_header[4] = { 'S', 'T', 'R', 'G' };
    struct neon_string_table
    {
        uint16_t string_count;
    };

    constexpr char bytecode_header[4] = { 'B', 'Y', 'T', 'E' };

    constexpr char words_header[4] = { 'W', 'O', 'R', 'D' };
    struct neon_word
    {
        uint16_t start_idx{0xFFFF};
        uint16_t end_idx{0xFFFF};
    };

    struct neon_word_list
    {
        uint16_t word_count;
        std::vector<neon_word> words;
    };

    constexpr char shapes_header[4] = { 'S', 'H', 'P', 'E' };
    struct neon_shape
    {
        uint16_t shape_id{ 0 };
        uint16_t palette_id{ 0xFFFF };
    };

    constexpr char palettes_header[4] = { 'P', 'A', 'L', 'S' };
    constexpr char ui_palette_header[4] = { 'P', 'A', 'L', 'U' };

    struct loc_header
    {
        char const magic[4] = { 'N', 'O', 'I', 'R' };
        uint16_t version{ 0 };
        uint16_t language{ 0 };
    };

        
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
        std::vector<neon_word_list> words_table;
        std::vector<neon_shape> shapes;
        std::vector<MPG::color_palette> palettes;
    };

    void serialize_to_neon_pak(std::filesystem::path file_path, std::shared_ptr<game_data> const& data, assembler_result const& result);
    void serialize_neon_loc(std::filesystem::path file_path, std::vector<std::string> const& string_table, std::vector<neon_word_list> const& words_table);

}
