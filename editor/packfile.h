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
        u8 magic[4]{ 'N', 'E', 'O', 'N' };
        u16 major_version{ 1 };
        u16 minor_version{ 0 };
    };

    struct neon_string
    {
        u16 size{ 0 };

        // Array of chars
    };

    constexpr char locations_header[4] = { 'L', 'O', 'C', 'S' };
    struct neon_locations_header
    {
        char name[4] = { 'L', 'O', 'C', 'S' };
        u16 dialogue_count;
    };

    struct neon_location
    {
        u16 name_id{ 0xFFFF };

        u16 first_bg_id{ 0xFFFF };
        u16 last_bg_id{ 0xFFFF };

        u16 first_scene_id{ 0xFFFF };
        u16 last_scene_id{ 0xFFFF };

        u16 first_shape_id{ 0xFFFF };
        u16 last_shape_id{ 0xFFFF };
        u16 shapes_file{ 0xFFFF };

        u16 speakers[8]{ 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF };
    };

    constexpr char scenes_header[4] = { 'S', 'C', 'N', 'S' };
    struct neon_scenes_header
    {
        char name[4] = { 'S', 'C', 'N', 'S' };
        u16 scene_count;
    };

    struct neon_scene
    {
        u16 name_id{ 0xFFFF };
        u16 first_desc_id{ 0xFFFF };
        u16 last_desc_id{ 0xFFFF };
        u16 on_enter{ 0xFFFF };
        u16 on_exit{ 0xFFFF };
        u16 background_id{ 0xFFFF };

        u16 first_region_id{ 0xFFFF };
        u16 last_region_id{ 0xFFFF };

        u16 music_id{ 0xFFFF };
    };

    struct neon_regions_header
    {
        char name[4] = { 'R', 'G', 'N', 'S' };
        u16 region_count{ 0xFFFF };
    };

    constexpr char regions_header[4] = { 'R', 'G', 'N', 'S' };
    struct neon_region
    {
        u16 x1, y1;
        u16 x2, y2;
        u16 pointer_id{ 0xFFFF };
        u16 description_id{ 0xFFFF };
        u16 goto_scene{ 0xFFFF };
        u16 script_offset{ 0xFFFF };
    };

    struct neon_dialogues_header
    {
        char name[4] = { 'D', 'L', 'G', 'S' };
        u16 dialogue_count;
    };

    constexpr char dialogues_header[4] = {'D', 'L', 'G', 'S'};

    struct neon_dialogue
    {
        u16 first_page_id{ 0xFFFF };
        u16 page_count{ 0xFFFF };
    };

    constexpr char pages_header [4] = { 'P', 'A', 'G', 'E' };
    struct neon_pages_header
    {
        char name[4] = { 'P', 'A', 'G', 'E' };
        u16 page_count;
    };

    struct neon_page
    {
        u16 speaker_id{ 0xFFFF };
        u16 text_id{ 0xFFFF };
        u16 set_flag{ 0xFFFF };
        u16 clear_flag{ 0xFFFF };
        u16 check_flag{ 0xFFFF };
        u16 page_id{ 0xFFFF };
        u16 first_choice_id{ 0xFFFF };
        u16 choice_count{ 0xFFFF };
        char enabled{ -1 };
        char self_disable{ 0 };
    };

    constexpr char choices_header[4] = { 'C', 'H', 'C', 'E' };
    struct neon_choices_header
    {
        char name[4] = { 'C', 'H', 'C', 'E' };
        u16 choice_count;
    };

    struct neon_choice
    {
        u16 text_id;
        u16 set_flag{ 0xFFFF };
        u16 clear_flag{ 0xFFFF };
        u16 check_flag{ 0xFFFF };
        u16 page_id{ 0xFFFF };
        u16 script_offset{ 0xFFFF };
        char enabled{ -1 };
        char self_disable{ 0 };
    };

    constexpr char string_header[4] = { 'S', 'T', 'R', 'G' };
    struct neon_string_table
    {
        u16 string_count;
    };

    constexpr char bytecode_header[4] = { 'B', 'Y', 'T', 'E' };

    constexpr char words_header[4] = { 'W', 'O', 'R', 'D' };
    struct neon_word
    {
        u16 start_idx{0xFFFF};
        u16 end_idx{0xFFFF};
    };

    struct neon_word_list
    {
        std::vector<neon_word> words;
        u16 word_count;
    };

    constexpr char shapes_header[4] = { 'S', 'H', 'P', 'E' };
    struct neon_shape
    {
        u16 shape_id{ 0 };
        u16 palette_id{ 0xFFFF };
    };

    constexpr char palettes_header[4] = { 'P', 'A', 'L', 'S' };
    constexpr char ui_palette_header[4] = { 'P', 'A', 'L', 'U' };

    constexpr char speakers_header[4] = { 'S', 'P', 'K', 'R' };

    struct loc_header
    {
        char const magic[4] = { 'N', 'O', 'I', 'R' };
        u16 version{ 0 };
        u16 language{ 0 };
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
        std::vector<MPG::blitz_shapes> speakers;
    };

    void serialize_to_neon_pak(std::filesystem::path file_path, std::shared_ptr<game_data> const& data, assembler_result const& result);
    void serialize_neon_loc(std::filesystem::path file_path, std::vector<std::string> const& string_table, std::vector<neon_word_list> const& words_table);

}
