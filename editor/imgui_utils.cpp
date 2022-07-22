#include "imgui_utils.h"
#include "IconsMaterialDesign.h"
#include <format>

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

    ImVec2 operator*(ImVec2 const& lhs, float scalar) noexcept
    {
        return ImVec2(lhs.x * scalar, lhs.y * scalar);
    }

    ImVec2 operator*(ImVec2 const& lhs, int scalar) noexcept
    {
        return ImVec2((int)lhs.x * scalar, (int)lhs.y * scalar);
    }

    ImVec2 operator/(ImVec2 const& lhs, float scalar) noexcept
    {
        return ImVec2(lhs.x / scalar, lhs.y / scalar);
    }

    ImVec2 operator/(ImVec2 const& lhs, int scalar) noexcept
    {
        return ImVec2((int)lhs.x / scalar, (int)lhs.y / scalar);
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
        ImGui::Begin(name.data(), closable ? &_is_open : nullptr, flags);
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
}