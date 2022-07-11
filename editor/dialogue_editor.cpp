#include "imgui.h"
#include "imgui_stdlib.h"
#include "IconsMaterialDesign.h"
#include "imgui_utils.h"

#include <format>

#include "dialogue_editor.h"
#include "utils.h"

namespace NEONnoir
{

    void dialogue_editor::display(std::weak_ptr<game_data> game_data)
    {
        auto dialogue_editor_window = ImGui_window(ICON_MD_FORUM " Dialogue Editor");

        if (auto data = game_data.lock())
        {
            display_editor(data);
        }
        else
        {
            display_placeholder();
        }
    }

    void dialogue_editor::display_placeholder() const noexcept
    {
        auto const origin = ImGui::GetCursorScreenPos();
        auto const size = ImGui::GetContentRegionAvail();

        ImGuiIO& io = ImGui::GetIO();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        draw_list->AddRectFilled(origin, origin + size, IM_COL32(4, 16, 32, 255));
        SetCursorCenteredText(origin + (size / 2), "No Game File");
        ImGui::TextColored({ 16.f / 255.f, 64.f / 255.f, 128.f / 255.f, 1.f }, "No Game File");
    }

    void dialogue_editor::display_editor(std::shared_ptr<game_data> data)
    {
        // Add dialogue button
        if (ImGui::Button(ICON_MD_ADD_COMMENT " Create a new dialogue"))
        {
            data->dialogues.push_back({});
        }


        if (ImGui::BeginTable("DialogueTable", 2, ImGuiTableFlags_BordersH | ImGuiTableFlags_SizingStretchProp))
        {
            auto deletion_index = -1;
            auto count = 0;
            size_t page_count = 0;

            // Display all the dialogues
            for (auto& dialogue : data->dialogues)
            {
                if (display_dialogue(dialogue, page_count))
                {
                    deletion_index = count;
                }

                count++;
                page_count += dialogue.pages.size();
            }

            if (deletion_index >= 0)
            {
                data->dialogues.erase(data->dialogues.begin() + deletion_index);
            }

            ImGui::EndTable();
        }
    }

    bool dialogue_editor::display_dialogue(dialogue& dialogue, size_t page_start_id)
    {
        ImGui::TableNextRow();

        auto request_deletion = display_dialogue_options(dialogue);

        display_pages(dialogue, page_start_id);

        return request_deletion;
    }

    bool dialogue_editor::display_dialogue_options(dialogue& dialogue)
    {
        ImGui::TableNextColumn();

        auto request_deletion = false;

        if (ImGui::BeginTable(std::format("DialogOptions##{}", (size_t)&dialogue).c_str(), 2, ImGuiTableFlags_SizingStretchProp, {300, 0}))
        {
            // Speaker name
            input_text("Speaker", dialogue.speaker);

            // Speaker image
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Speaker Image");

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            auto v = static_cast<int>(dialogue.image_id);
            ImGui::InputInt(std::format("##{}", (uint64_t)&v).c_str(), &v);
            dialogue.image_id = static_cast<uint16_t>(v);

            ImGui::EndTable();

            // Add Page
            if (ImGui::Button(make_id(ICON_MD_ADD_COMMENT " Add Page##{}", dialogue), { 300, 0 }))
            {
                dialogue.pages.push_back({});
            }

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
        if (ImGui::BeginTable(make_id("Pages", dialogue.pages), 1, ImGuiTableFlags_BordersInnerH))
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

            ImGui::EndTable();
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
        if (ImGui::BeginTable(make_id("##Page{}", page), 3, ImGuiTableFlags_SizingStretchProp))
        {
            input_text(std::format("Page {}", count), page.text);
            ToolTip(std::format("Page ID: {}", page_start_id + count).c_str());
            
            ImGui::TableNextColumn();

            if (ImGui::Button(ICON_MD_LIST))
            {
                page.choices.push_back({});
            }
            ToolTip("Add a new choice");

            ImGui::SameLine();

            // Enabled/disable page
            ImGui::Checkbox(make_id("##pageenabled{}", page.enabled), &page.enabled);
            ToolTip("Enable/Disable page");

            ImGui::SameLine();

            if (DeleteButton(make_id("##DeleteButton{}", page), ""))
            {
                keep_page = false;
            }
            ToolTip("Delete this page");

            ImGui::SameLine(0, 32);

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

            ImGui::EndTable();
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

        if (ImGui::BeginTable("foo", 2, ImGuiTableFlags_SizingStretchProp, { ImGui::GetColumnWidth(), 0 }))
        {
            input_text(std::format("Choice {}", count), choice.text);

            ImGui::EndTable();
        }

        ImGui::TableNextColumn();

        //auto popup_id = make_id("##more_options{}", choice);

        if (ImGui::Button(make_id(ICON_MD_MORE_VERT "##delete_choice{}", choice)))
        {
            ImGui::OpenPopup(make_id("##more_options{}", choice));
        }

        if (ImGui::BeginPopup(make_id("##more_options{}", choice)))
        {
            ImGui::Checkbox(make_id("Disable when clicked##{}", choice.self_disable), &choice.self_disable);
            ImGui::Checkbox(make_id("Script##{}", choice.has_script), &choice.has_script);

            if (!choice.has_script) ImGui::BeginDisabled();
            ImGui::SameLine();
            ImGui::SetNextItemWidth(128);
            ImGui::InputText(make_id("##choice_script{}", choice.script), &choice.script);

            if (!choice.has_script) ImGui::EndDisabled();

            ImGui::Checkbox(make_id("Flag##{}", choice.has_flag), &choice.has_flag);
            if (!choice.has_flag) ImGui::BeginDisabled();
            ImGui::SameLine();
            ImGui::SetNextItemWidth(128);
            ImGui::InputText(make_id("##choice_flag{}", choice.flag), &choice.flag);

            if (!choice.has_flag) ImGui::EndDisabled();

            ImGui::EndPopup();
        }

        ImGui::SameLine();

        // Enabled/disable page
        ImGui::Checkbox(make_id("##choicenabled{}", choice.enabled), &choice.enabled);
        ToolTip("Enable/Disable choice");

        ImGui::SameLine();

        if (DeleteButton(make_id("##DeleteButton{}", choice), ""))
        {
            keep_choice = false;
        }
        ToolTip("Delete this choice");

        ImGui::SameLine(0, 32);

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
