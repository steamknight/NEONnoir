#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include <unordered_map>
#include "glfw_utils.h"
#include "utils.h"
#include "simple_image.h"
#include "string_table.h"

namespace NEONnoir
{
    constexpr u16 NO_INDEX = 0xFFFF;

    enum class game_data_chunk_type : u32
    {
        location = 1,
        bytecode,
        strings,
        items
    };

    struct game_data_region
    {
        u16 x{ 0 }, y{ 0 };
        u16 width{ 0 }, height{ 0 };
        u16 shape_id{ 0 };
        u16 pointer_id{ 0 };
        u16 goto_scene{ NO_INDEX };
        std::string description_id;
        std::string script;
    };

    enum textbox_justification : u16
    {
        left,
        center,
        right
    };
    struct game_data_text_region
    {
        float x{ 0.f }, y{ 0.f };
        float width{ 0.f };
        textbox_justification justification{ textbox_justification::left };
        bool center_vertically{ false };
        std::string text_id{};
    };

    // TODO: This is for a future update
    //enum class region_display : u16
    //{
    //    invisible,
    //    text,
    //    shape
    //};

    //struct region_view
    //{
    //    region_display display{ region_display::invisible };
    //    u16 shape_id{ NO_INDEX };
    //    u16 text_width_in_chars{ 0 };
    //    bool has_text_frame{ true };
    //    std::string text_id{};
    //};

    //struct game_data_region_v2
    //{
    //    u16 x{ 0 }, y{ 0 };
    //    u16 width{ 0 }, height{ 0 };

    //    region_view active{};
    //    region_view hovered{};

    //    bool has_hover_state{ false };
    //    bool has_action{ false };

    //    u16 pointer_id{ 0 };
    //    u16 goto_scene{ NO_INDEX };
    //    std::string script;
    //};

    // OG regions would be:
    //  x, y, witdth, height as usual
    //  active.display = invisible
    //  hover.display = text
    //  hover.has_text_frame = true
    //  has_hover_state = true
    //  has_action = true

    // Text sections would be:
    // x, y as usual
    // width and height are calculated in game based on font
    // active.display = text
    // active.text_width_in_chars = whatever is wanted
    // active.has_text_frame = false
    // hover.display = invisible
    // has_hover_state = false
    // has_action = false

    // Shape buttons would be:
    // x, y, width, height as usual
    // active.display = shape
    // active.shape_id = whatever shape id local to the location
    // has_hover_state = true/false depending
    // hover.display = shape
    // hover.shape_id = hover state shape id
    // has_action = true

    // Menu items would have some trickery. First a 

    

    struct game_data_scene
    {
        std::string name;
        std::vector<std::string> description_id;
        std::string on_enter;
        std::string on_exit;
        u16 image_id{ 0 };
        u16 view_x{ 0 }, view_y{ 0 };
        u16 view_width{ 0 }, view_height{ 0 };
        u16 offset_x{ 0 }, offset_y{ 0 };
        u16 music_id{ NO_INDEX };
        std::vector<game_data_region> regions;
        std::vector<game_data_text_region> text_regions;
    };

    struct shape
    {
        u16 x{ 0 }, y{ 0 };
        u16 width{ 0 }, height{ 0 };
    };

    struct shape_container
    {
        std::string image_file;
        std::vector<shape> shapes;
        MPG::simple_image image;
        bool has_palette{ true };
    };

    struct game_data_location
    {
        std::string name;
        std::vector<std::string> backgrounds;
        std::vector<GLtexture> background_textures;
        std::vector<game_data_scene> scenes;
        std::vector<shape_container> shapes;
        std::vector<GLtexture> shapes_textures;
        std::string shapes_file;
        std::vector<u16> speakers{ 0, NO_INDEX, NO_INDEX, NO_INDEX, NO_INDEX, NO_INDEX, NO_INDEX, NO_INDEX };
    };

    struct dialogue_choice
    {
        std::string text_id;
        std::string script;
        std::string set_flag;
        std::string clear_flag;
        std::string check_flag;
        std::string add_item;
        u16 next_page_id{ NO_INDEX };
        bool enabled{ true };
        bool self_disable{ false };
        bool has_script{ false };
        bool has_set_flag{ false };
        bool has_clear_flag{ false };
        bool has_check_flag{ false };
    };

    struct dialogue_page
    {
        std::string text_id;
        std::string set_flag;
        std::string clear_flag;
        std::string check_flag;
        std::vector<dialogue_choice> choices;
        u16 next_page_id{ NO_INDEX };
        u16 speaker_id{ NO_INDEX };
        bool enabled{ true };
        bool self_disable{ false };
        bool has_set_flag{ false };
        bool has_clear_flag{ false };
        bool has_check_flag{ false };
    };
    
    struct dialogue
    {
        std::string name;
        std::vector<dialogue_page> pages;
    };

    struct speaker_info
    {
        std::string name;
        std::string image_path;
        MPG::simple_image image;
        GLtexture image_texture;
    };

    struct game_asset
    {
        std::string name{};
        std::filesystem::path relative_path{};
    };

    struct asset_collection
    {
        std::vector<game_asset> ui{};
        std::vector<game_asset> backgrounds{};
        std::vector<game_asset> music{};
        std::vector<game_asset> sfx{};
    };

    struct project_manifest
    {
        std::string game_name{};
        std::filesystem::path project_file{};
        std::filesystem::path game_file{};
        asset_collection assets{};
        u32 build_number{ 0 };
    };

    struct font
    {
        float width{ 6.f };
        float height{ 8.f };
        std::filesystem::path file_path{};
        GLtexture texture;
    };

    struct game_data
    {
        project_manifest manifest;
        std::vector<game_data_location> locations;
        std::vector<std::string> flags;
        std::vector<dialogue> dialogues;
        std::vector<speaker_info> speakers;
        string_table strings;

        std::string script_name{};

        i32 shape_start_id{ 10 };
        bool save_on_export{ true };

        font default_font;

        // Default color palette for the game UI.
        MPG::color_palette ui_palette
        {
            { 0x00, 0x00, 0x00, 0x00 },
            { 0x99, 0xE5, 0x50, 0x00 },
            { 0x6A, 0xBE, 0x30, 0x00 },
            { 0x22, 0x20, 0x34, 0x00 },
            { 0x32, 0x3C, 0x39, 0x00 },
            { 0x54, 0x61, 0x5B, 0x00 },
            { 0x67, 0x73, 0x68, 0x00 },
            { 0x7A, 0x85, 0x72, 0x00 },
            { 0xFB, 0xF2, 0x36, 0x00 },
            { 0x99, 0xE5, 0x50, 0x00 },
            { 0x6A, 0xBE, 0x30, 0x00 },
            { 0x37, 0x94, 0x6E, 0x00 },
            { 0x4B, 0x69, 0x2F, 0x00 },
            { 0x52, 0x4B, 0x24, 0x00 },
            { 0x32, 0x3C, 0x39, 0x00 },
            { 0x3F, 0x3F, 0x74, 0x00 },
            { 0x30, 0x60, 0x82, 0x00 },
            { 0x5B, 0x6E, 0xE1, 0x00 },
            { 0x63, 0x9B, 0xFF, 0x00 },
            { 0x32, 0x3C, 0x39, 0x00 },
            { 0x32, 0x3C, 0x39, 0x00 },
            { 0x54, 0x61, 0x5B, 0x00 },
            { 0x67, 0x73, 0x68, 0x00 },
            { 0x7A, 0x85, 0x72, 0x00 },
            { 0x9B, 0xAD, 0xB7, 0x00 },
            { 0x59, 0x56, 0x52, 0x00 },
            { 0x76, 0x42, 0x8A, 0x00 },
            { 0xAC, 0x32, 0x32, 0x00 },
            { 0xD9, 0x57, 0x63, 0x00 },
            { 0xD7, 0x7B, 0xBA, 0x00 },
            { 0x8F, 0x97, 0x4A, 0x00 },
            { 0x8A, 0x6F, 0x30, 0x00 },
        };

        void serialize(std::filesystem::path const& file_path);
        static std::shared_ptr<game_data> deserialize(std::filesystem::path const& file_path);
    };
}

