#pragma once
#include "imgui.h"
#include <string_view>
#include <vector>
#include "types.h"

namespace NEONnoir
{

    ImVec2 operator+(ImVec2 const& lhs, ImVec2 const& rhs) noexcept;

    ImVec2 operator-(ImVec2 const& lhs, ImVec2 const& rhs) noexcept;

    ImVec2 operator*(ImVec2 const& lhs, ImVec2 const& rhs) noexcept;
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
        operator bool() const noexcept { return _is_valid; }
    private:
        bool _is_open{ true };
        bool _is_valid{ true };
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

    namespace imgui
    {
        template<class T>
        struct button_option
        {
            ::std::string_view name;
            ::std::string_view label;
            T value;
        };

        ImGui_guard table(::std::string_view const& id, int columns_count, ImGuiTableFlags flags = 0, ImVec2 const& outer_size = ImVec2(0.0f, 0.0f), float inner_width = 0.0f);
        ImGui_guard popup(::std::string_view const& id, ImGuiWindowFlags flags = ImGuiWindowFlags_None);
        ImGui_guard push_style_color(ImGuiCol idx, const ImVec4& col);

        bool toggle_button(::std::string_view const& id, ::std::string_view const& label, bool& value, const ImVec2& size = ImVec2(0, 0));

        template<class T, size_t N>
        T button_combo(::std::array<button_option<T>, N> const& options);

        template<class T>
        ImGui_guard push_id(T* pointer)
        {
            ImGui::PushID(force_to<void*>(pointer));
            return ImGui_guard { &ImGui::PopID, true };
        }

        ImGui_guard push_id(int id);

        void combo_with_empty(std::vector<std::string> const& values, NEONnoir::u16& selected_value);

        template<class T, size_t N>
        T button_combo(::std::array<button_option<T>, N> const& options, T value)
        {
            auto selected_button_color = ImGui::GetStyleColorVec4(ImGuiCol_Button);
            auto unselected_button_color = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);

            auto result = value;
            for (auto const& option : options)
            {
                auto style = push_style_color(ImGuiCol_Button, option.value == value ? selected_button_color : unselected_button_color);
                if (ImGui::Button(option.name.data()))
                {
                    result = option.value;
                }
                ToolTip(option.label.data());

                ImGui::SameLine();
            }

            return result;
        }
    };
}
