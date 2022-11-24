#pragma once

#include "IconsMaterialDesign.h"
#include "editor_window_base.h"
#include "simple_image.h"

namespace NEONnoir
{
    class speaker_editor : public editor_window_base
    {
    public:
        speaker_editor() = default;
        virtual ~speaker_editor() = default;

    private:
        std::string_view get_title_name() const noexcept override { return ICON_MD_PERSON " Speaker Editor"; };
        void display_editor(std::shared_ptr<game_data> data) override;

        void display_toolbar(std::vector<speaker_info>& speakers) noexcept;
        bool display_speaker(speaker_info& speaker) noexcept;

        void save_shapes(std::filesystem::path const& shapes_file_path, std::vector<speaker_info>& speakers) const;
        void save_mpsh_shapes(std::filesystem::path const& shapes_file_path, std::vector<speaker_info>& speakers) const;

    private:
        int _bit_depth{ 5 };
        int _zoom{ 1 };

        static MPG::simple_image _no_person;
    };
}

