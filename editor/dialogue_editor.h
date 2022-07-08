#pragma once

#include <memory>
#include "game_data.h"
namespace NEONnoir
{
    class dialogue_editor
    {
    public:
        dialogue_editor() = default;

        void display(std::weak_ptr<game_data> game_data);

    private:
        void display_placeholder() const noexcept;
        void display_editor(std::shared_ptr<game_data> data);

        bool display_dialogue(dialogue& dialogue, size_t page_start_id);
        void display_pages(dialogue& dialogue, size_t page_start_id);
        bool display_page(dialogue_page& page, size_t count, size_t page_start_id);
        void display_choices(dialogue_page& page);
        bool display_choice(dialogue_choice& choice, size_t count);
        bool display_dialogue_options(dialogue& dialogue);
        
        void input_text(std::string_view const& label, std::string& value);
    };
}

