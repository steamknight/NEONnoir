#pragma once

#include "editor_window_base.h"

namespace NEONnoir
{
    class dialogue_editor : public editor_window_base
    {
    public:
        enum class dialogue_action_type
        {
            none,
            delete_dialogue,
            merge_down
        };

        enum class page_action_type
        {
            none,
            delete_page,
            create_above
        };

    public:
        dialogue_editor() = default;
        virtual ~dialogue_editor() = default;

    private:
        std::string_view get_title_name() const noexcept override { return ICON_MD_FORUM " Dialogue Editor"; };
        void display_editor(std::shared_ptr<game_data> data) override;

        dialogue_action_type display_dialogue(dialogue& dialogue, size_t page_start_id, std::vector<std::string> const& speakers);
        void display_pages(dialogue& dialogue, size_t page_start_id, std::vector<std::string> const& speakers);
        page_action_type display_page(dialogue_page& page, size_t count, size_t page_start_id, std::vector<std::string> const& speakers);
        void display_choices(dialogue_page& page);
        bool display_choice(dialogue_choice& choice, size_t count);
        dialogue_action_type display_dialogue_options(dialogue& dialogue);
        
        void input_text(std::string_view const& label, std::string& value);
    };
}

