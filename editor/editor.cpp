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
    void setup_theme()
    {
        // Rounded Visual Studio style by RedNicStone from ImThemes
        ImGuiStyle& style = ImGui::GetStyle();

        style.Alpha = 1.0f;
        style.DisabledAlpha = 0.6000000238418579f;
        style.WindowPadding = ImVec2(8.0f, 8.0f);
        style.WindowRounding = 4.0f;
        style.WindowBorderSize = 1.0f;
        style.WindowMinSize = ImVec2(32.0f, 32.0f);
        style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
        style.WindowMenuButtonPosition = ImGuiDir_Left;
        style.ChildRounding = 0.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupRounding = 4.0f;
        style.PopupBorderSize = 1.0f;
        style.FramePadding = ImVec2(4.0f, 3.0f);
        style.FrameRounding = 2.5f;
        style.FrameBorderSize = 0.0f;
        style.ItemSpacing = ImVec2(8.0f, 4.0f);
        style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
        style.CellPadding = ImVec2(4.0f, 2.0f);
        style.IndentSpacing = 21.0f;
        style.ColumnsMinSpacing = 6.0f;
        style.ScrollbarSize = 11.0f;
        style.ScrollbarRounding = 2.5f;
        style.GrabMinSize = 10.0f;
        style.GrabRounding = 2.0f;
        style.TabRounding = 3.5f;
        style.TabBorderSize = 0.0f;
        style.TabMinWidthForCloseButton = 0.0f;
        style.ColorButtonPosition = ImGuiDir_Right;
        style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
        style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

        style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
        style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.5921568870544434f, 0.5921568870544434f, 0.5921568870544434f, 1.0f);
        style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
        style.Colors[ImGuiCol_ChildBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
        style.Colors[ImGuiCol_PopupBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
        style.Colors[ImGuiCol_Border] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
        style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
        style.Colors[ImGuiCol_TitleBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
        style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
        style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
        style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.321568638086319f, 0.321568638086319f, 0.3333333432674408f, 1.0f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.3529411852359772f, 0.3529411852359772f, 0.3725490272045135f, 1.0f);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.3529411852359772f, 0.3529411852359772f, 0.3725490272045135f, 1.0f);
        style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
        style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
        style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
        style.Colors[ImGuiCol_Button] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
        style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
        style.Colors[ImGuiCol_Header] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
        style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
        style.Colors[ImGuiCol_Separator] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
        style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.3058823645114899f, 0.3058823645114899f, 0.3058823645114899f, 1.0f);
        style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
        style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2156862765550613f, 1.0f);
        style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.321568638086319f, 0.321568638086319f, 0.3333333432674408f, 1.0f);
        style.Colors[ImGuiCol_Tab] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
        style.Colors[ImGuiCol_TabHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
        style.Colors[ImGuiCol_TabActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
        style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
        style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
        style.Colors[ImGuiCol_PlotLines] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
        style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
        style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.1137254908680916f, 0.5921568870544434f, 0.9254902005195618f, 1.0f);
        style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
        style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.3098039329051971f, 0.3098039329051971f, 0.3490196168422699f, 1.0f);
        style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.2274509817361832f, 0.2274509817361832f, 0.2470588237047195f, 1.0f);
        style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.05999999865889549f);
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0f, 0.4666666686534882f, 0.7843137383460999f, 1.0f);
        style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
        style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.699999988079071f);
        style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.800000011920929f, 0.800000011920929f, 0.800000011920929f, 0.2000000029802322f);
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.1450980454683304f, 0.1450980454683304f, 0.1490196138620377f, 1.0f);
    }

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

        glfwSetWindowContentScaleCallback(_window.get(), [](GLFWwindow* window, float xscale, float /*yscale*/)
            {
                auto& style = ImGui::GetStyle();
                style.ScaleAllSizes(xscale);

                auto engine_ptr = force_to<editor*>(glfwGetWindowUserPointer(window));
                engine_ptr->load_fonts();
            });

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
 
        auto primary_monitor = glfwGetPrimaryMonitor();
        float xscale, yscale;
        glfwGetMonitorContentScale(primary_monitor, &xscale, &yscale);

        auto& style = ImGui::GetStyle();
        style.ScaleAllSizes(xscale);

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

        setup_theme();
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
        io.FontDefault = _ui_font;

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
            _scene_editor->display(_location_browser->get_selected_location(), _location_browser->get_selected_scene(), &_game_data->manifest.assets);

            if (_show_script_editor)
            {
                _script_editor->display(_monospaced_font);
            }

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
            ImGui::ShowDemoWindow();

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
            if (ImGui::BeginMenu("View"))
            {
                auto const show_script_editor_label = (_show_script_editor)
                    ? ICON_MD_CHECK " Script Editor"
                    : " Script Editor";

                if (ImGui::MenuItem(show_script_editor_label)) 
                {
                    _show_script_editor = !_show_script_editor;
                }

                ImGui::EndMenu();
            }
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