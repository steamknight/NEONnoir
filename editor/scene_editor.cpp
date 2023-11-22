#include "imgui.h"
#include "imgui_stdlib.h"
#include "IconsMaterialDesign.h"
#include "imgui_utils.h"
#include "glfw_utils.h"

#include <array>
#include <format>
#include <filesystem>

#include "scene_editor.h"
#include "utils.h"

namespace fs = std::filesystem;

namespace NEONnoir
{
    struct line_span
    {
        i32 begin_idx{ 0 }, end_idx{ 0 };
    };

    void scene_editor::display(game_data_location* location, game_data_scene* scene, asset_collection* assets)
    {
        if (location != nullptr && scene != nullptr)
        {
            _location = location;
            _scene = scene;
            _assets = assets;
            if (auto locations_window = ImGui_window(std::format("{}: {}###Scene", location->name, scene->name)))
            {
                display_editor();
            }
        }
    }

    void scene_editor::display_editor()
    {
        if (auto table = imgui::table("Layout", 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable))
        {
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            display_scene_properties();

            ImGui::TableNextColumn();
            display_scene();
        }
    }

    void scene_editor::display_scene_properties()
    {
        if (ImGui::BeginChild("SceneProps", { 0, 0 }, true))
        {
            if (auto props = imgui::table("PropsLayout", 2, ImGuiTableFlags_SizingStretchProp))
            {
                display_prop_string("Name", _scene->name);
                display_prop_descriptions("Description", _scene->description_id);
                display_prop_background();
                display_prop_string("On Enter", _scene->on_enter);
                display_prop_string("On Exit", _scene->on_exit);
                display_prop_checkbox("Is Cutscene", _scene->is_cutscene);
                //display_prop_int("Music ID", _scene->music_id);
                display_prop_music();
            }

            auto exit_regions = std::vector<std::string>{ "None" };
            for (auto const& scene : _location->scenes)
            {
                exit_regions.push_back(scene.name);
            }

            display_prop_regions(exit_regions);
            display_prop_text_regions();
        }
        
        ImGui::EndChild();
    }

    void scene_editor::display_prop_string(std::string_view const& label, std::string& value)
    {
        ImGui::TableNextRow();

        display_label(label.data());

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText(make_id("##{}", value), &value);
    }

    void scene_editor::display_prop_string_entry(std::string_view const& label, std::string& string_id)
    {
        if (string_id.empty())
        {
            string_id = _data->strings.create_string_entry("");
        }

        display_prop_string(label, _data->strings.get_string(string_id));
    }

    void scene_editor::display_prop_multistring(std::string_view const& label, std::string& value)
    {
        ImGui::TableNextRow();

        display_label(label.data());

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputTextMultiline(make_id("##{}", value), &value);
    }

    void scene_editor::display_prop_multistring_entry(std::string_view const& label, std::string& string_id)
    {
        if (string_id.empty())
        {
            string_id = _data->strings.create_string_entry("");
        }

        display_prop_multistring(label, _data->strings.get_string(string_id));
    }

    void scene_editor::display_prop_descriptions(std::string_view const& label, std::vector<std::string>& string_ids)
    {
        ImGui::TableNextRow();

        display_label(label.data());

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);

        if (ImGui::Button("Add description"))
        {
            string_ids.push_back({});
        }

        auto delete_index = std::optional<size_t>{ std::nullopt };
        auto count = 0;
        for (auto& string_id : string_ids)
        {
            ImGui::PushID(to<void*>(&string_id));
            if (DeleteButton("##delete"))
            {
                delete_index = count;
            }
            ImGui::SameLine();
            ImGui::SetNextItemWidth(-FLT_MIN);

            if (string_id.empty())
            {
                string_id = _data->strings.create_string_entry("");
            }
            ImGui::InputTextMultiline("##", &_data->strings.get_string(string_id), {0, 60});
            ImGui::PopID();
            
            count++;
        }

        if (delete_index)
        {
            auto const& text_id = string_ids[delete_index.value()];
            if (!text_id.empty())
            {
                _data->strings.remove_string(text_id);
            }

            string_ids.erase(string_ids.begin() + delete_index.value());
        }
    }

    void scene_editor::display_prop_int(std::string_view const& label, u16& value)
    {
        ImGui::TableNextRow();

        display_label(label.data());

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        int v = value;
        ImGui::InputInt(make_id("##{}", value), &v);
        value = to<u16>(v);
    }

    void scene_editor::display_prop_checkbox(std::string_view const& label, bool& value)
    {
        ImGui::TableNextRow();

        display_label(label.data());

        ImGui::TableNextColumn();
        ImGui::Checkbox(make_id("##{}", value), &value);
    }


    void scene_editor::display_prop_combo(std::string_view const& label, std::vector<std::string> const& values, u16& selected_value)
    {
        ImGui::TableNextRow();

        display_label(label.data());

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);

        selected_value++;
        ImGui::PushID((void*)&selected_value);
        if (ImGui::BeginCombo("##", values[selected_value].c_str()))
        {
            for (u16 n = 0; n < values.size(); n++)
            {
                auto const is_selected = selected_value == n;
                if (ImGui::Selectable(values[n].c_str(), is_selected))
                {
                    selected_value = n;
                }

                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopID();

        selected_value--;
    }

    void scene_editor::display_prop_enum(std::string_view const& label, std::vector<std::string> const& values, u16 & selected_value)
    {
        ImGui::TableNextRow();

        display_label(label.data());

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);

        ImGui::SliderInt(make_id("##{}", selected_value), (int*)&selected_value, 0, (int)(values.size() - 1), values[selected_value].c_str());
    }

    void scene_editor::display_prop_list(std::string_view const& label, std::vector<std::string>const& values, std::string& selected)
    {
        ImGui::TableNextRow();

        display_label(label.data());

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
 
        if (ImGui::BeginCombo("##ScriptCombo", "[NONE]"))
        {
            for (u16 index = 0; index < values.size(); index++)
            {
                auto const is_selected = values[index] == selected;

                if (ImGui::Selectable(values[index].c_str(), is_selected))
                {
                    selected = values[index];
                }

                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }
    }

    void scene_editor::display_label(std::string_view const& label)
    {
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label.data());
    }

    void scene_editor::display_prop_background()
    {
        display_label("Music");

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);

        auto& current_bg = _assets->backgrounds[_location->backgrounds[_scene->image_id]].name;
        
        if (ImGui::BeginCombo("##BackgroundCombo", current_bg.c_str()))
        {
            for (u16 index = 0; index < _location->backgrounds.size(); index++)
            {
                auto const is_selected = _scene->image_id == index;
                auto& bg_name = _assets->backgrounds[_location->backgrounds[index]].name;

                if (ImGui::Selectable(bg_name.c_str(), is_selected))
                {
                    _scene->image_id = index;
                }

                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }
    }

    void scene_editor::display_prop_music()
    {
        display_label("Music");

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);

        auto previous_song = std::string{ "<Previous Song>" };
        auto& current = (_scene->music_id != 0xFFFF)
            ? _assets->music[_scene->music_id].name
            : previous_song;
        
        if (ImGui::BeginCombo("##MusicCombo", current.c_str()))
        {
            bool any_selected = false;
            for (u16 index = 0; index < _assets->music.size(); index++)
            {
                auto const is_selected = _scene->music_id == index;
                auto& name = _assets->music[index].name;

                if (ImGui::Selectable(name.c_str(), is_selected))
                {
                    _scene->music_id = index;
                }

                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }

                any_selected |= is_selected;
            }

            if (ImGui::Selectable(previous_song.c_str(), !any_selected))
            {
                ImGui::SetItemDefaultFocus();
                _scene->music_id = 0xFFFF;
            }

            ImGui::EndCombo();
        }
    }

    void scene_editor::display_prop_regions(std::vector<std::string> const& exit_regions)
    {
        ImGui::NewLine();

        auto const origin = ImGui::GetCursorScreenPos();
        auto content_center = ImGui::GetColumnWidth() / 2.f;
        SetCursorCenteredText(origin + ImVec2{content_center, 0}, "Regions");
        ImGui::Text("%d Regions", _scene->regions.size());

        if (auto table = imgui::table("RegionsLayout", 2, ImGuiTableFlags_SizingStretchProp))
        {
            _selected_region_index = -1;
            auto region_to_delete = std::optional<size_t>{ std::nullopt };
            for (i32 index = 0; auto& region : _scene->regions)
            {
                auto region_id = imgui::push_id(&region);
                ImGui::TableNextRow();

                ImGui::BeginGroup();
                
                ImGui::TableNextColumn();
                ImGui::Text("Region %d", index);
                ImGui::TableNextColumn();
                if (DeleteButton("##{}"))
                {
                    region_to_delete = index;
                }
                ToolTip("Delete");

                ImGui::TableNextRow();
                display_prop_region_scalar("X", region.x);
                display_prop_region_scalar("Y", region.y);
                display_prop_region_scalar("Width", region.width);
                display_prop_region_scalar("Height", region.height);
                display_prop_string_entry("Hover Text", region.description_id);

                ImGui::TableNextRow();
                display_label("Mouse Pointer");
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                std::array pointer_options = {
                    imgui::button_option{ ICON_MD_NORTH_WEST,   "Ponter",   to<u16>(0)   },
                    imgui::button_option{ ICON_MD_SEARCH,       "Examine",  to<u16>(1)   },
                    imgui::button_option{ ICON_MD_PAN_TOOL_ALT, "Use",      to<u16>(2)   },
                    imgui::button_option{ ICON_MD_LOGOUT,       "Exit",     to<u16>(3)   },
                };
                region.pointer_id = imgui::button_combo(pointer_options, region.pointer_id);

                display_prop_combo("Exit To Scene", exit_regions, region.goto_scene);
                display_prop_string("Script Name", region.script);

                ImGui::NewLine();
                
                ImGui::EndGroup();

                if (ImGui::IsItemHovered())
                {
                    _selected_region_index = index;
                }

                index++;
            }

            if (region_to_delete)
            {
                auto const& region = _scene->regions[region_to_delete.value()];
                if (!region.description_id.empty())
                {
                    _data->strings.remove_string(region.description_id);
                }

                _scene->regions.erase(_scene->regions.begin() + region_to_delete.value());
            }
        }
    }

    void scene_editor::display_prop_region_scalar(std::string_view const& label, u16& value)
    {
        display_label(label.data());

        ImGui::TableNextColumn();
        u16 const step_size = 1;
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputScalar(make_id("##{}", value), ImGuiDataType_U16, &value, &step_size, nullptr, "%u");
    }

    void scene_editor::display_prop_region_scalar(std::string_view const& label, float& value)
    {
        display_label(label.data());

        ImGui::TableNextColumn();
        float const step_size = 1.f;
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputScalar(make_id("##{}", value), ImGuiDataType_Float, &value, &step_size, nullptr, "%.0f");
    }

    void scene_editor::display_prop_text_regions()
    {
        ImGui::NewLine();

        auto const origin = ImGui::GetCursorScreenPos();
        auto content_center = ImGui::GetColumnWidth() / 2.f;
        SetCursorCenteredText(origin + ImVec2{ content_center, 0 }, "Text Regions");
        ImGui::Text("%d Text Regions", _scene->text_regions.size());

        if (auto table = imgui::table("text_region_layout", 2, ImGuiTableFlags_SizingStretchProp))
        {
            _selected_text_region_index = -1;
            auto region_to_delete = std::optional<size_t>{ std::nullopt };

            for (size_t index = 0; auto & text_region : _scene->text_regions)
            {
                ImGui::TableNextRow();

                auto id = imgui::push_id(&text_region);
                ImGui::BeginGroup();

                ImGui::TableNextColumn();
                ImGui::Text("Text Region %d", index);

                ImGui::TableNextColumn();
                if (DeleteButton("##"))
                {
                    region_to_delete = index;
                }

                ImGui::SameLine();
                std::array justify_options = {
                    imgui::button_option{ ICON_MD_FORMAT_ALIGN_LEFT,    "Left Justify",     textbox_justification::left     },
                    imgui::button_option{ ICON_MD_FORMAT_ALIGN_CENTER,  "Center",           textbox_justification::center   },
                    imgui::button_option{ ICON_MD_FORMAT_ALIGN_RIGHT,   "Right Justify",    textbox_justification::right    },
                };

                text_region.justification = imgui::button_combo(justify_options, text_region.justification);

                ImGui::SameLine();
                if (imgui::toggle_button(ICON_MD_VERTICAL_ALIGN_CENTER, "Align Vertically", text_region.center_vertically))
                {
                    text_region.center_vertically = !text_region.center_vertically;
                }

                ImGui::TableNextRow();

                display_prop_region_scalar("X", text_region.x);
                display_prop_region_scalar("Y", text_region.y);
                display_prop_region_scalar("Width", text_region.width);
                display_prop_multistring_entry("Text", text_region.text_id);

                ImGui::NewLine();
                ImGui::EndGroup();

                index++;
            }

            if (region_to_delete)
            {
                auto const& region = _scene->text_regions[region_to_delete.value()];
                if (!region.text_id.empty())
                {
                    _data->strings.remove_string(region.text_id);
                }

                _scene->text_regions.erase(_scene->text_regions.begin() + region_to_delete.value());
            }
        }
    }

    void scene_editor::display_scene()
    {
        display_scene_toolbar();

        ImGuiIO& io = ImGui::GetIO(); (void)io;

        auto texture = _assets->backgrounds[_location->backgrounds[_scene->image_id]].texture;
        ImGui::Image((void*)(intptr_t)texture.texture_id, ImVec2((float)(texture.width * _zoom), (float)(texture.height * _zoom)));

        auto image_min = ImGui::GetItemRectMin();
        bool image_hovered = ImGui::IsItemHovered();
        if (image_hovered)
        {
            _last_mouse = (io.MousePos - image_min) / _zoom;
        }

        auto draw_list = ImGui::GetWindowDrawList();

        if (_add_region_mode)
        {
            glfwSetCursor(_main_window, _crosshair_cursor.get());
            if (io.MouseDown[0] && _last_mouse.x > 0 && _last_mouse.y > 0)
            {
                if (_add_region_p0.x < 0 && _add_region_p0.y < 0)
                {
                    _add_region_p0 = io.MousePos;
                    _add_region_dragging = true;
                }
            }
            if (io.MouseReleased[0] && _add_region_dragging)
            {
                auto p0 = (_add_region_p0 - image_min) / _zoom;
                auto p1 = (io.MousePos - image_min) / _zoom;

                auto p_min = ImVec2
                {
                    std::min(p0.x, p1.x),
                    std::min(p0.y, p1.y)
                };

                auto size = ImVec2
                {
                    std::fabsf(p1.x - p0.x),
                    std::fabsf(p1.y - p0.y)
                };

                // Sort the points so we don't get wonky regions
                auto region = game_data_region{
                    .x = to<u16>(p_min.x),
                    .y = to<u16>(p_min.y),
                    .width = to<u16>(size.x),
                    .height = to<u16>(size.y),
                    .shape_id = NO_INDEX,
                    .pointer_id = to<u16>(0), 
                    .goto_scene = NO_INDEX,
                    .description_id = "", 
                    .script = ""
                };

                _scene->regions.push_back(region);
                _add_region_mode = false;
                _add_region_dragging = false;
                _add_region_p0 = { -1, -1 };
                glfwSetCursor(_main_window, nullptr);
            }

            if (_add_region_dragging)
            {
                draw_list->AddRectFilled(_add_region_p0, io.MousePos, IM_COL32(255, 165, 0, 64));
                draw_list->AddRect(_add_region_p0, io.MousePos, IM_COL32(255, 165, 0, 255), 0.f, 0, 2.f);
            }
        }
        else if (_add_text_mode && image_hovered)
        {
            glfwSetCursor(_main_window, _crosshair_cursor.get());

            if (io.MouseReleased[0] && _last_mouse.x > 0 && _last_mouse.y > 0)
            {
                auto location = (io.MousePos - image_min) / _zoom;

                auto text_region = game_data_text_region{
                    .x = location.x,
                    .y = location.y,
                    .width = 1
                };

                _scene->text_regions.push_back(text_region);
                _add_text_mode = false;
                glfwSetCursor(_main_window, nullptr);
            }
        }

        for (auto index = 0; auto const& region : _scene->regions)
        {
            auto p0 = ImVec2{ to<float>(region.x), to<float>(region.y) } * _zoom + image_min;
            auto p1 = ImVec2{ to<float>(region.x + region.width), to<float>(region.y + region.height) } * _zoom + image_min;

            if (_selected_region_index == index)
            {
                draw_list->AddRectFilled(p0, p1, IM_COL32(255, 165, 0, 64));
                draw_list->AddRect(p0, p1, IM_COL32(255, 165, 0, 255), 0.f, 0, 2.f);
            }
            else
            {
                draw_list->AddRectFilled(p0, p1, IM_COL32(13, 129, 255, 64));
                draw_list->AddRect(p0, p1, IM_COL32(13, 129, 255, 255), 0.f, 0, 2.f);
            }

            index++;
        }

        for (auto& text_region : _scene->text_regions)
        {
            auto p0 = ImVec2
            {
                text_region.x,
                text_region.center_vertically ? 0.f : text_region.y
            } * _zoom + image_min;

            auto p1 = ImVec2
            {
                text_region.x + _data->default_font.width,
                (text_region.center_vertically ? 0.f : text_region.y) + _data->default_font.height,
            } * _zoom + image_min;

            auto char_size = ImVec2
            {
                _data->default_font.width * _zoom,
                _data->default_font.height * _zoom
            };
            size_t line_count{ 1 };
            if (!text_region.text_id.empty())
            {
                auto char_height = to<float>(_data->default_font.height * _zoom);
                auto const text = std::string_view{ _data->strings.get_string_latin1(text_region.text_id) };

                auto current_p0 = p0;
                auto current_p1 = p1;

                i32 space_remaining = to<i32>(text_region.width);
                auto const& words = _data->strings.get_words_latin1(text_region.text_id);
                auto lines = std::vector<line_span>{};
                auto line = std::optional<line_span>{ std::nullopt };
                for (auto const& word : words)
                {
                    i32 word_length = (word.end_idx - word.begin_idx + 1);

                    if (!line)
                    {
                        line = line_span{ word.begin_idx, word.end_idx };
                    }
 
                    if (word_length == 0 || (space_remaining - word_length) < 0)
                    {
                        lines.push_back(line.value());
                        
                        if (word_length > 0)
                        {
                            line = line_span{ word.begin_idx, word.end_idx };
                        }
                        else
                        {
                            line = std::nullopt;
                        }

                        space_remaining = to<i32>(text_region.width);
                        line_count++;
                    }

                    if (word_length > 0)
                    {
                        line.value().end_idx = word.end_idx;
                        space_remaining -= (word_length + 1);
                    }
                }
                if (line && words.size() > 0)
                {
                    lines.push_back(line.value());
                }

                if (text_region.center_vertically)
                {
                    auto text_height = line_count * _data->default_font.height;
                    current_p0.y = ((texture.height - text_height) / 2.f) * _zoom + image_min.y;
                    current_p1.y = current_p0.y + (_data->default_font.height * _zoom);
                }

                for (auto const current_line : lines)
                {
                    auto line_length = current_line.end_idx - current_line.begin_idx + 1;
                    auto offset = to<float>(text_region.width - line_length) * _data->default_font.width * _zoom;

                    switch (text_region.justification)
                    {
                        case textbox_justification::left: 
                            // Do nothing
                            break;
                        case textbox_justification::center:
                            current_p0.x += (offset / 2.f);
                            current_p1.x += (offset / 2.f);
                            break;
                        case textbox_justification::right:
                            current_p0.x += offset;
                            current_p1.x += offset;
                            break;
                    }
                    print_line(text.substr(current_line.begin_idx, line_length), draw_list, current_p0, current_p1);

                    current_p0.x = p0.x; current_p1.x = p1.x;
                    current_p0.y += char_height;
                    current_p1.y += char_height;
                }
            }

            auto rect_p1 = ImVec2
            {
                text_region.width * _data->default_font.width + text_region.x,
                text_region.center_vertically ? texture.height : (_data->default_font.height * line_count) + text_region.y,
            } *_zoom + image_min;

            draw_list->AddRectFilled(p0, rect_p1, IM_COL32(129, 13, 255, 64));
            draw_list->AddRect(p0, rect_p1, IM_COL32(129, 13, 255, 255), 0.f, 0, 2.f);
        }
    }

    void scene_editor::print_line(std::string_view const& line, ImDrawList* draw_list, ImVec2& p0, ImVec2& p1)
    {
        auto char_width = to<float>(_data->default_font.width * _zoom);
        auto char_size_uv = ImVec2
        {
            _data->default_font.width / to<float>(_data->default_font.texture.width),
            _data->default_font.height / to<float>(_data->default_font.texture.height),
        };

        for (auto ch : line)
        {
            // We ignore the first 32 characters because they're not visibile and therefore not part of the font.
            ch -= 32;

            auto char_uv = ImVec2
            {
                to<float>(ch % 32),
                to<float>(ch / 32)
            };
            char_uv = char_uv * char_size_uv;

            draw_list->AddImage(_data->default_font.texture, p0, p1, char_uv, char_uv + char_size_uv);
            p0.x += char_width;
            p1.x += char_width;
        }
    }

    void scene_editor::display_scene_toolbar() noexcept
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));

        auto button_size = ImVec2{ 128, 0 };
        if (ImGui::SmallButton(ICON_MD_CROP))
        {
            _add_region_mode = true;
        }
        ToolTip("Add Region");

        ImGui::SameLine();
        if (ImGui::SmallButton(ICON_MD_DESCRIPTION))
        {
            _add_text_mode = true;
        }
        ToolTip("Add Text Region");

        ImGui::SameLine();
        if (ImGui::SmallButton(ICON_MD_ZOOM_IN))
        {
            _zoom++;
        }
        ToolTip("Zoom In");

        ImGui::SameLine();
        if (ImGui::SmallButton(ICON_MD_ZOOM_OUT) && _zoom > 1)
        {
            _zoom--;
        };
        ToolTip("Zoom Out");

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui::Text(ICON_MD_NEAR_ME " %.0f, %.0f\t" ICON_MD_ZOOM_IN " %d00%%", _last_mouse.x, _last_mouse.y, _zoom);

        ImGui::PopStyleColor();
    }
}