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
#include <iostream>

#include "utils.h"
#include "editor.h"
#include "packfile.h"
#include "lang_pack_util.h"

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
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

        _window = GLFWwindow_ptr{ glfwCreateWindow(setting.width, setting.height, setting.title.c_str(), nullptr, nullptr) };
        ensure_valid(_window.get(), "Unable to create GLFW window");

        glfwMakeContextCurrent(_window.get());

        // Initialize GLAD/OpenGL
        gladLoadGL((GLADloadfunc)glfwGetProcAddress);
        glViewport(0, 0, setting.width, setting.height);

        glfwSetWindowUserPointer(_window.get(), this);

        //glfwSetWindowContentScaleCallback(_window.get(), [](GLFWwindow* window, float /*xscale*/, float /*yscale*/)
        //    {
        //        auto engine_ptr = force_to<editor*>(glfwGetWindowUserPointer(window));
        //        engine_ptr->load_fonts();
        //    });

        // Intitialize IMGUI
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        // Enable Docking
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigWindowsMoveFromTitleBarOnly = true;

        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(_window.get(), true);
        ImGui_ImplOpenGL3_Init("#version 330");
 
        //// Load some custom fonts
        //_ui_font = io.Fonts->AddFontFromFileTTF("data/Roboto-Medium.ttf", 18 * _dpi_scale_x);

        //// Add icons to the ui font
        //auto config = ImFontConfig{};
        //config.MergeMode = true;
        //config.PixelSnapH = true;
        //config.GlyphOffset = { 0, 4 };
        //config.GlyphMinAdvanceX = 18;
        //ImWchar const icon_range[] = { ICON_MIN_MD, ICON_MAX_16_MD, 0 };
        //io.Fonts->AddFontFromFileTTF("data/MaterialIcons-Regular.ttf" , 18 * _dpi_scale_x, &config, icon_range);
        //io.Fonts->Build();

        //_monospaced_font = io.Fonts->AddFontFromFileTTF("data/CascadiaCode.ttf", 18 * _dpi_scale_x);
        load_fonts();
    }

    editor::~editor() noexcept
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwTerminate();
    }

    void editor::load_fonts() noexcept
    {
        glfwGetWindowContentScale(_window.get(), &_dpi_scale_x, &_dpi_scale_y);

        // Load some custom fonts
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        io.Fonts->Clear();

        _ui_font = io.Fonts->AddFontFromFileTTF("data/Roboto-Medium.ttf", 18 * _dpi_scale_x);

        // Add icons to the ui font
        auto config = ImFontConfig{};
        config.MergeMode = true;
        config.PixelSnapH = true;
        config.GlyphOffset = { 0, 4 };
        config.GlyphMinAdvanceX = 18;
        ImWchar const icon_range[] = { ICON_MIN_MD, ICON_MAX_16_MD, 0 };
        io.Fonts->AddFontFromFileTTF("data/MaterialIcons-Regular.ttf", 18 * _dpi_scale_x, &config, icon_range);

        _monospaced_font = io.Fonts->AddFontFromFileTTF("data/CascadiaCode.ttf", 18 * _dpi_scale_x);
        io.Fonts->Build();

        ImGui_ImplGlfw_NewFrame();
    }

    void editor::run()
    {
        new_project();

        while (!glfwWindowShouldClose(_window.get()))
        {
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

            process_main_menu();

            if (_palette_injector && !_palette_injector->display())
            {
                _palette_injector.reset();
            }

            if (_image_converter && !_image_converter->display())
            {
                _image_converter.reset();
            }

            if (_shape_editor_tool && !_shape_editor_tool->display())
            {
                _shape_editor_tool.reset();
            }

            _location_browser->display();
            _scene_editor->display(_location_browser->get_selected_location(), _location_browser->get_selected_scene());
            _script_editor->display(_monospaced_font);
            _dialogue_editor->display();
            _shapes_editor->display(_location_browser->get_selected_location());
            _speaker_editor->display();
            _string_table->display();
            _manifest_editor->display();

            if (_show_properties_popup)
            {
                ImGui::OpenPopup("Properties");

                ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                if (ImGui::BeginPopupModal("Properties"))
                {
                    if (auto table = imgui::table("properties", 2, ImGuiTableFlags_SizingStretchProp))
                    {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::AlignTextToFramePadding();
                        ImGui::TextUnformatted("Shapes Start Id:");

                        ImGui::TableNextColumn();
                        ImGui::SetNextItemWidth(100);
                        ImGui::InputInt(make_id("##{}", _game_data->shape_start_id), &_game_data->shape_start_id);

                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::AlignTextToFramePadding();
                        ImGui::TextUnformatted("Autosave on Export:");

                        ImGui::TableNextColumn();
                        ImGui::Checkbox(make_id("##{}", _game_data->save_on_export), &_game_data->save_on_export);
                        
                    }

                    ImGui::NewLine();
                    auto width = ImGui::GetWindowWidth();
                    auto size = ImVec2{ 120.f * _dpi_scale_x, 0.f };

                    ImGui::Dummy({ width - size.x - 25.f, 0.f });
                    ImGui::SameLine();
                    if (ImGui::Button("Close", size))
                    {
                        _show_properties_popup = false;
                        ImGui::CloseCurrentPopup(); 
                    }

                    ImGui::EndPopup();
                }
            }

            if (_show_error_popup)
            {
                ImGui::OpenPopup("Error");
                ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                if (ImGui::BeginPopupModal("Error"))
                {
                    ImGui::Text(_error_message.c_str());

                    ImGui::NewLine();
                    auto width = ImGui::GetWindowWidth();
                    auto size = ImVec2{ 120.f * _dpi_scale_x, 0.f };

                    ImGui::Dummy({ width - size.x - 25.f, 0.f });
                    ImGui::SameLine();
                    if (ImGui::Button("Bummer", size))
                    {
                        _show_error_popup = false;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
            }

            // Vairn, Removed Demo Window.
            //ImGui::ShowDemoWindow();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(_window.get());
            glfwPollEvents();
        }
    }

    void editor::new_project()
    {
        _game_data = std::make_shared<game_data>();
        initialize_editors();
    }
    
    void editor::load_project(std::filesystem::path const& file_path)
    {
        _game_data = game_data::deserialize(file_path);
        _game_data->manifest.project_file = fs::relative(file_path);

        initialize_editors();
    }

    void editor::initialize_editors()
    {
        _location_browser = std::make_unique<location_browser>(_game_data, _window.get());
        _scene_editor = std::make_unique<scene_editor>(_game_data, _window.get());
        _script_editor = std::make_unique<script_editor>(_game_data, _window.get());
        _dialogue_editor = std::make_unique<dialogue_editor>(_game_data, _window.get());
        _shapes_editor = std::make_unique<shapes_editor>(_game_data, _window.get());
        _speaker_editor = std::make_unique<speaker_editor>(_game_data, _window.get());
        _string_table = std::make_unique<string_table_editor>(_game_data, _window.get());
        _manifest_editor = std::make_unique<manifest_editor>(_game_data, _window.get());

        if (_game_data->script_name != "")
        {
            _script_editor->load_script(_game_data->script_name);
        }
    }

    void editor::export_neon_file()
    {
        if (_game_data->manifest.game_file.empty())
        {
            auto file = save_file_dialog("neon");
            if (!file) return;

            _game_data->manifest.game_file = fs::relative(file.value()).string();
        }

        try
        {
            auto result = _script_editor->compile();
            if (_game_data->save_on_export)
            {
                save_project();
                _script_editor->save_script(_game_data->script_name);
            }
            serialize_to_neon_pak(_game_data->manifest.game_file, _game_data, result);

        }
        catch (std::exception const& ex)
        {
            std::cout << ex.what();
            _show_error_popup = true;
            _error_message = ex.what();
        }
    }

    void editor::save_project()
    {
        if (_game_data->manifest.project_file == "")
        {
            auto file = save_file_dialog("json");
            if (file)
            {
                _game_data->manifest.project_file = file.value();
            }
        }

        if (_game_data->manifest.project_file != "")
        {
            _game_data->serialize(_game_data->manifest.project_file);
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
                    new_project();
                }

                if (ImGui::MenuItem("Open", "Ctrl+O")) 
                {
                    auto file = open_file_dialog("json");
                    if (file)
                    {
                        try
                        {
                            load_project(file.value().data());
                        }
                        catch (std::exception const& ex)
                        {
                            std::cout << ex.what();
                            _show_error_popup = true;
                            _error_message = ex.what();
                        }
                    }
                }

                if (!_game_data) ImGui::BeginDisabled();
                if (ImGui::MenuItem("Save", "Ctrl+S")) 
                {
                    save_project();
                }
                if (ImGui::MenuItem("Save As..."))
                {
                    auto file = save_file_dialog("json");
                    if (file.has_value())
                    {
                        _game_data->manifest.project_file = file.value();
                        _game_data->serialize(_game_data->manifest.project_file);
                    }
                }
                if (!_game_data) ImGui::EndDisabled();

                ImGui::Separator();
                if (!_game_data) ImGui::BeginDisabled();
                if (ImGui::MenuItem("Properties..."))
                {
                    _show_properties_popup = true;
                }

                ImGui::Separator();
                if (ImGui::MenuItem("Export NEON file"))
                {
                    export_neon_file();
                }
                if (ImGui::MenuItem("Export NEON file As..."))
                {

                    auto file = save_file_dialog("neon");
                    if (file)
                    {
                        _game_data->manifest.game_file = fs::relative(file.value()).string();
                        export_neon_file();
                    }
                }
                if (!_game_data) ImGui::EndDisabled();

                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Alt+F4"))
                {
                    glfwSetWindowShouldClose(_window.get(), GLFW_TRUE);
                }

                ImGui::EndMenu();
            }
            //if (ImGui::BeginMenu("Edit"))
            //{
            //    if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            //    if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
            //    ImGui::Separator();
            //    if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            //    if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            //    if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            //    ImGui::EndMenu();
            //}
            if (ImGui::BeginMenu(ICON_MD_CONSTRUCTION " Tools"))
            {
                if (ImGui::MenuItem(ICON_MD_PALETTE " Palette Injector..."))
                {
                    if (!_palette_injector)
                    {
                        _palette_injector = std::make_unique<palette_injector>();
                    }
                    _palette_injector->display();
                }

                if (ImGui::MenuItem(ICON_MD_IMAGE " Image Converter..."))
                {
                    if (!_image_converter)
                    {
                        _image_converter = std::make_unique<image_converter>();
                    }
                    _image_converter->display();
                }

                if (ImGui::MenuItem(ICON_MD_CROP " Shape Editor..."))
                {
                    if (!_shape_editor_tool)
                    {
                        _shape_editor_tool = std::make_unique<shape_editor_tool>();
                    }
                    _shape_editor_tool->display();
                }

                if (ImGui::MenuItem(ICON_MD_FILE_DOWNLOAD " Lang pack to CSV"))
                {
                    auto file = open_file_dialog("noir");
                    if (file.has_value())
                    {
                        lang_pack_to_csv(file.value());
                    }
                }

                if (ImGui::MenuItem(ICON_MD_FILE_UPLOAD " CSV to Lang pack"))
                {
                    auto file = open_file_dialog("noir");
                    if (file.has_value())
                    {
                        csv_to_lang_pack(file.value());
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
}