#pragma once
#include <memory>

#include "game_data.h"

namespace NEONnoir
{
    class shapes_editor
    {
    public:
        shapes_editor() = default;

        void display(std::weak_ptr<game_data> game_data);

    private:
        void display_editor(std::shared_ptr<game_data> data);
        void display_placeholder() const noexcept;

    };
}
