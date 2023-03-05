#pragma once

#include <memory>
#include <optional>
#include <vector>
#include <string>
#include <string_view>
#include "game_data.h"
#include "IconsMaterialDesign.h"
#include "glfw_utils.h"

namespace NEONnoir
{
    class editor_window_base
    {
    public:
        editor_window_base(std::shared_ptr<game_data> data, GLFWwindow* window) : _data(data), _main_window(window) {};
        virtual ~editor_window_base() = default;

        virtual void display();

    protected:
        virtual std::string_view get_title_name() const noexcept { return ""; }
        virtual void display_editor() {};

        void display_placeholder(std::string_view const& text = "No Game File");

        void display_combo_with_empty(std::string_view const& label, std::vector<std::string> const& values, u16& selected_value);


        std::vector<std::string>& get_speaker_list(std::vector<speaker_info> const& speakers);

    protected:
        std::shared_ptr<game_data> _data;
        std::optional<std::vector<std::string>> _speaker_list_with_empty;
        GLFWwindow* _main_window{ nullptr };
    };
}

