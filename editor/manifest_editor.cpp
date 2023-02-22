#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_utils.h"

#include "manifest_editor.h"

namespace NEONnoir
{
    void manifest_editor::display_editor()
    {
        display_gameinfo();
        ImGui::NewLine();
        display_fileinfo();
        ImGui::NewLine();
        display_asset_list("UI", _data->manifest.assets.ui);
        ImGui::NewLine();
        display_asset_list("Backgrounds", _data->manifest.assets.backgrounds);
        ImGui::NewLine();
        display_asset_list("Music", _data->manifest.assets.music);
        ImGui::NewLine();
        display_asset_list("SFX", _data->manifest.assets.sfx);
    }

    void manifest_editor::display_label(std::string_view const& label)
    {
        ImGui::TableNextColumn();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label.data());
    }

    void manifest_editor::display_string(std::string_view const& id, std::string& value)
    {
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText(id.data(), &value);
    }

    void manifest_editor::display_number(std::string_view const& id, u32& value)
    {
        ImGui::TableNextColumn();
        i32 v = to<i32>(value);
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputInt(id.data(), &v);
        value = to<u32>(value);
    }

    void manifest_editor::display_path(std::string_view const& id, std::filesystem::path& value)
    {
        ImGui::TableNextColumn();
        if (ImGui::Button(make_id(ICON_MD_FILE_OPEN "##{}open_", id)))
        {
            auto result = open_file_dialog("*");
            if (result)
            {
                value = std::filesystem::relative(result.value());
            }
        }

        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::TextUnformatted(value.string().c_str());
    }

    void manifest_editor::display_asset(game_asset& asset)
    {
        auto _ = imgui::push_id(&asset);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputText("##asset_name", &asset.name);


    }

    void manifest_editor::display_gameinfo()
    {
        auto _ = imgui::table("manifest_gameinfo", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        display_label("Game Title");
        display_string("##game_name", _data->manifest.game_name);

        ImGui::TableNextRow();
        display_label("Build Number");
        display_number("##data_build",  _data->manifest.build_number);
    }

    void manifest_editor::display_fileinfo()
    {
        ImGui::TextUnformatted("Files");

        auto _ = imgui::table("manifest_fileinfo", 2, ImGuiTableFlags_SizingFixedSame | ImGuiTableFlags_Resizable);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableNextRow();
        display_label("Project File");
        display_path("##project_file", _data->manifest.project_file);

        ImGui::TableNextRow();
        display_label("Build File");
        display_path("##build_file", _data->manifest.game_file);
    }

    void manifest_editor::display_asset_list(std::string_view const& label, std::vector<game_asset>& assets)
    {
        auto _ = imgui::push_id(&assets);
        ImGui::Text("Assets: %s", label.data());
        {
            auto table = imgui::table(label.data(), 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 1.f);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 3.f);
            for (auto& asset : assets)
            {
                auto __ = imgui::push_id(&asset);

                ImGui::TableNextRow();
                display_string("##asset_name", asset.name);
                ImGui::SameLine();

                if (DeleteButton("##assert_delete"))
                {

                }

                display_path("##asset_path", asset.relative_path);
            }
        }
        
        if (ImGui::Button(ICON_MD_ADD " Add Asset"))
        {
            assets.push_back({});
        }
    }
}
