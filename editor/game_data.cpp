#include <json.hpp>
#include <fstream>
#include <filesystem>
#include <sstream>
#include "game_data.h"

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace NEONnoir
{

    void to_json(json& j, shape const& s)
    {
        j = json{
            { "x",              s.x },
            { "y",              s.y },
            { "width",          s.width },
            { "height",         s.height },
        };
    }

    void to_json(json& j, shape_container const& s)
    {
        j = json{
            { "image_file",     s.image_file },
            { "has_palette",    s.has_palette },
            { "shapes",         s.shapes }
        };
    }

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
            { "scenes", l.scenes },
            { "shapes", l.shapes },
            { "shapes_file", l.shapes_file },
        };
    }

    void to_json(json& j, dialogue_choice const& c)
    {
        j = json{
            { "text",           c.text },
            { "script",         c.script },
            { "set_flag",       c.set_flag },
            { "check_flag",     c.check_flag },
            { "next_page_id" ,  c.next_page_id },
            { "enabled",        c.enabled },
            { "self_disable",   c.self_disable },
            { "has_script",     c.has_script },
            { "has_set_flag",   c.has_set_flag },
            { "has_check_flag", c.has_check_flag },
        };
    }

    void to_json(json& j, dialogue_page const& p)
    {
        j = json{
            { "text",           p.text },
            { "set_flag",       p.set_flag },
            { "check_flag",     p.check_flag },
            { "choices",        p.choices },
            { "next_page_id" ,  p.next_page_id },
            { "enabled",        p.enabled },
            { "self_disable",   p.self_disable },
            { "has_set_flag",   p.has_set_flag },
            { "has_check_flag", p.has_check_flag },
        };
    }

    void to_json(json& j, dialogue const& d)
    {
        j = json{
            { "speaker",        d.speaker },
            { "image_id",       d.image_id },
            { "pages" ,         d.pages },
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

    void from_json(const json& j, shape& s)
    {
        j.at("x").get_to(s.x);
        j.at("y").get_to(s.y);
        j.at("width").get_to(s.width);
        j.at("height").get_to(s.height);
    }

    void from_json(const json& j, shape_container& s)
    {
        j.at("image_file").get_to(s.image_file);
        j.at("has_palette").get_to(s.has_palette);
        j.at("shapes").get_to(s.shapes);
    }

    void from_json(const json& j, game_data_location& l)
    {
        j.at("name").get_to(l.name);
        j.at("backgrounds").get_to(l.backgrounds);
        j.at("scenes").get_to(l.scenes);
        if (j.contains("shapes"))
            j.at("shapes").get_to(l.shapes);
        j.at("shapes_file").get_to(l.shapes_file);
    }

    void from_json(const json& j, dialogue_choice& c)
    {
        j.at("text").get_to(c.text);
        j.at("script").get_to(c.script);
        j.at("set_flag").get_to(c.set_flag);
        j.at("check_flag").get_to(c.check_flag);
        j.at("next_page_id").get_to(c.next_page_id);
        j.at("enabled").get_to(c.enabled);
        j.at("self_disable").get_to(c.self_disable);
        j.at("has_script").get_to(c.has_script);
        j.at("has_set_flag").get_to(c.has_set_flag);
        j.at("has_check_flag").get_to(c.has_check_flag);
    }

    void from_json(const json& j, dialogue_page& p)
    {
        j.at("text").get_to(p.text);
        j.at("set_flag").get_to(p.set_flag);
        j.at("check_flag").get_to(p.check_flag);
        j.at("choices").get_to(p.choices);
        j.at("next_page_id").get_to(p.next_page_id);
        j.at("enabled").get_to(p.enabled);
        j.at("has_set_flag").get_to(p.has_set_flag);
        j.at("has_check_flag").get_to(p.has_check_flag);
    }

    void from_json(const json& j, dialogue& d)
    {
        j.at("speaker").get_to(d.speaker);
        j.at("image_id").get_to(d.image_id);
        j.at("pages").get_to(d.pages);
    }

    void from_json(const json& j, game_data& g)
    {
        j.at("locations").get_to(g.locations);
        j.at("dialogues").get_to(g.dialogues);
    }

    void game_data::serialize(std::string const& filename)
    {
        auto savefile = std::ofstream{ filename, std::ios::trunc };
        if (savefile)
        {
            auto locs = json(locations);
            auto root = json
            {
                { "locations", locs},
                { "dialogues", json(dialogues)}
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

                for (auto& container : location.shapes)
                {
                    location.shapes_textures.push_back(load_texture(container.image_file));
                    container.palette = read_bmp_palette(container.image_file);
                }
            }


            return std::make_shared<game_data>(data);
        }

        throw std::runtime_error{ "Could not read file" };
    }
}
