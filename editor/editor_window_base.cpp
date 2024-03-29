#include "imgui.h"
//#include "imgui_stdlib.h"
//#include "IconsMaterialDesign.h"
#include "imgui_utils.h"
//#include "glfw_utils.h"

#include "editor_window_base.h"

namespace NEONnoir
{
    void editor_window_base::display()
    {
        if (auto dialogue_editor_window = ImGui_window(get_title_name()))
        {
            display_editor();
        }
    }

    void editor_window_base::display_placeholder(std::string_view const& text)
    {
        auto const origin = ImGui::GetCursorScreenPos();
        auto const size = ImGui::GetContentRegionAvail();

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        draw_list->AddRectFilled(origin, origin + size, IM_COL32(4, 16, 32, 255));

        SetCursorCenteredText(origin + (size / 2), text);
        ImGui::TextColored({ 16.f / 255.f, 64.f / 255.f, 128.f / 255.f, 1.f }, text.data());
    }

    void editor_window_base::display_combo_with_empty(std::string_view const& label, std::vector<std::string> const& values, u16& selected_value)
    {
        ImGui::TableNextRow();

        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label.data());

        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);

        // Sanity check in case something's been deleted
        selected_value = (selected_value < values.size() - 1) ? selected_value : NO_INDEX;

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

    std::vector<std::string>& editor_window_base::get_speaker_list(std::vector<speaker_info> const& speakers)
    {
        if (!_speaker_list_with_empty)
        {
            _speaker_list_with_empty = std::vector<std::string>{ "------" };
            for (auto const& speaker : speakers)
            {
                _speaker_list_with_empty.value().push_back(speaker.name);
            }
        }

        return _speaker_list_with_empty.value();
    }
}
