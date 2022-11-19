#include <json.hpp>
#include <fstream>
#include <filesystem>
#include <sstream>
#include "game_data.h"

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace NEONnoir
{
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
        shape,
        x, y,
        width, height
    );

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
        shape_container,
        image_file,
        has_palette,
        shapes
    );

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
        game_data_region,
        x, y,
        width, height,
        shape_id,
        pointer_id,
        goto_scene,
        description,
        script
    );

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
        game_data_scene,
        name,
        description,
        on_enter,
        on_exit,
        image_id,
        view_x,
        view_y,
        view_width,
        view_height,
        offset_x,
        offset_y,
        music_id,
        regions
    );

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
        game_data_location,
        name,
        backgrounds,
        scenes,
        shapes,
        shapes_file
    );

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
        dialogue_choice,
        text,
        script,
        set_flag,
        clear_flag,
        check_flag,
        next_page_id,
        enabled,
        self_disable,
        has_script,
        has_set_flag,
        has_clear_flag,
        has_check_flag
    );

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
        dialogue_page,
        text,
        set_flag,
        clear_flag,
        check_flag,
        next_page_id,
        choices,
        enabled,
        self_disable,
        has_set_flag,
        has_clear_flag,
        has_check_flag
    );

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
        dialogue, speaker_id, pages
    );

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
        speaker_info, name, image_path
    );

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
        game_data,
        locations,
        dialogues,
        speakers,
        script_name
    );

    void game_data::serialize(std::string const& filename)
    {
        auto savefile = std::ofstream{ filename, std::ios::trunc };
        if (savefile)
        {
            auto locs = json(locations);
            auto root = json
            {
                { "locations", locs},
                { "dialogues", json(dialogues)},
                { "speakers", json(speakers)},
                { "script_name", script_name},
                { "save_on_export", script_name},
            };

            savefile << root.dump(2);
            savefile.close();
        }
    }

    std::shared_ptr<game_data> game_data::deserialize(std::string const& filename)
    {
        if (!fs::exists(filename))
        {
            throw std::runtime_error{ std::format("File '{}' does not exist.", filename) };
        }

        auto savefile = std::ifstream{ filename };
        if (savefile)
        {
            auto data_path = fs::path{ filename }.parent_path();
            fs::current_path(data_path);

            auto buffer = std::stringstream{};
            buffer << savefile.rdbuf();

            auto j = json::parse(buffer.str());
            auto data = j.get<game_data>();

            for (auto & location : data.locations)
            {
                for (auto & background : location.backgrounds)
                {
                    auto bg_image = MPG::load_image(background);
                    location.background_textures.push_back(load_texture(bg_image));
                }

                for (auto& container : location.shapes)
                {
                    container.image = MPG::load_image(container.image_file);
                    location.shapes_textures.push_back(load_texture(container.image));
                }
            }

            for (auto& speaker : data.speakers)
            {
                speaker.image = MPG::load_image(speaker.image_path);
                speaker.image_texture = load_texture(speaker.image);
            }

            return std::make_shared<game_data>(data);
        }

        throw std::runtime_error{ "Could not read file" };
    }
}
