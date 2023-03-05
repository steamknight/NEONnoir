#include "imgui_utils.h"
#include "IconsMaterialDesign.h"
#include <format>
#include "types.h"

namespace NEONnoir
{
    ImVec2 operator+(ImVec2 const& lhs, ImVec2 const& rhs) noexcept
    {
        return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
    }

    ImVec2 operator-(ImVec2 const& lhs, ImVec2 const& rhs) noexcept
    {
        return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y);
    }

    ImVec2 operator*(ImVec2 const& lhs, ImVec2 const& rhs) noexcept
    {
        return ImVec2(lhs.x * rhs.x, lhs.y * rhs.y);
    }

    ImVec2 operator*(ImVec2 const& lhs, float scalar) noexcept
    {
        return ImVec2(lhs.x * scalar, lhs.y * scalar);
    }

    // This is meant to truncate
    ImVec2 operator*(ImVec2 const& lhs, int scalar) noexcept
    {
        return ImVec2(
            to<float>(to<int>(lhs.x) * scalar), 
            to<float>(to<int>(lhs.y) * scalar)
        );
    }

    ImVec2 operator/(ImVec2 const& lhs, float scalar) noexcept
    {
        return ImVec2(lhs.x / scalar, lhs.y / scalar);
    }

    // This is meant to truncate
    ImVec2 operator/(ImVec2 const& lhs, int scalar) noexcept
    {
        return ImVec2(
            to<float>(to<int>(lhs.x) / scalar),
            to<float>(to<int>(lhs.y) / scalar)
        );
    }

    void HelpMarker(const char* desc)
    {
        ImGui::Text(ICON_MD_HELP_OUTLINE);
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    void ToolTip(char const* desc)
    {
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    void SetCursorCenteredText(ImVec2 center, std::string_view const& text, float wrap_width)
    {
        auto text_sz = ImGui::CalcTextSize(text.data(), nullptr, false, wrap_width);
        ImGui::SetCursorScreenPos(center - (text_sz / 2));
    }

    bool DeleteButton(std::string const& id, std::string_view const& label, ImVec2 const& size)
    {
        ImGui::PushID(id.c_str());
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.4f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
        auto clicked = ImGui::Button(std::format(ICON_MD_DELETE "{}", label).c_str(), size);
        ImGui::PopStyleColor(3);
        ImGui::PopID();

        return clicked;
    }

    bool DeleteSelectable(std::string const& id, std::string_view const& label)
    {
        ImGui::PushID(id.c_str());
        ImGui::PushStyleColor(ImGuiCol_Header, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.4f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
        auto clicked = ImGui::Selectable(std::format(ICON_MD_DELETE "{} ", label).c_str());
        ImGui::PopStyleColor(3);
        ImGui::PopID();

        return clicked;
    }

    ImGui_window::ImGui_window(std::string_view const& name, bool closable, ImGuiWindowFlags flags)
    {
        _is_valid = ImGui::Begin(name.data(), closable ? &_is_open : nullptr, flags);
    }

    ImGui_window::~ImGui_window()
    {
        ImGui::End();
    }

    ImGui_guard imgui::table(std::string_view const& id, int columns_count, ImGuiTableFlags flags, ImVec2 const& outer_size, float inner_width)
    {
        return ImGui_guard
        {
            &ImGui::EndTable,
            ImGui::BeginTable(id.data(), columns_count, flags, outer_size, inner_width) 
        };
    }

    ImGui_guard imgui::popup(::std::string_view const& id, ImGuiWindowFlags flags)
    {
        return ImGui_guard
        {
            &ImGui::EndPopup,
            ImGui::BeginPopup(id.data(), flags)
        };
    }

    ImGui_guard imgui::push_style_color(ImGuiCol idx, const ImVec4& col)
    {
        ImGui::PushStyleColor(idx, col);
        return ImGui_guard
        {
            []() {ImGui::PopStyleColor(); } ,
            true
        };
    }

    bool imgui::toggle_button(::std::string_view const& id, ::std::string_view const& label, bool& value, const ImVec2& size)
    {
        auto selected_button_color = ImGui::GetStyleColorVec4(ImGuiCol_Button);
        auto unselected_button_color = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
        auto style = push_style_color(ImGuiCol_Button, value ? selected_button_color : unselected_button_color);
        auto result = ImGui::Button(id.data(), size);
        ToolTip(label.data());

        return result;
    }

    ImGui_guard imgui::push_id(int id)
    {
        ImGui::PushID(id);
        return ImGui_guard{ &ImGui::PopID, true };
    }

    void imgui::combo_with_empty(std::vector<std::string> const& values, u16& selected_value)
    {
        // Sanity check in case something's been deleted
        selected_value = (selected_value < values.size() - 1) ? selected_value : 0xFFFF;

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
}