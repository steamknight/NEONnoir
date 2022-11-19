#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_utils.h"

#include <format>

#include "dialogue_editor.h"
#include "utils.h"

namespace NEONnoir
{
    void dialogue_editor::display_editor(std::shared_ptr<game_data> data)
    {
        // Add dialogue button
        if (ImGui::Button(ICON_MD_ADD_COMMENT " Create a new dialogue"))
        {
            data->dialogues.push_back({});
        }

        auto speakers = std::vector<std::string>{ "Unnamed" };
        for (auto const& speaker : data->speakers)
        {
            speakers.push_back(speaker.name);
        }

        //if (auto table = imgui::table("DialogueTable", 2, ImGuiTableFlags_BordersH | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable))
        //{
            auto deletion_index = -1;
            auto count = 0;
            size_t page_count = 0;

            // Display all the dialogues
            for (auto& dialogue : data->dialogues)
            {
                auto speaker_name = (dialogue.speaker_id != 0xFFFF)
                    ? data->speakers[dialogue.speaker_id].name
                    : "Unnamed";

                if (ImGui::TreeNode(std::format(("Dialogue {}:{}###dialogue{}"), count, speaker_name, (size_t)&dialogue).c_str()))
                {
                    if (display_dialogue(dialogue, page_count, speakers))
                    {
                        deletion_index = count;
                    }

                    ImGui::TreePop();
                }

                count++;
                page_count += dialogue.pages.size();
            }

            if (deletion_index >= 0)
            {
                data->dialogues.erase(data->dialogues.begin() + deletion_index);
            }
        //}
    }

    bool dialogue_editor::display_dialogue(dialogue& dialogue, size_t page_start_id, std::vector<std::string> const& speakers)
    {
        //ImGui::TableNextRow();

        auto request_deletion = display_dialogue_options(dialogue, speakers);

        display_pages(dialogue, page_start_id);

        return request_deletion;
    }

    bool dialogue_editor::display_dialogue_options(dialogue& dialogue, std::vector<std::string> const& speakers)
    {
        //ImGui::TableNextColumn();

        auto request_deletion = false;

        if (ImGui::BeginTable(std::format("DialogOptions##{}", (size_t)&dialogue).c_str(), 2, ImGuiTableFlags_SizingStretchProp))
        {
            // Speaker
            display_combo_with_empty("Speaker", speakers, dialogue.speaker_id);

            ImGui::EndTable();

            // Add Page
            if (ImGui::Button(make_id(ICON_MD_ADD_COMMENT " Add Page##{}", dialogue), { 300, 0 }))
            {
                dialogue.pages.push_back({});
            }
            ImGui::SameLine();

            // Delete
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (DeleteButton(std::format("##DialogueDelete{}", (size_t)&dialogue), " Delete", {300, 0}))
            {
                request_deletion = true;
            }
        }

        return request_deletion;
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
        auto deletion_index = -1;
        if (auto table = imgui::table(make_id("Pages", dialogue.pages), 1, ImGuiTableFlags_BordersInnerH))
        {
            auto count = 0;
            for (auto& page : dialogue.pages)
            {
                if (!display_page(page, count, page_start_id))
                {
                    deletion_index = count;
                }

                count++;
            }
        }

        if (deletion_index >= 0)
        {
            dialogue.pages.erase(dialogue.pages.begin() + deletion_index);
        }
    }

    bool dialogue_editor::display_page(dialogue_page& page, size_t count, size_t page_start_id)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        bool keep_page = true;
        if (auto table = imgui::table(make_id("##Page{}", page), 3, ImGuiTableFlags_SizingStretchProp))
        {
            input_text(std::format("Page {}", count), page.text);
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

                ImGui::EndPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button(ICON_MD_LIST))
            {
                page.choices.push_back({});
            }
            ToolTip("Add a new choice");

            ImGui::SameLine();

            if (DeleteButton(make_id("##DeleteButton{}", page), ""))
            {
                keep_page = false;
            }
            ToolTip("Delete this page");

            ImGui::SameLine(128);

            // Set optional page goto
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Goto:");
            ImGui::SameLine();
            auto v = static_cast<int>(page.next_page_id);
            ImGui::SetNextItemWidth(128);
            ImGui::InputInt(std::format("##gotopage{}", (uint64_t)&v).c_str(), &v);
            page.next_page_id = static_cast<uint16_t>(v);
                
            ImGui::TableNextRow();
            ImGui::TableNextColumn();


            display_choices(page);
        }

        return keep_page;
    }

    void dialogue_editor::display_choices(dialogue_page& page)
    {
        auto count = 0;

        auto deletion_index = -1;
        for (auto& choice : page.choices)
        {
            if (!display_choice(choice, count))
            {
                deletion_index = count;
            }

            count++;
        }

        if (deletion_index >= 0)
        {
            page.choices.erase(page.choices.begin() + deletion_index);
        }
    }

    bool dialogue_editor::display_choice(dialogue_choice& choice, size_t count)
    {
        ImGui::TableNextColumn();

        auto keep_choice = true;

        if (auto table = imgui::table("choicetext", 2, ImGuiTableFlags_SizingStretchProp, { ImGui::GetColumnWidth(), 0 }))
        {
            input_text(std::format("Choice {}", count), choice.text);
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

        ImGui::SameLine(128);

        // Set optional page goto
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Goto:");
        ImGui::SameLine();
        auto v = static_cast<int>(choice.next_page_id);
        ImGui::SetNextItemWidth(128);
        ImGui::InputInt(std::format("##gotopage{}", (uint64_t)&choice.next_page_id).c_str(), &v);
        choice.next_page_id = static_cast<uint16_t>(v);

        ImGui::TableNextColumn();

        return keep_choice;
    }
}
