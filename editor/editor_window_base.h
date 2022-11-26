#pragma once

#include <memory>
#include <optional>
#include <vector>
#include <string>
#include <string_view>
#include "game_data.h"
#include "IconsMaterialDesign.h"

namespace NEONnoir
{
    class editor_window_base
    {
    public:
        editor_window_base() = default;
        virtual ~editor_window_base() = default;

        virtual void display(std::weak_ptr<game_data> data);

    protected:
        virtual std::string_view get_title_name() const noexcept = 0;
        virtual void display_editor(std::shared_ptr<game_data> data) = 0;

        void display_placeholder(std::string_view const& text = "No Game File");

        void display_combo_with_empty(std::string_view const& label, std::vector<std::string> const& values, u16& selected_value);


        std::vector<std::string>& get_speaker_list(std::vector<speaker_info> const& speakers);

    protected:
        std::optional<std::vector<std::string>> _speaker_list_with_empty;
    };
}

