#include <json.hpp>
#include <fstream>
#include <filesystem>
#include <sstream>
#include "game_data.h"

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace NEONnoir
{
    void to_json(json& j, game_data_region const& r)
    {
        j = json{
            { "x", r.x },
            { "y", r.y },
            { "width", r.width },
            { "height", r.height },
            { "shape_id", r.shape_id },
            { "pointer_id", r.pointer_id },
            { "description", r.description },
            { "script", r.script }
        };
    }

    void to_json(json& j, game_data_scene const& s)
    {
        j = json{
            { "name", s.name },
            { "image_id", s.image_id },
            { "view_x", s.view_x},
            { "view_y", s.view_y},
            { "view_width", s.view_width},
            { "view_height", s.view_height},
            { "offset_x", s.offset_x},
            { "offset_y", s.offset_y},
            { "regions", s.regions}
        };
    }

    void to_json(json& j, game_data_location const& l)
    {
        j = json{
            { "name", l.name },
            { "backgrounds", l.backgrounds },
            { "scenes", l.scenes}
        };
    }

    void from_json(const json& j, game_data_region& r)
    {
        j.at("x").get_to(r.x);
        j.at("y").get_to(r.y);
        j.at("width").get_to(r.width);
        j.at("height").get_to(r.height);
        j.at("shape_id").get_to(r.shape_id);
        j.at("pointer_id").get_to(r.pointer_id);
        j.at("description").get_to(r.description);
        j.at("script").get_to(r.script);
    }

    void from_json(const json& j, game_data_scene& s)
    {
        j.at("name").get_to(s.name);
        j.at("image_id").get_to(s.image_id);
        j.at("view_x").get_to(s.view_x);
        j.at("view_y").get_to(s.view_y);
        j.at("view_width").get_to(s.view_width);
        j.at("view_height").get_to(s.view_height);
        j.at("offset_x").get_to(s.offset_x);
        j.at("offset_y").get_to(s.offset_y);
        j.at("regions").get_to(s.regions);
    }

    void from_json(const json& j, game_data_location& l)
    {
        j.at("name").get_to(l.name);
        j.at("backgrounds").get_to(l.backgrounds);
        j.at("scenes").get_to(l.scenes);
    }

    void from_json(const json& j, game_data& g)
    {
        j.at("locations").get_to(g.locations);
    }

    void game_data::serialize(std::string const& filename)
    {
        auto savefile = std::ofstream{ filename, std::ios::trunc };
        if (savefile)
        {
            auto locs = json(locations);
            auto root = json
            {
                { "locations", locs}
            };

            savefile << root;
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
            auto buffer = std::stringstream{};
            buffer << savefile.rdbuf();

            auto j = json::parse(buffer.str());
            auto data = j.get<game_data>();

            for (auto & location : data.locations)
            {
                for (auto & background : location.backgrounds)
                {
                    location.background_textures.push_back(load_texture(background));
                }
            }

            return std::make_shared<game_data>(data);
        }

        throw std::runtime_error{ "Could not read file" };
    }
}
