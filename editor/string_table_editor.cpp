#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_utils.h"

#include "string_table_editor.h"
#include "utils.h"

namespace NEONnoir
{
    void string_table_editor::display_editor()
    {
        display_toolbar();

        auto text_box_size = ImVec2{ -FLT_MIN, (ImGui::GetIO().FontDefault->FontSize * 4.0f) + (2.0f * ImGui::GetStyle().FramePadding.y) };

        auto content_size = ImGui::GetContentRegionAvail();
        auto details_size = content_size - ((text_box_size + ImVec2{0.0f, ImGui::GetStyle().FramePadding.y}) * 2.0f);

        auto const flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_NoHostExtendX | ImGuiTableFlags_ScrollY;
        if (auto table = imgui::table("string_table", 4, flags, _selected_string_index ? details_size : ImVec2{0.0f, 0.0f}))
        {
            ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
            ImGui::TableSetupColumn(" ");
            ImGui::TableSetupColumn("String Id");
            ImGui::TableSetupColumn("String");
            ImGui::TableSetupColumn("Description");
            ImGui::TableHeadersRow();

            auto clipper = ImGuiListClipper{};
            clipper.Begin(to<int>(_data->strings.entries.size()));


            while (clipper.Step())
            {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
                {
                    auto _ = imgui::push_id(to<int>(row));

                    auto& string_entry = _data->strings.entries[row];

                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    ImGui::AlignTextToFramePadding();
                    ImGui::TextUnformatted(std::format("{}", row).c_str());

                    ImGui::TableNextColumn();
                    if (ImGui::Button(make_id(ICON_MD_MORE_VERT "##{}", string_entry.id)))
                    {
                        ImGui::OpenPopup(make_id("##more_options123{}", string_entry));
                    }

                    ImGui::SetNextItemWidth(256);
                    if (ImGui::BeginPopup(make_id("##more_options123{}", string_entry)))
                    {
                        ImGui::TextWrapped("Changing the string id can have catastrophic effects.");
                        auto string_id{ string_entry.id };
                        if (ImGui::InputText(make_id("##{}", string_id), &string_id, ImGuiInputTextFlags_EnterReturnsTrue))
                        {
                            _data->strings.change_string_id(string_entry.id, string_id);
                            ImGui::CloseCurrentPopup();
                        }

                        ImGui::Separator();

                        if (ImGui::Button(make_id(ICON_MD_CONTENT_COPY "##{}", string_entry.id)))
                        {
                            ImGui::LogToClipboard();
                            ImGui::LogText(string_entry.id.c_str());
                            ImGui::LogFinish();
                        }

                        ImGui::EndPopup();

                    }

                    ImGui::SameLine();

                    //ImGui::AlignTextToFramePadding();
                    //ImGui::TextUnformatted(string_entry.id.c_str());
                    if (ImGui::Selectable(std::format("{}##{}", string_entry.id.c_str(), row).c_str()))
                    {
                        _selected_string_index = row;
                    }

                    ImGui::TableNextColumn();
                    auto size = ImGui::CalcTextSize(string_entry.value.c_str());
                    size.x = -FLT_MIN;
                    size.y += (ImGui::GetStyle().FramePadding.y * 2.0f);
                    ImGui::InputTextMultiline(make_id("##{}", string_entry.value), &string_entry.value, size);

                    ImGui::TableNextColumn();
                    ImGui::InputTextMultiline(make_id("##{}", string_entry.description), &string_entry.description, size);
                }
            }

        }

        if (_selected_string_index)
        {
            auto& string_entry = _data->strings.entries[_selected_string_index.value()];
            auto size = ImVec2{ -FLT_MIN, (ImGui::GetIO().FontDefault->FontSize * 4.0f) + (2.0f * ImGui::GetStyle().FramePadding.y) };

            ImGui::InputTextMultiline(make_id("String##{}", string_entry.value), &string_entry.value, size);
            ImGui::InputTextMultiline(make_id("String##{}", string_entry.description), &string_entry.description, size);
        }
    }

    void string_table_editor::display_toolbar() noexcept
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));

        if (ImGui::Button(ICON_MD_PLAYLIST_ADD))
        {
            _data->strings.create_string_entry("");
        }
        ToolTip("Add new string entry");
        ImGui::SameLine();

        if (ImGui::Button(ICON_MD_TRANSLATE))
        {
            auto file = save_file_dialog("pot");
            if (file)
            {
                _data->strings.generate_po_file(file.value());
            }
        }
        ToolTip("Export .POT file");

        ImGui::PopStyleColor();
    }
}
