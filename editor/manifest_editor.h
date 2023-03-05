#pragma once
#include "editor_window_base.h"

namespace NEONnoir
{
    class manifest_editor : public editor_window_base
    {
    public:
        manifest_editor(std::shared_ptr<game_data> data, GLFWwindow* window) : editor_window_base(data, window) { };
        virtual ~manifest_editor() = default;

    private:
        std::string_view get_title_name() const noexcept override { return ICON_MD_RECEIPT_LONG " Manifest"; }
        void display_editor() override;

        void display_label(std::string_view const& label);
        void display_string(std::string_view const& id, std::string& value);
        void display_number(std::string_view const& id, u32& value);
        void display_path(std::string_view const& id, std::filesystem::path& value);
        void display_asset(game_asset& asset);
        void display_gameinfo();
        void display_fileinfo();
        void display_asset_list(std::string_view const& label, std::vector<game_asset>& assets);
    };
}

