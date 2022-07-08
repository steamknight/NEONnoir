#pragma once
#include "imgui.h"
#include <string_view>

namespace NEONnoir
{

    ImVec2 operator+(ImVec2 const& lhs, ImVec2 const& rhs) noexcept;

    ImVec2 operator-(ImVec2 const& lhs, ImVec2 const& rhs) noexcept;

    ImVec2 operator*(ImVec2 const& lhs, float scalar) noexcept;
    ImVec2 operator*(ImVec2 const& lhs, int scalar) noexcept;

    ImVec2 operator/(ImVec2 const& lhs, float scalar) noexcept;
    ImVec2 operator/(ImVec2 const& lhs, int scalar) noexcept;

    void HelpMarker(const char* desc);

    void ToolTip(char const* desc);

    void SetCursorCenteredText(ImVec2 center, std::string_view const& text, float wrap_width = -1.f);

    bool DeleteButton(std::string const& id, std::string_view const& label = "", ImVec2 const& size = {0, 0});
    bool DeleteSelectable(std::string const& id, std::string_view const& label = "");

    class ImGui_window
    {
    public:
        ImGui_window(std::string_view const& name, bool closable = false, ImGuiWindowFlags flags = ImGuiWindowFlags_None);
        ~ImGui_window();

        bool is_closing() const noexcept { return !_is_open; }
    private:
        bool _is_open{ false };
    };

}