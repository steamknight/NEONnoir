#include <json.hpp>
#include <fstream>
#include <filesystem>
#include <sstream>
#include "game_data.h"

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;
namespace fs = std::filesystem;

// nlohmann json defines a macro NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT to make it easy to create serialization/deserialization of types,
// however, it doesn't work with ordered_json, which preserves the order of the properties. This here is a small alteration to support this.
#ifndef DEFINE_ORDERED_TYPE
#define DEFINE_ORDERED_TYPE(Type, ...)  \
    inline void to_json(nlohmann::ordered_json& nlohmann_json_j, const Type& nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, __VA_ARGS__)) } \
    inline void from_json(const nlohmann::json& nlohmann_json_j, Type& nlohmann_json_t) { Type nlohmann_json_default_obj; NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM_WITH_DEFAULT, __VA_ARGS__)) }
#endif

namespace NEONnoir
{
    DEFINE_ORDERED_TYPE(
        shape,
        x, y,
        width, height
    );

    DEFINE_ORDERED_TYPE(
        shape_container,
        image_file,
        has_palette,
        shapes
    );


    DEFINE_ORDERED_TYPE(
        game_data_region,
        x, y,
        width, height,
        shape_id,
        pointer_id,
        goto_scene,
        description_id,
        script
    );

    NLOHMANN_JSON_SERIALIZE_ENUM(textbox_justification, {
        {textbox_justification::left, "left"},
        {textbox_justification::center, "center"},
        {textbox_justification::right, "right"},
     });

    DEFINE_ORDERED_TYPE(
        game_data_text_region,
        x, y,
        width,
        justification,
        center_vertically,
        text_id
    );

    DEFINE_ORDERED_TYPE(
        game_data_scene,
        name,
        description_id,
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
        regions,
        text_regions,
        is_cutscene
    );

    DEFINE_ORDERED_TYPE(
        game_data_location,
        name,
        backgrounds,
        scenes,
        shapes,
        shapes_file,
        speakers
    );

    DEFINE_ORDERED_TYPE(
        dialogue_choice,
        text_id,
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

    DEFINE_ORDERED_TYPE(
        dialogue_page,
        speaker_id,
        text_id,
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

    DEFINE_ORDERED_TYPE(
        dialogue, name, pages
    );

    DEFINE_ORDERED_TYPE(
        speaker_info, name, image_path
    );

    DEFINE_ORDERED_TYPE(
        game_asset,
        name,
        relative_path
    );

    DEFINE_ORDERED_TYPE(
        asset_collection,
        ui,
        backgrounds,
        music,
        sfx
    );

    DEFINE_ORDERED_TYPE(
        project_manifest,
        game_name,
        build_number,
        project_file,
        game_file,
        assets
    );

    DEFINE_ORDERED_TYPE(
        font,
        width,
        height,
        file_path
    );

    DEFINE_ORDERED_TYPE(
        game_data,
        manifest,
        default_font,
        locations,
        dialogues,
        speakers,
        script_name
    );

    void game_data::serialize(std::filesystem::path const& file_path)
    {
        auto savefile = std::ofstream{ file_path, std::ios::trunc };
        if (savefile)
        {
            auto root = ordered_json
            {
                { "manifest", ordered_json(manifest) },
                { "default_font", ordered_json(default_font) },
                { "locations", ordered_json(locations) },
                { "dialogues", ordered_json(dialogues) },
                { "speakers", ordered_json(speakers) },
                { "script_name", script_name },
                { "save_on_export", script_name },
            };

            savefile << root.dump(2);
            savefile.close();
        }

        auto strings_path = (file_path.parent_path() / file_path.stem());
        strings_path += "_strings.json";
        strings.serialize(strings_path);
    }

    std::shared_ptr<game_data> game_data::deserialize(std::filesystem::path const& file_path)
    {
        if (!fs::exists(file_path))
        {
            throw std::runtime_error{ std::format("File '{}' does not exist.", file_path.string()) };
        }

        auto savefile = std::ifstream{ file_path };
        if (savefile)
        {
            auto data_path = fs::path{ file_path }.parent_path();
            fs::current_path(data_path);

            auto buffer = std::stringstream{};
            buffer << savefile.rdbuf();

            auto j = json::parse(buffer.str());
            auto data = j.get<game_data>();

            for (auto& asset : data.manifest.assets.backgrounds)
            {
                asset.texture = load_texture(MPG::load_image(asset.relative_path));
            }

            for (auto & location : data.locations)
            {
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

            if (fs::exists(data.default_font.file_path))
            {
                data.default_font.texture = load_texture(MPG::load_image(data.default_font.file_path, true));
            }

            //// begin temporary
            //for (auto& location : data.locations)
            //{
            //    for (auto& scene : location.scenes)
            //    {
            //        for (auto& description : scene.description)
            //        {
            //            scene.description_id.push_back(std::string{ data.strings.create_string_entry(description) });
            //        }

            //        for (auto& region : scene.regions)
            //        {
            //            region.description_id = data.strings.create_string_entry(region.description);
            //        }
            //    }
            //}

            //for (auto& dialogue : data.dialogues)
            //{
            //    for (auto& page : dialogue.pages)
            //    {
            //        page.text_id = data.strings.create_string_entry(page.text);

            //        for (auto& choice : page.choices)
            //        {
            //            choice.text_id = data.strings.create_string_entry(choice.text);
            //        }
            //    }
            //}
            //// end temporary

            auto strings_path = (file_path.parent_path() / file_path.stem());
            strings_path += "_strings.json";
            data.strings.deserialize(strings_path);

            return std::make_shared<game_data>(data);
        }

        throw std::runtime_error{ "Could not read file" };
    }

    auto find_asset_by_name(std::vector<game_asset> const& assets, std::string const& asset_name)
    {
        auto has_name = [&](game_asset const& asset)
        {
            return asset.name == asset_name;
        };

        return std::find_if(assets.begin(), assets.end(), has_name);
    };

    bool asset_collection::contains_asset(std::string const& asset_name)
    {
        return (find_asset_by_name(ui, asset_name) != ui.end()
            || find_asset_by_name(backgrounds, asset_name) != backgrounds.end()
            || find_asset_by_name(music, asset_name) != music.end()
            || find_asset_by_name(sfx, asset_name) != sfx.end()
        );
    }

    size_t asset_collection::get_asset_id(std::string const& name)
    {
        size_t id = 0;
        auto get_id = [&](auto const& assets)
        {
            for (auto const& asset : assets)
            {
                if (asset.name == name)
                    return id;

                id++;
            }

            return id;
        };

        if (find_asset_by_name(ui, name) != ui.end())
        {
            return get_id(ui);
        }
        else if (find_asset_by_name(backgrounds, name) != backgrounds.end())
        {
            return ui.size() + get_id(backgrounds);
        }
        else if (find_asset_by_name(music, name) != music.end())
        {
            return ui.size() + backgrounds.size() + get_id(music);
        }
        else if (find_asset_by_name(sfx, name) != sfx.end())
        {
            return ui.size() + backgrounds.size() + music.size() + get_id(sfx);
        }
        else
        {
            return 0;
        }
    }
}
