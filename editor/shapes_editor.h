#pragma once
#include <memory>
#include <optional>
#include <string>
#include <filesystem>

#include "editor_window_base.h"
#include "game_data.h"
#include "image_viewer.h"

namespace NEONnoir
{
    class shapes_editor : public editor_window_base
    {
    public:
        shapes_editor(std::shared_ptr<game_data> data, GLFWwindow* window) : editor_window_base(data, window) { };
        virtual ~shapes_editor() = default;

        void display(game_data_location* location);

    private:
        void display_editor() override;
        void save_shapes(std::filesystem::path const& shapes_file_path) const;

    private:
        game_data_location* _location{ nullptr };
        std::optional<size_t> _selected_image{ std::nullopt };
        std::optional<size_t> _shape_container_to_delete{ std::nullopt };
        std::optional<size_t> _shape_to_delete{ std::nullopt };
        image_viewer _shape_image;
    };
}
