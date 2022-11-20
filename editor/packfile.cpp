#include <fstream>
#include <filesystem>
#include <format>
#include "packfile.h"

namespace fs = std::filesystem;

namespace NEONnoir
{
    uint16_t get_script_offset(std::string const& script_name, assembler_result const& result)
    {
        if (script_name == "")
        {
            return 0xFFFF;
        }

        if (result.scripts_meta.count(script_name) == 0)
        {
            throw packer_error(std::format("Reference to non-existing script '{}'", script_name));
        }

        return result.scripts_meta.at(script_name).start;
    }

    uint16_t get_flag_id(std::string const& flag, assembler_result const& result)
    {
        if (result.flags.count(flag) == 0)
        {
            throw packer_error(std::format("Reference to non-existing flag '{}'", flag));
        }

        return result.flags.at(flag);
    }

    uint16_t get_const_id(std::string const& constant, assembler_result const& result)
    {
        if (result.constants.count(constant) == 0)
        {
            throw packer_error(std::format("Reference to non-existing constant '{}'", constant));
        }

        return result.constants.at(constant).value;
    }

    neon_packfile generate_packfile(std::shared_ptr<game_data> const& data, assembler_result const& result)
    {
        auto pak = neon_packfile{};

        auto script_strings = std::vector<string_constant>{};
        for (auto& [k, v] : result.strings)
        {
            script_strings.push_back(v);
        }

        std::sort(script_strings.begin(), script_strings.end(),
            [](string_constant a, string_constant b)
            {
                return a.id < b.id;
            });

        for (auto& text : script_strings)
        {
            pak.string_table.push_back(text.value);
        }

        // Before any other palettes, lets put the speaker palettes
        for (auto const& speaker : data->speakers)
        {
            pak.palettes.push_back(speaker.image.color_palette);
        }

        for (auto const& location : data->locations)
        {
            auto loc = neon_location{};
            loc.name_id = to<uint16_t>(pak.string_table.size());
            pak.string_table.push_back(location.name);

            loc.first_bg_id = to<uint16_t>(pak.string_table.size());
            loc.last_bg_id = loc.first_bg_id + to<uint16_t>(location.backgrounds.size()) - 1;
            for (auto const& background : location.backgrounds)
            {
                auto path = std::filesystem::path{ background };
                
                pak.string_table.push_back(path.stem().replace_extension("iff").string());
            }

            loc.first_scene_id = to<uint16_t>(pak.scenes.size());
            loc.last_scene_id = loc.first_scene_id + to<uint16_t>(location.scenes.size()) - 1;

            for (auto const& scene : location.scenes)
            {
                auto s = neon_scene{};
                s.name_id = to<uint16_t>(pak.string_table.size());
                pak.string_table.push_back(scene.name);

                if (scene.description.size() > 0)
                {
                    s.first_desc_id = to<uint16_t>(pak.string_table.size());

                    for (auto& desc : scene.description)
                    {
                       pak.string_table.push_back(desc);
                    }

                    // Compensate for the extra string added to the table
                    s.last_desc_id = to<uint16_t>(pak.string_table.size()) - 1;
                }

                s.background_id = scene.image_id;

                s.first_region_id = to<uint16_t>(pak.regions.size());
                s.last_region_id = s.first_region_id + to<uint16_t>(scene.regions.size()) - 1;

                s.on_enter = get_script_offset(scene.on_enter, result);
                s.on_exit = get_script_offset(scene.on_exit, result);

                s.music_id = scene.music_id;

                for (auto const& region : scene.regions)
                {
                    auto r = neon_region{};
                    r.x1 = region.x;
                    r.y1 = region.y;
                    r.x2 = region.x + region.width;
                    r.y2 = region.y + region.height;
                    r.pointer_id = region.pointer_id;
                    r.goto_scene = region.goto_scene;

                    if (region.description.size() > 0)
                    {
                        r.description_id = to<uint16_t>(pak.string_table.size());
                        pak.string_table.push_back(region.description);
                    }
                    else
                    {
                        r.description_id = 0xFFFF;
                    }

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
                    else
                    {
                        r.script_offset = 0xFFFF;
                    }

                    pak.regions.push_back(r);
                }

                pak.scenes.push_back(s);
            }

            if (location.shapes.size() > 0)
            {
                auto shape_id = to<uint16_t>(data->shape_start_id);
                loc.first_shape_id = to<uint16_t>(pak.shapes.size());
                for (auto const& container : location.shapes)
                {
                    for (auto const& shape : container.shapes)
                    {
                        pak.shapes.push_back({ shape_id, to<uint16_t>(pak.palettes.size()) });
                        shape_id++;
                    }

                    pak.palettes.push_back(container.image.color_palette);
                }
                loc.last_shape_id = to<uint16_t>(pak.shapes.size()) - 1; // compensate for the extra +1 at the end of loop

                if (location.shapes_file == "")
                {
                    throw std::runtime_error("Missing shapes file name.");
                }
                loc.shapes_file = to<uint16_t>(pak.string_table.size());
                pak.string_table.push_back(location.shapes_file);
            }

            auto max_speakers = std::min(to<size_t>(8), location.speakers.size());
            for (size_t i = 0; i < max_speakers; i++)
            {
                loc.speakers[i] = location.speakers[i];
            }

            pak.locations.push_back(loc);
        }

        for (auto const& dialogue : data->dialogues)
        {
            auto d = neon_dialogue{};
            d.speaker_id = dialogue.speaker_id;

            d.first_page_id = to<uint16_t>(pak.pages.size());
            auto page_count = 0;
            for (auto const& page : dialogue.pages)
            {
                auto p = neon_page{};
                p.text_id = to<uint16_t>(pak.string_table.size());
                pak.string_table.push_back(page.text);

                p.page_id = page.next_page_id;
                if (p.page_id != 0xFFFF) p.page_id += d.first_page_id;

                p.first_choice_id = (page.choices.size() > 0) ? to<uint16_t>(pak.choices.size()) : 0xFFFF;
                auto choice_count = 0;
                for (auto const& choice : page.choices)
                {
                    auto c = neon_choice{};
                    c.text_id = to<uint16_t>(pak.string_table.size());
                    pak.string_table.push_back("*" + choice.text);

                    c.page_id = choice.next_page_id;
                    if (c.page_id != 0xFFFF) c.page_id += d.first_page_id;

                    c.script_offset = !choice.has_script
                        ? 0xFFFF
                        : get_script_offset(choice.script, result);

                    c.set_flag = !choice.has_set_flag
                        ? 0xFFFF
                        : get_flag_id(choice.set_flag, result);

                    c.clear_flag = !choice.has_clear_flag
                        ? 0xFFFF
                        : get_flag_id(choice.clear_flag, result);

                    c.check_flag = !choice.has_check_flag
                        ? 0xFFFF
                        : get_flag_id(choice.check_flag, result);

                    c.enabled = choice.enabled ? 0xFF : 0;

                    c.self_disable = choice.self_disable ? 0xFF : 0;

                    pak.choices.push_back(c);

                    choice_count++;
                }

                p.set_flag = !page.has_set_flag
                    ? 0xFFFF
                    : get_flag_id(page.set_flag, result);

                p.clear_flag = !page.has_clear_flag
                    ? 0xFFFF
                    : get_flag_id(page.clear_flag, result);

                p.check_flag = !page.has_check_flag
                    ? 0xFFFF
                    : get_flag_id(page.check_flag, result);

                p.choice_count = to<uint16_t>(page.choices.size());
                p.enabled = page.enabled ? 0xFF : 0;
                p.self_disable = page.self_disable ? 0xFF : 0;
                
                pak.pages.push_back(p);

                page_count++;
            }

            d.page_count = to<uint16_t>(dialogue.pages.size());
            pak.dialogues.push_back(d);
        }

        // Calculate all words
        for (auto const& text : pak.string_table)
        {
            auto word_list = neon_word_list{};

            auto start_idx = 0;
            auto current = 0;

            while (start_idx < text.size())
            {
                current = start_idx;
                while (current < text.size() && text[current] != ' ' && text[current] != '\n')
                {
                    current++;
                }

                word_list.words.push_back(
                    {
                        to<uint16_t>(start_idx),
                        to<uint16_t>(current - 1)
                    }
                );

                if (text[current] == '\n')
                {
                    word_list.words.push_back({});
                }

                start_idx = current + 1;
            }

            word_list.word_count = to<uint16_t>(word_list.words.size());
            pak.words_table.push_back(word_list);
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
        if (!neonpack)
        {
            throw std::runtime_error{ std::format("Could not open/create '{}'", file_path.string()) };
        }

        // Write the header
        neonpack.write((char*)&pak.header.magic, 4);
        write(neonpack, pak.header.major_version);
        write(neonpack, pak.header.minor_version);

        // Write all locations
        neonpack.write(locations_header, 4);
        write(neonpack, to<uint32_t>(pak.locations.size()));
        for (auto const& location: pak.locations)
        {
            write(neonpack, location.name_id);
            write(neonpack, location.first_bg_id);
            write(neonpack, location.last_bg_id);
            write(neonpack, location.first_scene_id);
            write(neonpack, location.last_scene_id);
            write(neonpack, location.first_shape_id);
            write(neonpack, location.last_shape_id);
            write(neonpack, location.shapes_file);
            neonpack.write(reinterpret_cast<char const*>(&location.speakers[0]), sizeof(location.speakers));
        }

        // Write all scenes
        neonpack.write(scenes_header, 4);
        write(neonpack, to<uint32_t>(pak.scenes.size()));
        for (auto const& scene : pak.scenes)
        {
            write(neonpack, scene.name_id);
            write(neonpack, scene.first_desc_id);
            write(neonpack, scene.last_desc_id);
            write(neonpack, scene.on_enter);
            write(neonpack, scene.on_exit);
            write(neonpack, scene.background_id);
            write(neonpack, scene.first_region_id);
            write(neonpack, scene.last_region_id);
            write(neonpack, scene.music_id);
        }

        // Write all regions
        neonpack.write(regions_header, 4);
        write(neonpack, to<uint32_t>(pak.regions.size()));
        for (auto const& region : pak.regions)
        {
            write(neonpack, region.x1);
            write(neonpack, region.y1);
            write(neonpack, region.x2);
            write(neonpack, region.y2);
            write(neonpack, region.pointer_id);
            write(neonpack, region.goto_scene);
            write(neonpack, region.description_id);
            write(neonpack, region.script_offset);
        }

        // Dialogues
        neonpack.write(dialogues_header, 4);
        write(neonpack, to<uint32_t>(pak.dialogues.size()));

        for (auto const& dialogue : pak.dialogues)
        {
            write(neonpack, dialogue.first_page_id);
            write(neonpack, dialogue.page_count);
            write(neonpack, dialogue.speaker_id);
        }

        // Pages
        neonpack.write(pages_header, 4);
        write(neonpack, to<uint32_t>(pak.pages.size()));

        for (auto const& page : pak.pages)
        {
            write(neonpack, page.text_id);
            write(neonpack, page.set_flag);
            write(neonpack, page.clear_flag);
            write(neonpack, page.check_flag);
            write(neonpack, page.page_id);
            write(neonpack, page.first_choice_id);
            write(neonpack, page.choice_count);
            neonpack.write(&page.enabled, 1);
            neonpack.write(&page.self_disable, 1);
        }

        // Choices
        neonpack.write(choices_header, 4);
        write(neonpack, to<uint32_t>(pak.choices.size()));

        for (auto const& choice : pak.choices)
        {
            write(neonpack, choice.text_id);
            write(neonpack, choice.set_flag);
            write(neonpack, choice.clear_flag);
            write(neonpack, choice.check_flag);
            write(neonpack, choice.page_id);
            write(neonpack, choice.script_offset);
            neonpack.write(&choice.enabled, 1);
            neonpack.write(&choice.self_disable, 1);
        }

        // Write the bytecode "header
        neonpack.write(bytecode_header, 4);
        write(neonpack, to<uint32_t>(result.bytecode.size()));
        for (auto const& code : result.bytecode)
        {
            write(neonpack, to<uint16_t>(code));
        }

        // Write the shapes header
        neonpack.write(shapes_header, 4);
        write(neonpack, to<uint32_t>(pak.shapes.size()));

        for (auto const& shape : pak.shapes)
        {
            write(neonpack, shape.shape_id);
            write(neonpack, shape.palette_id);
        }

        // Write palettes header
        neonpack.write(palettes_header, 4);
        write(neonpack, to<uint32_t>(pak.palettes.size()));

        for (auto& palette : pak.palettes)
        {
            for (auto& entry : palette)
            {
                neonpack.write(reinterpret_cast<char*>(&entry.r), 1);
                neonpack.write(reinterpret_cast<char*>(&entry.g), 1);
                neonpack.write(reinterpret_cast<char*>(&entry.b), 1);
                neonpack.write(reinterpret_cast<char*>(&entry.a), 1);
            }
        }
        
        // Write palettes header
        neonpack.write(ui_palette_header, 4);
        write(neonpack, to<uint32_t>(data->ui_palette.size()));
        for (auto& entry : data->ui_palette)
        {
            neonpack.write(reinterpret_cast<char*>(&entry.r), 1);
            neonpack.write(reinterpret_cast<char*>(&entry.g), 1);
            neonpack.write(reinterpret_cast<char*>(&entry.b), 1);
            neonpack.write(reinterpret_cast<char*>(&entry.a), 1);
        }

        neonpack.close();

        serialize_neon_loc(file_path.parent_path(), pak.string_table, pak.words_table);
    }

    void serialize_neon_loc(std::filesystem::path file_path, std::vector<std::string> const& string_table, std::vector<neon_word_list> const& words_table)
    {
        // Save the default language pack (en)
        auto locpack_path = file_path / "lang/en.noir";
        locpack_path = locpack_path.make_preferred();
        fs::create_directories(locpack_path.parent_path());

        auto locpack = std::ofstream{ locpack_path, std::ios::binary | std::ios::trunc };

        if (!locpack)
        {
            throw std::runtime_error{ std::format("Could not open/create '{}'", locpack_path.string()) };
        }

        // Write the pack header
        auto header = loc_header{};
        header.version = 1;                             // Eventually this will be updated
        header.language = 1;                            // Eventually this will be a language code
        locpack.write(header.magic, 4);
        write(locpack, header.version);
        write(locpack, header.language);

        // Write the string table "header"
        locpack.write(string_header, 4);
        write(locpack, to<uint32_t>(string_table.size()));

        auto string_size = 0u;
        for (auto const& entry : string_table)
        {
            string_size += to<uint32_t>(entry.size());
            string_size += 4;
        }
        write(locpack, string_size);

        for (auto const& entry : string_table)
        {
            write(locpack, to<uint32_t>(entry.size()));
            locpack.write(entry.data(), entry.size());
        }

        // Write words header
        locpack.write(words_header, 4);
        write(locpack, to<uint32_t>(words_table.size()));

        auto words_size = 0u;
        for (auto const& entry : words_table)
        {
            words_size += 4; // size of word list
            words_size += to<uint32_t>(entry.words.size() * sizeof(neon_word));
        }
        write(locpack, words_size);

        for (auto const& entry : words_table)
        {
            write(locpack, to<uint32_t>(entry.words.size()));
            for (auto const& word : entry.words)
            {
                write(locpack, word.start_idx);
                write(locpack, word.end_idx);
            }
        }
    }
}