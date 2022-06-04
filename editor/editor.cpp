#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include "IconsMaterialDesign.h"
#include "imgui_utils.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <filesystem>
#include <format>

#include "utils.h"
#include "editor.h"

namespace fs = std::filesystem;

namespace NEONnoir
{
    editor::editor(editor::settings const& setting)
    {
        // Initialize GLFW
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        _window = GLFWwindow_ptr{ glfwCreateWindow(setting.width, setting.height, setting.title.c_str(), nullptr, nullptr) };
        ensure_valid(_window.get(), "Unable to create GLFW window");

        glfwMakeContextCurrent(_window.get());

        // Initialize GLAD/OpenGL
        gladLoadGL((GLADloadfunc)glfwGetProcAddress);
        glViewport(0, 0, setting.width, setting.height);

        glfwGetWindowContentScale(_window.get(), &_dpi_scale_x, &_dpi_scale_y);

        // Intitialize IMGUI
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        // Enable Docking
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(_window.get(), true);
        ImGui_ImplOpenGL3_Init("#version 330");
 
        // Load some custom fonts
        _ui_font = io.Fonts->AddFontFromFileTTF("data/Roboto-Medium.ttf", 18);
        //_monospaced_font = io.Fonts->AddFontFromFileTTF("data/CascadiaCode.ttf", 18);

        // Add icons to the ui font
        auto config = ImFontConfig{};
        config.MergeMode = true;
        config.PixelSnapH = true;
        config.GlyphOffset = { 0, 4 };
        config.GlyphMinAdvanceX = 18;
        ImWchar const icon_range[] = { ICON_MIN_MD, ICON_MAX_16_MD, 0 };
        io.Fonts->AddFontFromFileTTF("data/MaterialIcons-Regular.ttf" , 18, &config, icon_range);
        io.Fonts->Build();
    }

    editor::~editor() noexcept
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwTerminate();
    }

    void editor::run()
    {
        auto texture = load_texture("C:/Users/mass/source/NeonNoirEdit/source.bmp");
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        auto zoom = 1;
        ImVec2 region_start{-1,-1};
        while (!glfwWindowShouldClose(_window.get()))
        {
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

            process_main_menu();

            //ImGui::Begin("Scene");

            //if (ImGui::Button(ICON_MD_ZOOM_IN " Zoom In")) {zoom++; }
            //ImGui::SameLine();
            //if (ImGui::Button(ICON_MD_ZOOM_OUT " Zoom Out") && zoom > 1) { zoom--; };

            //ImGui::Image((void*)(intptr_t)texture.texture_id, ImVec2(texture.width * zoom * _dpi_scale_x, texture.height * zoom * _dpi_scale_y));
            //if (ImGui::IsItemHovered())
            //{
            //    auto image_min = ImGui::GetItemRectMin();
            //    auto imagePos = (io.MousePos - image_min) / zoom;
            //    ImGui::Text("Image: (%.0f, %.0f)", imagePos.x, imagePos.y);

            //    //auto draw_list = ImGui::GetWindowDrawList();
            //    //
            //    //if (region_start.x < 0)
            //    //{
            //    //    region_start.x = io.MousePos.x - pos.x;
            //    //    region_start.y = io.MousePos.y - pos.y;
            //    //}

            //    //auto size = ImVec2(
            //    //    io.MousePos.x - pos.x,
            //    //    io.MousePos.y - pos.y
            //    //);

            //    //draw_list->AddRect(region_start, size, IM_COL32(255, 0, 0, 255), 0.f, 0, zoom);


            //    //ImVec4 tint_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);   // No tint
            //    //ImVec4 border_col = ImVec4(1.0f, 1.0f, 1.0f, 0.5f); // 50% opaque white
            //    //ImVec2 pos = ImGui::GetCursorScreenPos();
            //    //ImGui::BeginTooltip();
            //    //float region_sz = 32.0f;
            //    //float region_x = io.MousePos.x - pos.x - region_sz * 0.5f;
            //    //float region_y = io.MousePos.y - pos.y - region_sz * 0.5f;
            //    //float zoom = 4.0f;
            //    //if (region_x < 0.0f) { region_x = 0.0f; }
            //    //else if (region_x > texture.width - region_sz) { region_x = texture.width - region_sz; }
            //    ///*if (region_y < 0.0f) { region_y = 0.0f; }
            //    //else */
            //    //    if (region_y > texture.height - region_sz) { region_y = texture.height - region_sz; }
            //    //ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
            //    //ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
            //    //ImVec2 uv0 = ImVec2((region_x) / texture.width, (region_y) / texture.height);
            //    //ImVec2 uv1 = ImVec2((region_x + region_sz) / texture.width, (region_y + region_sz) / texture.height);
            //    //ImGui::Image((void*)(intptr_t)texture.texture_id, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, tint_col, border_col);
            //    //ImGui::EndTooltip();
            //    
            //}
            //ImGui::End();

            if (_palette_injector && !_palette_injector->display())
            {
                _palette_injector.reset();
            }

            _location_browser.display();

            _scene_editor.use(_game_data,
                _location_browser.get_selected_location_index(),
                _location_browser.get_selected_scene());

            _scene_editor.display();



            ImGui::ShowDemoWindow();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


            glfwSwapBuffers(_window.get());
            glfwPollEvents();
        }
    }

    void editor::process_main_menu()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New", "Ctrl+N"))
                {
                    // TODO: check if there already is game_data in memory
                    _game_data = std::make_shared<game_data>();
                    _location_browser.use(_game_data);
                }
                if (ImGui::MenuItem("Open", "Ctrl+O")) {}
                if (ImGui::MenuItem("Save", "Ctrl+S")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Alt+F4"))
                {
                    glfwSetWindowShouldClose(_window.get(), GLFW_TRUE);
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
                if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "CTRL+X")) {}
                if (ImGui::MenuItem("Copy", "CTRL+C")) {}
                if (ImGui::MenuItem("Paste", "CTRL+V")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(ICON_MD_CONSTRUCTION " Tools"))
            {
                if (ImGui::MenuItem("Palette Injector..."))
                {
                    if (!_palette_injector)
                    {
                        _palette_injector = std::make_unique<palette_injector>();
                    }
                    _palette_injector->display();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
}