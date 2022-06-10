#include <fstream>
#include "packfile.h"

namespace fs = std::filesystem;

namespace NEONnoir
{
    neon_packfile generate_packfile(std::shared_ptr<game_data> const& data)
    {
        auto pak = neon_packfile{};

        for (auto const& location : data->locations)
        {
            auto loc = neon_location{};
            loc.name_id = static_cast<uint16_t>(pak.string_table.size());
            pak.string_table.push_back(location.name);

            loc.background_count = static_cast<uint16_t>(location.backgrounds.size());
            loc.first_background_id = static_cast<uint16_t>(pak.string_table.size());
            for (auto const& background : location.backgrounds)
            {
                pak.string_table.push_back(background);
            }

            loc.scene_count = static_cast<uint16_t>(location.scenes.size());
            loc.first_scene_id = static_cast<uint16_t>(pak.scenes.size());

            for (auto const& scene : location.scenes)
            {
                auto s = neon_scene{};
                s.name_id = static_cast<uint16_t>(pak.string_table.size());
                pak.string_table.push_back(scene.name);

                s.background_id = scene.image_id;

                s.region_count = static_cast<uint16_t>(scene.regions.size());
                s.first_region_id = static_cast<uint16_t>(pak.regions.size());

                for (auto const& region : scene.regions)
                {
                    auto r = neon_region{};
                    r.x = region.x;
                    r.y = region.y;
                    r.width = region.width;
                    r.height = region.height;

                    r.description_id = static_cast<uint16_t>(pak.string_table.size());
                    pak.string_table.push_back(region.description);

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

    void serialize_to_neon_pak(fs::path file_path, std::shared_ptr<game_data> const& data)
    {
        auto pak = generate_packfile(data);

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
            write(neonpack, location.background_count);
            write(neonpack, location.first_background_id);
            write(neonpack, location.scene_count);
            write(neonpack, location.first_scene_id);
        }

        // Write all scenes
        write(neonpack, static_cast<uint32_t>(pak.scenes.size()));
        for (auto const& scene : pak.scenes)
        {
            write(neonpack, scene.name_id);
            write(neonpack, scene.background_id);
            write(neonpack, scene.region_count);
            write(neonpack, scene.first_region_id);
        }

        // Write all regions
        write(neonpack, static_cast<uint32_t>(pak.regions.size()));
        for (auto const& region : pak.regions)
        {
            write(neonpack, region.x);
            write(neonpack, region.y);
            write(neonpack, region.width);
            write(neonpack, region.height);
            write(neonpack, region.description_id);
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

        neonpack.close();
    }
}