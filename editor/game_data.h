#pragma once
#include <vector>
#include <string>
#include "glfw_utils.h"
#include "utils.h"
#include "simple_image.h"

namespace NEONnoir
{
    enum class game_data_chunk_type : uint32_t
    {
        location = 1,
        bytecode,
        strings,
        items
    };

    struct game_data_region
    {
        uint16_t x{ 0 }, y{ 0 };
        uint16_t width{ 0 }, height{ 0 };
        uint16_t shape_id{ 0 };
        uint16_t pointer_id{ 0 };
        std::string description;
        std::string script;
    };

    struct game_data_scene
    {
        std::string name;
        std::string on_enter;
        std::string on_exit;
        uint16_t image_id{ 0 };
        uint16_t view_x{ 0 }, view_y{ 0 };
        uint16_t view_width{ 0 }, view_height{ 0 };
        uint16_t offset_x{ 0 }, offset_y{ 0 };
        uint16_t music_id{ 0xFFFF };
        std::vector<game_data_region> regions;
    };

    struct shape
    {
        uint16_t x{ 0 }, y{ 0 };
        uint16_t width{ 0 }, height{ 0 };
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
    };

    struct dialogue_choice
    {
        std::string text;
        std::string script;
        std::string set_flag;
        std::string check_flag;
        uint16_t next_page_id{ 0xFFFF };
        bool enabled{ true };
        bool self_disable{ false };
        bool has_script{ false };
        bool has_set_flag{ false };
        bool has_check_flag{ false };
    };

    struct dialogue_page
    {
        std::string text;
        std::string set_flag;
        std::string check_flag;
        std::vector<dialogue_choice> choices;
        uint16_t next_page_id{ 0xFFFF };
        bool enabled{ true };
        bool self_disable{ false };
        bool has_set_flag{ false };
        bool has_check_flag{ false };
    };

    struct dialogue
    {
        std::string speaker;
        uint16_t image_id{ 0xFFFF };
        std::vector<dialogue_page> pages;
    };

    struct game_data
    {
        std::vector<game_data_location> locations;
        std::vector<std::string> flags;
        std::vector<dialogue> dialogues;
        std::vector<std::string> extra_files{ "data/theme.mod", "data/ambient.mod" };

        std::string script_name{};

        void serialize(std::string const& filename);
        int32_t shape_start_id{ 10 };
        bool save_on_export{ true };

        std::string filename{};

        static std::shared_ptr<game_data> deserialize(std::string const& filename);
    };
}

