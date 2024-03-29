#include <fstream>
#include <filesystem>
#include <format>

#include "utils.h"
#include "packfile.h"

namespace fs = std::filesystem;

namespace NEONnoir
{
    u16 get_script_offset(std::string const& script_name, assembler_result const& result)
    {
        if (script_name == "")
        {
            return NO_INDEX;
        }

        if (result.scripts_meta.count(script_name) == 0)
        {
            throw packer_error(std::format("Reference to non-existing script '{}'", script_name));
        }

        return result.scripts_meta.at(script_name).start;
    }

    u16 get_flag_id(std::string const& flag, assembler_result const& result)
    {
        if (result.flags.count(flag) == 0)
        {
            throw packer_error(std::format("Reference to non-existing flag '{}'", flag));
        }

        return result.flags.at(flag);
    }

    u16 get_const_id(std::string const& constant, assembler_result const& result)
    {
        if (result.constants.count(constant) == 0)
        {
            throw packer_error(std::format("Reference to non-existing constant '{}'", constant));
        }

        return result.constants.at(constant).value;
    }

    std::string to_portable_path_string(fs::path const& path)
    {
        auto portable_path = path.string();
        std::replace(portable_path.begin(), portable_path.end(), '\\', '/');

        return portable_path;
    }

    neon_packfile generate_packfile(std::shared_ptr<game_data> const& data, assembler_result const& result)
    {
        auto pak = neon_packfile{};

        // Prepare the asset manifest
        pak.manifest.ui_count = to<u32>(data->manifest.assets.ui.size());
        for (auto const& ui : data->manifest.assets.ui)
        {
            pak.manifest.assets.push_back(to_portable_path_string(ui.relative_path));
        }

        pak.manifest.bg_count = to<u32>(data->manifest.assets.backgrounds.size());
        for (auto const& bg: data->manifest.assets.backgrounds)
        {
            pak.manifest.assets.push_back(to_portable_path_string(bg.relative_path));
        }

        pak.manifest.mus_count = to<u32>(data->manifest.assets.music.size());
        for (auto const& mus : data->manifest.assets.music)
        {
            pak.manifest.assets.push_back(to_portable_path_string(mus.relative_path));
        }

        pak.manifest.sfx_count = to<u32>(data->manifest.assets.sfx.size());
        for (auto const& sfx : data->manifest.assets.sfx)
        {
            pak.manifest.assets.push_back(to_portable_path_string(sfx.relative_path));
        }

        // Before any other palettes, lets put the speaker palettes
        for (auto const& speaker : data->speakers)
        {
            pak.palettes.push_back(speaker.image.color_palette);
        }
        
        for (auto const& string_entry : data->strings.entries)
        {
            pak.string_table.push_back(data->strings.get_string_latin1(string_entry.id));
        }

        for (auto const& location : data->locations)
        {
            auto loc = neon_location{};
            loc.name_id = to<u16>(pak.string_table.size());

            // TODO do something about this
            //pak.string_table.push_back(UTF8toISO8859_1(location.name.c_str()));

            auto bg_count = to<u16>(std::min(to<u16>(8), to<u16>(location.backgrounds.size())));
            for (u16 bg_id = 0; bg_id < bg_count; bg_id++)
            {
                loc.backgrounds[bg_id] = to<u16>(pak.manifest.xform_bg_id(location.backgrounds[bg_id]));
            }

            loc.first_scene_id = to<u16>(pak.scenes.size());
            loc.last_scene_id = loc.first_scene_id + to<u16>(location.scenes.size()) - 1;

            for (auto const& scene : location.scenes)
            {
                auto s = neon_scene{};
                s.name_id = to<u16>(pak.string_table.size());

                // TODO and something about this
                //pak.string_table.push_back(UTF8toISO8859_1(scene.name.c_str()));

                if (scene.description_id.size() > 0)
                {
                    s.first_desc_id = to<u16>(data->strings.get_string_index(scene.description_id[0]));
                    s.last_desc_id = to<u16>(s.first_desc_id + scene.description_id.size() - 1);
                }

                s.background_id = scene.image_id;

                s.first_region_id = to<u16>(pak.regions.size());
                s.last_region_id = s.first_region_id + to<u16>(scene.regions.size()) - 1;

                s.first_text_id = to<u16>(pak.text_regions.size());
                s.last_text_id = s.first_text_id + to<u16>(scene.text_regions.size()) - 1;

                s.on_enter = get_script_offset(scene.on_enter, result);
                s.on_exit = get_script_offset(scene.on_exit, result);

                s.music_id = pak.manifest.xform_mus_id(scene.music_id);
                s.is_cutscene = scene.is_cutscene ? -1 : 0;

                for (auto const& region : scene.regions)
                {
                    auto r = neon_region
                    {
                        .x1 = region.x,
                        .y1 = region.y,
                        .x2 = to<u16>(region.x + region.width),
                        .y2 = to<u16>(region.y + region.height),
                        .pointer_id = region.pointer_id,
                        .goto_scene = region.goto_scene,
                    };

                    if (!region.description_id.empty() && data->strings.get_string(region.description_id).size() > 0)
                    {
                        r.description_id = to<u16>(data->strings.get_string_index(region.description_id));
                    }
                    else
                    {
                        r.description_id = NO_INDEX;
                    }

                    if (region.script != "")
                    {
                        if (result.scripts_meta.count(region.script) == 0)
                        {
                            throw packer_error(std::format("Region '{}/{}/{}' references non-existing script '{}'", location.name, scene.name, data->strings.get_string(region.description_id), region.script));
                        }
                        else
                        {
                            r.script_offset = result.scripts_meta.at(region.script).start;
                        }
                    }
                    else
                    {
                        r.script_offset = NO_INDEX;
                    }

                    pak.regions.push_back(r);
                }

                for (auto const& text_region : scene.text_regions)
                {
                    auto t = neon_text_region
                    {
                        .x = to<u16>(text_region.x),
                        .y = to<u16>(text_region.y),
                        .width = to<u16>(text_region.width),
                        .justify = to<char>(text_region.justification),
                        .center_vertical = to<char>(text_region.center_vertically ? - 1 : 0)
                    };

                    if (!text_region.text_id.empty() && data->strings.get_string(text_region.text_id).size() > 0)
                    {
                        t.text_id = to<u16>(data->strings.get_string_index(text_region.text_id));
                    }

                    pak.text_regions.push_back(t);
                }

                pak.scenes.push_back(s);
            }

            if (location.shapes.size() > 0)
            {
                loc.first_shape_id = to<u16>(pak.shapes.size());
                for (auto const& container : location.shapes)
                {
                    for (auto shape_id = to<u16>(data->shape_start_id); shape_id < container.shapes.size(); shape_id++)
                    {
                        pak.shapes.push_back({ shape_id, to<u16>(pak.palettes.size()) });
                    }

                    pak.palettes.push_back(container.image.color_palette);
                }
                loc.last_shape_id = to<u16>(pak.shapes.size()) - 1; // compensate for the extra +1 at the end of loop

                if (location.shapes_file == "")
                {
                    throw std::runtime_error("Missing shapes file name.");
                }
                loc.shapes_file = to<u16>(pak.string_table.size());
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

            d.first_page_id = to<u16>(pak.pages.size());
            auto page_count = 0;
            for (auto const& page : dialogue.pages)
            {
                auto p = neon_page{};
                p.speaker_id = page.speaker_id;
                p.text_id = to<u16>(data->strings.get_string_index(page.text_id));

                p.page_id = page.next_page_id;
                if (p.page_id != NO_INDEX) p.page_id += d.first_page_id;

                p.first_choice_id = (page.choices.size() > 0) ? to<u16>(pak.choices.size()) : NO_INDEX;
                auto choice_count = 0;
                for (auto const& choice : page.choices)
                {
                    auto c = neon_choice{};
                    c.text_id = to<u16>(data->strings.get_string_index(choice.text_id));
                    pak.string_table[c.text_id] = "*" + pak.string_table[c.text_id];

                    c.page_id = choice.next_page_id;
                    if (c.page_id != NO_INDEX) c.page_id += d.first_page_id;

                    c.script_offset = !choice.has_script
                        ? NO_INDEX
                        : get_script_offset(choice.script, result);

                    c.set_flag = !choice.has_set_flag
                        ? NO_INDEX
                        : get_flag_id(choice.set_flag, result);

                    c.clear_flag = !choice.has_clear_flag
                        ? NO_INDEX
                        : get_flag_id(choice.clear_flag, result);

                    c.check_flag = !choice.has_check_flag
                        ? NO_INDEX
                        : get_flag_id(choice.check_flag, result);

                    c.enabled = choice.enabled ? 0xFF : 0;

                    c.self_disable = choice.self_disable ? 0xFF : 0;

                    pak.choices.push_back(c);

                    choice_count++;
                }

                p.set_flag = !page.has_set_flag
                    ? NO_INDEX
                    : get_flag_id(page.set_flag, result);

                p.clear_flag = !page.has_clear_flag
                    ? NO_INDEX
                    : get_flag_id(page.clear_flag, result);

                p.check_flag = !page.has_check_flag
                    ? NO_INDEX
                    : get_flag_id(page.check_flag, result);

                p.choice_count = to<u16>(page.choices.size());
                p.enabled = page.enabled ? 0xFF : 0;
                p.self_disable = page.self_disable ? 0xFF : 0;
                
                pak.pages.push_back(p);

                page_count++;
            }

            d.page_count = to<u16>(dialogue.pages.size());
            pak.dialogues.push_back(d);
        }

        // Calculate all words
        for (auto const& text : pak.string_table)
        {
            auto words = string_table::calculate_words(text);

            auto word_list = neon_word_list{};
            word_list.word_count = to<u16>(words.size());
            word_list.words.reserve(word_list.word_count);

            std::ranges::transform(words, std::back_inserter(word_list.words), [](auto& word) -> neon_word { return { to<u16>(word.begin_idx), to<u16>(word.end_idx) }; });

            pak.words_table.push_back(word_list);
        }

        return pak;
    }

    void serialize_asset_group(std::ofstream& neonpack, std::vector<game_asset> const& assets)
    {
        auto path_size = 0u;
        for (auto const& entry : assets)
        {
            path_size += to<u32>(entry.relative_path.string().size());
            path_size += 4;
        }
        write(neonpack, path_size);

        for (auto const& entry : assets)
        {
            write(neonpack, to<u32>(entry.relative_path.string().size()));
            neonpack.write(entry.relative_path.string().data(), entry.relative_path.string().size());
        }
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

        // Write the file manifest
        neonpack.write(manifest_header, 4);
        write(neonpack, to<u32>(pak.manifest.assets.size()));

        auto total_asset_path_size = 0u;
        for (auto const& entry : pak.manifest.assets)
        {
            total_asset_path_size += to<u32>(entry.size());
            total_asset_path_size += 4;
        }
        write(neonpack, total_asset_path_size);

        for (auto const& entry : pak.manifest.assets)
        {
            write(neonpack, to<u32>(entry.size()));
            neonpack.write(entry.data(), entry.size());
        }

        // Write all locations
        neonpack.write(locations_header, 4);
        write(neonpack, to<u32>(pak.locations.size()));
        for (auto const& location: pak.locations)
        {
            write(neonpack, location.name_id);

            for (auto const& bg_id : location.backgrounds)
            {
                write(neonpack, bg_id);
            }

            write(neonpack, location.first_scene_id);
            write(neonpack, location.last_scene_id);
            write(neonpack, location.first_shape_id);
            write(neonpack, location.last_shape_id);
            write(neonpack, location.shapes_file);

            for (auto const& speaker : location.speakers)
            {
                write(neonpack, speaker);
            }
        }

        // Write all scenes
        neonpack.write(scenes_header, 4);
        write(neonpack, to<u32>(pak.scenes.size()));
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
            write(neonpack, scene.first_text_id);
            write(neonpack, scene.last_text_id);
            write(neonpack, scene.music_id);
            neonpack.write(&scene.is_cutscene, 1);
            neonpack.write(&scene.padding, 1);
        }

        // Write all regions
        neonpack.write(regions_header, 4);
        write(neonpack, to<u32>(pak.regions.size()));
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

        // Write all text regions
        neonpack.write(text_regions_header, 4);
        write(neonpack, to<u32>(pak.text_regions.size()));
        for (auto const& text_region : pak.text_regions)
        {
            write(neonpack, text_region.x);
            write(neonpack, text_region.y);
            write(neonpack, text_region.width);
            write(neonpack, text_region.text_id);
            neonpack.write(&text_region.justify, 1);
            neonpack.write(&text_region.center_vertical, 1);
        }

        // Dialogues
        neonpack.write(dialogues_header, 4);
        write(neonpack, to<u32>(pak.dialogues.size()));

        for (auto const& dialogue : pak.dialogues)
        {
            write(neonpack, dialogue.first_page_id);
            write(neonpack, dialogue.page_count);
        }

        // Pages
        neonpack.write(pages_header, 4);
        write(neonpack, to<u32>(pak.pages.size()));

        for (auto const& page : pak.pages)
        {
            write(neonpack, page.speaker_id);
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
        write(neonpack, to<u32>(pak.choices.size()));

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
        write(neonpack, to<u32>(result.bytecode.size()));
        for (auto const& code : result.bytecode)
        {
            write(neonpack, to<u16>(code));
        }

        // Write the shapes header
        neonpack.write(shapes_header, 4);
        write(neonpack, to<u32>(pak.shapes.size()));

        for (auto const& shape : pak.shapes)
        {
            write(neonpack, shape.shape_id);
            write(neonpack, shape.palette_id);
        }

        // Write palettes header
        neonpack.write(palettes_header, 4);
        write(neonpack, to<u32>(pak.palettes.size()));

        for (auto& palette : pak.palettes)
        {
            for (auto& entry : palette)
            {
                neonpack.write(force_to<char*>(&entry.r), 1);
                neonpack.write(force_to<char*>(&entry.g), 1);
                neonpack.write(force_to<char*>(&entry.b), 1);
                neonpack.write(force_to<char*>(&entry.a), 1);
            }
        }
        
        // Write palettes header
        neonpack.write(ui_palette_header, 4);
        write(neonpack, to<u32>(data->ui_palette.size()));
        for (auto& entry : data->ui_palette)
        {
            neonpack.write(force_to<char*>(&entry.r), 1);
            neonpack.write(force_to<char*>(&entry.g), 1);
            neonpack.write(force_to<char*>(&entry.b), 1);
            neonpack.write(force_to<char*>(&entry.a), 1);
        }

        // Write speakers header
        neonpack.write(speakers_header, 4);
        write(neonpack, to<u32>(data->speakers.size()));

        // Write speaker offset and sizes
        auto offset = 0u;
        for (auto const& speaker : pak.speakers)
        {
            // Write offset
            write(neonpack, offset);

            // Write size
            auto const size = to<u32>(speaker.get_size());
            write(neonpack, size);

            offset += size;
        }

        // Write speaker shapes data
        for (auto const& speaker : pak.speakers)
        {
            // Write the header
            write(neonpack, speaker.width);
            write(neonpack, speaker.height);
            write(neonpack, speaker.bit_depth);
            write(neonpack, speaker.ebwidth);
            write(neonpack, speaker.blitsize);
            write(neonpack, speaker.handle_x);
            write(neonpack, speaker.handle_y);
            write(neonpack, speaker.data_ptr);
            write(neonpack, speaker.cookie_ptr);
            write(neonpack, speaker.onebpmem);
            write(neonpack, speaker.onebpmemx);
            write(neonpack, speaker.allbpmem);
            write(neonpack, speaker.allbpmemx);
            write(neonpack, speaker.padding);

            // Write the shape bypes
            neonpack.write(force_to<char const*>(&speaker.data[0]), speaker.data.size());
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
        write(locpack, to<u32>(string_table.size()));

        auto string_size = 0u;
        for (auto const& entry : string_table)
        {
            string_size += to<u32>(entry.size());
            string_size += 4;
        }
        write(locpack, string_size);

        for (auto const& entry : string_table)
        {
            write(locpack, to<u32>(entry.size()));
            locpack.write(entry.data(), entry.size());
        }

        // Write words header
        locpack.write(words_header, 4);
        write(locpack, to<u32>(words_table.size()));

        auto words_size = 0u;
        for (auto const& entry : words_table)
        {
            words_size += 4; // size of word list
            words_size += to<u32>(entry.words.size() * sizeof(neon_word));
        }
        write(locpack, words_size);

        for (auto const& entry : words_table)
        {
            write(locpack, to<u32>(entry.words.size()));
            for (auto const& word : entry.words)
            {
                write(locpack, word.start_idx);
                write(locpack, word.end_idx);
            }
        }
    }
}