#include "image_viewer.h"
#include "IconsMaterialDesign.h"

#include "imgui_utils.h"

void NEONnoir::image_viewer::display(GLtexture const& texture, std::vector<shape>& regions) noexcept
{
    // Toolbar
    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));

    if (ImGui::Button(ICON_MD_CROP))
    {
        _add_region_mode = true;
    }
    ToolTip("Add region");

    ImGui::SameLine();

    if (ImGui::SmallButton(ICON_MD_ZOOM_IN))
    {
        _zoom++;
    }
    ToolTip("Zoom In");

    ImGui::SameLine();

    if (ImGui::SmallButton(ICON_MD_ZOOM_OUT) && _zoom > 1)
    {
        _zoom--;
    }
    ToolTip("Zoom Out");

    ImGui::PopStyleColor();

    // Stats
    ImGui::Text(ICON_MD_NEAR_ME " %.0f, %.0f\t" ICON_MD_ZOOM_IN " %d00%%", _last_mouse.x, _last_mouse.y, _zoom);
    ImGui::Image((void*)(intptr_t)texture.texture_id, ImVec2((float)(texture.width * _zoom), (float)(texture.height * _zoom)));

    // Image
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    auto image_min = ImGui::GetItemRectMin();
    if (ImGui::IsItemHovered())
    {
        _last_mouse = (io.MousePos - image_min) / _zoom;
    }

    // Regions
    auto draw_list = ImGui::GetWindowDrawList();
    for (auto index = 0; index < regions.size(); index++)
    {
        auto const& region = regions[index];

        auto p0 = ImVec2{ static_cast<float>(region.x), static_cast<float>(region.y) } *_zoom + image_min;
        auto p1 = ImVec2{ static_cast<float>(region.x + region.width), static_cast<float>(region.y + region.height) } *_zoom + image_min;

        if (_selected_region_index == index)
        {
            draw_list->AddRectFilled(p0, p1, IM_COL32(255, 165, 0, 64));
            draw_list->AddRect(p0, p1, IM_COL32(255, 165, 0, 255), 0.f, 0, 2.f);
        }
        else
        {
            draw_list->AddRectFilled(p0, p1, IM_COL32(13, 129, 255, 64));
            draw_list->AddRect(p0, p1, IM_COL32(13, 129, 255, 255), 0.f, 0, 2.f);
        }
    }

    // Add Regions
    if (_add_region_mode)
    {
        if (io.MouseDown[0] && _last_mouse.x > 0 && _last_mouse.y > 0)
        {
            if (_add_region_p0.x < 0 && _add_region_p0.y < 0)
            {
                _add_region_p0 = io.MousePos;
                _add_region_dragging = true;
            }
        }
        if (io.MouseReleased[0] && _add_region_dragging)
        {
            auto p0 = (_add_region_p0 - image_min) / _zoom;
            auto p1 = (io.MousePos - image_min) / _zoom;

            auto p_min = ImVec2
            {
                std::min(p0.x, p1.x),
                std::min(p0.y, p1.y)
            };

            auto size = ImVec2
            {
                std::fabsf(p1.x - p0.x),
                std::fabsf(p1.y - p0.y)
            };

            // Sort the points so we don't get wonky regions
            auto region = shape{
                static_cast<uint16_t>(p_min.x), static_cast<uint16_t>(p_min.y),
                static_cast<uint16_t>(size.x), static_cast<uint16_t>(size.y)
            };

            regions.push_back(region);
            _add_region_mode = false;
            _add_region_dragging = false;
            _add_region_p0 = { -1, -1 };
        }

        if (_add_region_dragging)
        {
            draw_list->AddRectFilled(_add_region_p0, io.MousePos, IM_COL32(255, 165, 0, 64));
            draw_list->AddRect(_add_region_p0, io.MousePos, IM_COL32(255, 165, 0, 255), 0.f, 0, 2.f);
        }
    }

    for (auto index = 0; index < regions.size(); index++)
    {
        auto const& region = regions[index];

        auto p0 = ImVec2{ static_cast<float>(region.x), static_cast<float>(region.y) } *_zoom + image_min;
        auto p1 = ImVec2{ static_cast<float>(region.x + region.width), static_cast<float>(region.y + region.height) } *_zoom + image_min;

        if (_selected_region_index == index)
        {
            draw_list->AddRectFilled(p0, p1, IM_COL32(255, 165, 0, 64));
            draw_list->AddRect(p0, p1, IM_COL32(255, 165, 0, 255), 0.f, 0, 2.f);
        }
        else
        {
            draw_list->AddRectFilled(p0, p1, IM_COL32(13, 129, 255, 64));
            draw_list->AddRect(p0, p1, IM_COL32(13, 129, 255, 255), 0.f, 0, 2.f);
        }
    }
}

