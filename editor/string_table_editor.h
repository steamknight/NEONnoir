#pragma once
#include "editor_window_base.h"

namespace NEONnoir
{
    class string_table_editor : public editor_window_base
    {

    public:
        string_table_editor(std::shared_ptr<game_data> data, GLFWwindow* window) : editor_window_base(data, window) { };
        virtual ~string_table_editor() = default;

    private:
        std::string_view get_title_name() const noexcept override { return ICON_MD_LIST " String Table"; };
        void display_editor() override;
        void display_toolbar() noexcept;

    private:
        std::optional<int> _selected_string_index{ std::nullopt };
    };

}

