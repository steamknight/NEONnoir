#include <fstream>
#include <filesystem>
#include <format>
#include "packfile.h"

namespace fs = std::filesystem;

namespace NEONnoir
{
    neon_packfile generate_packfile(std::shared_ptr<game_data> const& data, assembler_result const& result)
    {
        auto pak = neon_packfile{};

        for (auto& [k, v] : result.strings)
        {
            pak.string_table.push_back(v.value);
        }

        for (auto const& location : data->locations)
        {
            auto loc = neon_location{};
            loc.name_id = static_cast<uint16_t>(pak.string_table.size());
            pak.string_table.push_back(location.name);

            loc.first_bg_id = static_cast<uint16_t>(pak.string_table.size());
            loc.last_bg_id = loc.first_bg_id + static_cast<uint16_t>(location.backgrounds.size()) - 1;
            for (auto const& background : location.backgrounds)
            {
                auto path = std::filesystem::path{ background };
                
                pak.string_table.push_back(path.stem().replace_extension("iff").string());
            }

            loc.first_scene_id = static_cast<uint16_t>(pak.scenes.size());
            loc.last_scene_id = loc.first_scene_id + static_cast<uint16_t>(location.scenes.size()) - 1;

            for (auto const& scene : location.scenes)
            {
                auto s = neon_scene{};
                s.name_id = static_cast<uint16_t>(pak.string_table.size());
                pak.string_table.push_back(scene.name);

                s.background_id = scene.image_id;

                s.first_region_id = static_cast<uint16_t>(pak.regions.size());
                s.last_region_id = s.first_region_id + static_cast<uint16_t>(scene.regions.size()) - 1;

                for (auto const& region : scene.regions)
                {
                    auto r = neon_region{};
                    r.x1 = region.x;
                    r.y1 = region.y;
                    r.x2 = region.x + region.width;
                    r.y2 = region.y + region.height;
                    r.pointer_id = region.pointer_id;

                    r.description_id = static_cast<uint16_t>(pak.string_table.size());
                    pak.string_table.push_back(region.description);

                    if (region.script != "")
                    {
                        if (result.scripts_meta.count(region.script) == 0)
                        {
                            throw packer_error(std::format("Region '{}/{}/{}' references non-existing script '{}'", location.name, scene.name, region.description, region.script));
                        }
                        else
                        {
                            r.script_offset = result.scripts_meta.at(region.script).start;
                        }
                    }

                    pak.regions.push_back(r);
                }

                pak.scenes.push_back(s);
            }

            pak.locations.push_back(loc);
        }

        return pak;
    }

    void write(std::ofstream& stream, uint16_t value)
    {
        auto data = reinterpret_cast<char*>(&value);
        stream.write(&data[1], 1);
        stream.write(&data[0], 1);
    }

    void write(std::ofstream& stream, uint32_t value)
    {
        auto data = reinterpret_cast<char*>(&value);
        stream.write(&data[3], 1);
        stream.write(&data[2], 1);
        stream.write(&data[1], 1);
        stream.write(&data[0], 1);
    }

    void serialize_to_neon_pak(fs::path file_path, std::shared_ptr<game_data> const& data, assembler_result const& result)
    {
        auto pak = generate_packfile(data, result);

        if (!file_path.has_extension())
        {
            file_path = file_path.replace_extension("neon");
        }

        auto neonpack = std::ofstream{ file_path, std::ios::binary | std::ios::trunc };

        // Write the header
        neonpack.write((char*)&pak.header.magic, 4);
        write(neonpack, pak.header.major_version);
        write(neonpack, pak.header.minor_version);

        // Write all locations
        write(neonpack, static_cast<uint32_t>(pak.locations.size()));
        for (auto const& location: pak.locations)
        {
            write(neonpack, location.name_id);
            write(neonpack, location.first_bg_id);
            write(neonpack, location.last_bg_id);
            write(neonpack, location.first_scene_id);
            write(neonpack, location.last_scene_id);
        }

        // Write all scenes
        write(neonpack, static_cast<uint32_t>(pak.scenes.size()));
        for (auto const& scene : pak.scenes)
        {
            write(neonpack, scene.name_id);
            write(neonpack, scene.background_id);
            write(neonpack, scene.first_region_id);
            write(neonpack, scene.last_region_id);
        }

        // Write all regions
        write(neonpack, static_cast<uint32_t>(pak.regions.size()));
        for (auto const& region : pak.regions)
        {
            write(neonpack, region.x1);
            write(neonpack, region.y1);
            write(neonpack, region.x2);
            write(neonpack, region.y2);
            write(neonpack, region.pointer_id);
            write(neonpack, region.description_id);
            write(neonpack, region.script_offset);
        }

        // Write the string table "header"
        write(neonpack, static_cast<uint32_t>(pak.string_table.size()));

        auto string_size = 0u;
        for (auto const& entry : pak.string_table)
        {
            string_size += static_cast<uint32_t>(entry.size());
            string_size += 4;
        }
        write(neonpack, string_size);

        for (auto const& entry : pak.string_table)
        {
            write(neonpack, static_cast<uint32_t>(entry.size()));
            neonpack.write(entry.data(), entry.size());
        }

        // Write the bytecode "header

        write(neonpack, static_cast<uint32_t>(result.bytecode.size()));
        for (auto const& code : result.bytecode)
        {
            write(neonpack, static_cast<uint16_t>(code));
        }

        neonpack.close();
    }
}