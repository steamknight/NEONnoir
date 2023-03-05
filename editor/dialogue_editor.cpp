#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_utils.h"

#include <format>
#include <optional>

#include "dialogue_editor.h"
#include "utils.h"

namespace NEONnoir
{
    using optional_index = std::optional<size_t>;

    void dialogue_editor::display_editor()
    {
        // Add dialogue button
        if (ImGui::Button(ICON_MD_ADD_COMMENT " Create a new dialogue"))
        {
            _data->dialogues.push_back({});
        }

        auto merge_index = optional_index{ std::nullopt };
        auto delete_index = optional_index{ std::nullopt };
        auto count = 0;
        size_t page_count = 0;

        // Display all the dialogues
        for (auto& dialogue : _data->dialogues)
        {
            if(ImGui::CollapsingHeader(std::format(("{}: {}###dialogue{}"), count, dialogue.name, (size_t)&dialogue).c_str()))
            {
                auto action = display_dialogue(dialogue, page_count);
                if (action == dialogue_editor::dialogue_action_type::merge_down)
                {
                    merge_index = count;
                }
                else if (action == dialogue_editor::dialogue_action_type::delete_dialogue)
                {
                    delete_index = count;
                }
            }

            count++;
            page_count += dialogue.pages.size();
        }

        // Do nothing if it's the last one
        if (merge_index && merge_index.value() < _data->dialogues.size() - 1)
        {
            auto top_index = merge_index.value();
            auto bottom_index = top_index + 1;
            u16 last_page_index = to<u16>(_data->dialogues[top_index].pages.size());

            for (auto& page : _data->dialogues[bottom_index].pages)
            {
                // Update the links between pages and choices
                if (page.next_page_id != NO_INDEX) page.next_page_id += last_page_index;

                for (auto& choice : page.choices)
                {
                    if (choice.next_page_id != NO_INDEX) choice.next_page_id += last_page_index;
                }

                _data->dialogues[top_index].pages.push_back(page);
            }

            delete_index = bottom_index;
        }

        if (delete_index)
        {
            _data->dialogues.erase(_data->dialogues.begin() + delete_index.value());
        }
    }

    dialogue_editor::dialogue_action_type dialogue_editor::display_dialogue(dialogue& dialogue, size_t page_start_id)
    {
        auto request_action = display_dialogue_options(dialogue);

        auto id = imgui::push_id(&dialogue);

        display_pages(dialogue, page_start_id);

        // Add Page
        if (ImGui::Button(make_id(ICON_MD_ADD_COMMENT " Add Page##{}", dialogue), { 300, 0 }))
        {
            dialogue.pages.push_back({});
        }

        return request_action;
    }

    dialogue_editor::dialogue_action_type dialogue_editor::display_dialogue_options(dialogue& dialogue)
    {
        auto request_action = dialogue_editor::dialogue_action_type::none;

        // Dialogue name
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Name:");
        ImGui::SameLine();

        ImGui::InputText(make_id("##{}", dialogue.name), &dialogue.name);
        ImGui::SameLine();

        if (dialogue.pages.size() > 0)
        {
            if (ImGui::Button(make_id(ICON_MD_PERSON "##default_speaker{}", dialogue)))
            {
                for (auto& page : dialogue.pages)
                {
                    page.speaker_id = dialogue.pages[0].speaker_id;
                }
            }
            ToolTip("Set all speakers to the same value as Page 0");
            ImGui::SameLine();
        }

        if (ImGui::Button(make_id(ICON_MD_ARROW_DOWNWARD "##merge_down{}", dialogue)))
        {
            request_action = dialogue_editor::dialogue_action_type::merge_down;
            ToolTip("Merge this dialogue down");
        }
        ImGui::SameLine();

        // Delete
        if (DeleteButton(make_id("##DialogueDelete{}",dialogue)))
        {
            request_action = dialogue_editor::dialogue_action_type::delete_dialogue;
        }

        return request_action;
    }

    void dialogue_editor::input_text(std::string_view const& label, std::string& value)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label.data());

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);

        auto size = ImGui::CalcTextSize(value.c_str());
        size.x = -FLT_MIN;
        size.y += 8;
        ImGui::InputTextMultiline(make_id("##{}", value), & value, size);
    }

    void dialogue_editor::display_pages(dialogue& dialogue, size_t page_start_id)
    {
        ImGui::TableNextColumn();
        auto create_index = optional_index{ std::nullopt };
        auto delete_index = optional_index{ std::nullopt };

        if (auto table = imgui::table(make_id("Pages", dialogue.pages), 1, ImGuiTableFlags_BordersInnerH))
        {
            auto count = 0;
            for (auto& page : dialogue.pages)
            {
                auto action = display_page(page, count, page_start_id);
                if (action == page_action_type::create_above)
                {
                    create_index = count;
                }

                if (action == page_action_type::delete_page)
                {
                    delete_index = count;
                }

                count++;
            }
        }

        if (create_index)
        {
            auto const begin = dialogue.pages.begin() + create_index.value();
            dialogue.pages.insert(begin, dialogue_page{});

            // Need to refresh the iterator
            auto const insert_begin = dialogue.pages.begin() + create_index.value() + 1;
            std::for_each(insert_begin, dialogue.pages.end(), [](dialogue_page& page)
                {
                    if (page.next_page_id != NO_INDEX)
                        page.next_page_id++;

                    for (auto& choice : page.choices)
                    {
                        if (choice.next_page_id != NO_INDEX) choice.next_page_id += 1;
                    }
                });
        }

        if (delete_index)
        {
            auto const& page = dialogue.pages[delete_index.value()];
            if (!page.text_id.empty())
            {
                _data->strings.remove_string(page.text_id);
            }

            for (auto const& choice : page.choices)
            {
                if (!choice.text_id.empty())
                {
                    _data->strings.remove_string(choice.text_id);
                }
            }

            dialogue.pages.erase(dialogue.pages.begin() + delete_index.value());

            std::for_each(dialogue.pages.begin() + delete_index.value(), dialogue.pages.end(),
                [](dialogue_page& page)
                {
                    if (page.next_page_id != NO_INDEX)
                    {
                        page.next_page_id--;
                    }

                    for (auto& choice : page.choices)
                    {
                        if (choice.next_page_id != NO_INDEX) choice.next_page_id -= 1;
                    }
                }
            );
            
        }
    }

    dialogue_editor::page_action_type dialogue_editor::display_page(dialogue_page& page, size_t count, size_t page_start_id)
    {
        auto action = page_action_type::none;
        auto& speakers = get_speaker_list(_data->speakers);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        if (auto table = imgui::table(make_id("##Page{}", page), 3, ImGuiTableFlags_SizingStretchProp))
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Page %d", count);

            ImGui::TableNextColumn();

            ImGui::SetNextItemWidth(ImGui::CalcTextSize(speakers[0].c_str()).x * 4.f);
            imgui::combo_with_empty(speakers, page.speaker_id);
            ImGui::SameLine();

            ImGui::SetNextItemWidth(-FLT_MIN);

            if (page.text_id.empty())
            {
                page.text_id = _data->strings.create_string_entry("");
            }

            auto size = ImGui::CalcTextSize(_data->strings.get_string(page.text_id).c_str());
            size.x = -FLT_MIN;
            size.y += 8;

            ImGui::InputTextMultiline(make_id("##{}", page.text_id), &_data->strings.get_string(page.text_id), size);

            //input_text(std::format("Page {}", count), page.text);
            ToolTip(std::format("Page ID: {}", page_start_id + count).c_str());
            
            ImGui::TableNextColumn();

            if (ImGui::Button(make_id(ICON_MD_MORE_VERT "##more{}", page)))
            {
                ImGui::OpenPopup(make_id("##more_options{}", page));
            }

            ImGui::SetNextItemWidth(256);
            if (ImGui::BeginPopup(make_id("##more_options{}", page)))
            {
                ImGui::Checkbox(make_id("Page enabled##pageenabled{}", page.enabled), &page.enabled);

                ImGui::Checkbox(make_id("Disable when read##{}", page.self_disable), &page.self_disable);

                ImGui::Checkbox(make_id("Set Flag##{}", page.has_set_flag), &page.has_set_flag);
                if (!page.has_set_flag) ImGui::BeginDisabled();
                ImGui::SameLine(128);
                ImGui::SetNextItemWidth(128);
                ImGui::InputText(make_id("##page_setflag{}", page.set_flag), &page.set_flag);
                if (!page.has_set_flag) ImGui::EndDisabled();

                ImGui::Checkbox(make_id("Clear Flag##{}", page.has_clear_flag), &page.has_clear_flag);
                if (!page.has_clear_flag) ImGui::BeginDisabled();
                ImGui::SameLine(128);
                ImGui::SetNextItemWidth(128);
                ImGui::InputText(make_id("##page_clearflag{}", page.clear_flag), &page.clear_flag);
                if (!page.has_clear_flag) ImGui::EndDisabled();

                ImGui::Checkbox(make_id("Check Flag##{}", page.has_check_flag), &page.has_check_flag);
                if (!page.has_check_flag) ImGui::BeginDisabled();
                ImGui::SameLine(128);
                ImGui::SetNextItemWidth(128);
                ImGui::InputText(make_id("##page_checkflag{}", page.check_flag), &page.check_flag);
                if (!page.has_check_flag) ImGui::EndDisabled();

                ImGui::Separator();

                if (ImGui::Button(make_id(ICON_MD_PUBLISH "##create_new_page_above{}", page)))
                {
                    action = page_action_type::create_above;
                }
                ToolTip("Create a new page above this one.");

                ImGui::SameLine();

                if (DeleteButton(make_id("##DeleteButton{}", page), ""))
                {
                    action = page_action_type::delete_page;
                }
                ToolTip("Delete this page");

                ImGui::EndPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button(ICON_MD_LIST))
            {
                page.choices.push_back({});
            }
            ToolTip("Add a new choice");

            ImGui::SameLine(96);

            // Set optional page goto
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Goto:");
            ImGui::SameLine();
            auto v = to<int>(page.next_page_id);
            ImGui::SetNextItemWidth(128);
            ImGui::InputInt(std::format("##gotopage{}", (u64)&v).c_str(), &v);
            page.next_page_id = to<u16>(v);
                
            ImGui::TableNextRow();
            ImGui::TableNextColumn();


            display_choices(page);
        }

        return action;
    }

    void dialogue_editor::display_choices(dialogue_page& page)
    {
        auto count = 0;

        auto deletion_index = optional_index{ std::nullopt };
        for (auto& choice : page.choices)
        {
            if (!display_choice(choice, count))
            {
                deletion_index = count;
            }

            count++;
        }

        if (deletion_index)
        {
            auto const& choice = page.choices[deletion_index.value()];
            if (!choice.text_id.empty())
            {
                _data->strings.remove_string(choice.text_id);
            }

            page.choices.erase(page.choices.begin() + deletion_index.value());
        }
    }

    bool dialogue_editor::display_choice(dialogue_choice& choice, size_t count)
    {
        ImGui::TableNextColumn();

        auto keep_choice = true;

        if (auto table = imgui::table("choicetext", 2, ImGuiTableFlags_SizingStretchProp, { ImGui::GetColumnWidth(), 0 }))
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(std::format("Choice {}", count).data());

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);

            if (choice.text_id.empty())
            {
                choice.text_id = _data->strings.create_string_entry("");
            }

            auto size = ImGui::CalcTextSize(_data->strings.get_string(choice.text_id).c_str());
            size.x = -FLT_MIN;
            size.y += 8;
            ImGui::InputTextMultiline(make_id("##{}", choice.text_id), &_data->strings.get_string(choice.text_id), size);
        }

        ImGui::TableNextColumn();

        //auto popup_id = make_id("##more_options{}", choice);

        if (ImGui::Button(make_id(ICON_MD_MORE_VERT "##delete_choice{}", choice)))
        {
            ImGui::OpenPopup(make_id("##more_options{}", choice));
        }

        ImGui::SetNextItemWidth(256);
        if (ImGui::BeginPopup(make_id("##more_options{}", choice)))
        {
            ImGui::Checkbox(make_id("Choice enabled##choicenabled{}", choice.enabled), &choice.enabled);

            ImGui::Checkbox(make_id("Disable when clicked##{}", choice.self_disable), &choice.self_disable);
            ImGui::Checkbox(make_id("Script##{}", choice.has_script), &choice.has_script);

            if (!choice.has_script) ImGui::BeginDisabled();
            ImGui::SameLine(128);
            ImGui::SetNextItemWidth(128);
            ImGui::InputText(make_id("##choice_script{}", choice.script), &choice.script);

            if (!choice.has_script) ImGui::EndDisabled();

            ImGui::Checkbox(make_id("Set Flag##{}", choice.has_set_flag), &choice.has_set_flag);
            if (!choice.has_set_flag) ImGui::BeginDisabled();
            ImGui::SameLine(128);
            ImGui::SetNextItemWidth(128);
            ImGui::InputText(make_id("##choice_setflag{}", choice.set_flag), &choice.set_flag);
            if (!choice.has_set_flag) ImGui::EndDisabled();

            ImGui::Checkbox(make_id("Clear Flag##{}", choice.has_clear_flag), &choice.has_clear_flag);
            if (!choice.has_clear_flag) ImGui::BeginDisabled();
            ImGui::SameLine(128);
            ImGui::SetNextItemWidth(128);
            ImGui::InputText(make_id("##choice_clearflag{}", choice.clear_flag), &choice.clear_flag);
            if (!choice.has_clear_flag) ImGui::EndDisabled();

            ImGui::Checkbox(make_id("Check Flag##{}", choice.has_check_flag), &choice.has_check_flag);
            if (!choice.has_check_flag) ImGui::BeginDisabled();
            ImGui::SameLine(128);
            ImGui::SetNextItemWidth(128);
            ImGui::InputText(make_id("##choice_checkflag{}", choice.check_flag), &choice.check_flag);
            if (!choice.has_check_flag) ImGui::EndDisabled();

            ImGui::EndPopup();
        }

        ImGui::SameLine();

        if (DeleteButton(make_id("##DeleteButton{}", choice), ""))
        {
            keep_choice = false;
        }
        ToolTip("Delete this choice");

        ImGui::SameLine(96);

        // Set optional page goto
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Goto:");
        ImGui::SameLine();
        auto v = to<int>(choice.next_page_id);
        ImGui::SetNextItemWidth(128);
        ImGui::InputInt(std::format("##gotopage{}", (u64)&choice.next_page_id).c_str(), &v);
        choice.next_page_id = to<u16>(v);

        ImGui::TableNextColumn();

        return keep_choice;
    }
}
