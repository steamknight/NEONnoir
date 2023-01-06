#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "glfw_utils.h"
#include "utils.h"
#include "simple_image.h"
#include "string_table.h"

namespace NEONnoir
{
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
        u16 goto_scene{ 0xFFFF };
        std::string description;
        std::string description_id;
        std::string script;
    };

    struct game_data_scene
    {
        std::string name;
        std::vector<std::string> description;
        std::vector<std::string> description_id;
        std::string on_enter;
        std::string on_exit;
        u16 image_id{ 0 };
        u16 view_x{ 0 }, view_y{ 0 };
        u16 view_width{ 0 }, view_height{ 0 };
        u16 offset_x{ 0 }, offset_y{ 0 };
        u16 music_id{ 0xFFFF };
        std::vector<game_data_region> regions;
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
        std::vector<u16> speakers{ 0, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF };
    };

    struct dialogue_choice
    {
        std::string text;
        std::string text_id;
        std::string script;
        std::string set_flag;
        std::string clear_flag;
        std::string check_flag;
        std::string add_item;
        u16 next_page_id{ 0xFFFF };
        bool enabled{ true };
        bool self_disable{ false };
        bool has_script{ false };
        bool has_set_flag{ false };
        bool has_clear_flag{ false };
        bool has_check_flag{ false };
    };

    struct dialogue_page
    {
        std::string text;
        std::string text_id;
        std::string set_flag;
        std::string clear_flag;
        std::string check_flag;
        std::vector<dialogue_choice> choices;
        u16 next_page_id{ 0xFFFF };
        u16 speaker_id{ 0xFFFF };
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

    struct game_data
    {
        std::vector<game_data_location> locations;
        std::vector<std::string> flags;
        std::vector<dialogue> dialogues;
        std::vector<speaker_info> speakers;
        string_table strings;

        std::string script_name{};

        void serialize(std::string const& filename);
        i32 shape_start_id{ 10 };
        bool save_on_export{ true };

        std::string filename{};

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

        static std::shared_ptr<game_data> deserialize(std::string const& filename);
    };
}

