#pragma once
#include "imgui.h"
#include <string_view>
#include <functional>

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
        bool _is_open{ true };
    };

    typedef void(*cleanup_fn)();
    class ImGui_guard
    {
    public:
        ImGui_guard(cleanup_fn cleanup, bool is_valid) : _cleanup(cleanup), _is_valid(is_valid) {};
        ~ImGui_guard() noexcept { if (_is_valid) _cleanup(); }

        operator bool() const noexcept { return _is_valid; }
    private:
        cleanup_fn _cleanup;
        bool _is_valid;
    };

    class imgui
    {
    public:
        static ImGui_guard table(std::string_view const& id, int columns_count, ImGuiTableFlags flags = 0, ImVec2 const& outer_size = ImVec2(0.0f, 0.0f), float inner_width = 0.0f);
    };

}