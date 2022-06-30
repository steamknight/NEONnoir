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

namespace fs = std::filesystem;

namespace NEONnoir
{
    const TextEditor::LanguageDefinition& NEONscript_language_definition()
    {
        static bool inited = false;
        static TextEditor::LanguageDefinition langDef;
        if (!inited)
        {
            static const char* const keywords[] = {
                ".FLAGS", ".CONST", ".TEXT", ".SCRIPT"
            };

            for (auto& k : keywords)
                langDef.mKeywords.insert(k);

            static const char* const identifiers[] = {
                "noop", "clear", "set", "load", "store", "and", "or", "not", "jump", "jift", "jiff", 
                "setbg", "draw", "hasi ", "addi", "remi", "goto", "map", "text", "gameover"
            };
            for (auto& k : identifiers)
            {
                TextEditor::Identifier id;
                id.mDeclaration = "Built-in function";
                langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
            }

            langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("L?\\\"(\\\\.|[^\\\"])*\\\"", TextEditor::PaletteIndex::String));
            langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("\\\'[^\\\']*\\\'", TextEditor::PaletteIndex::String));
            langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?", TextEditor::PaletteIndex::Number));
            langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?", TextEditor::PaletteIndex::Number));
            langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("[+-]?[0-9]+[Uu]?[lL]?[lL]?", TextEditor::PaletteIndex::Number));
            langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("[a-zA-Z_][a-zA-Z0-9_]*", TextEditor::PaletteIndex::Identifier));
            langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", TextEditor::PaletteIndex::Punctuation));

            langDef.mSingleLineComment = ";";
            langDef.mCommentStart = "/*";
            langDef.mCommentEnd = "*/";

            langDef.mCaseSensitive = true;
            langDef.mAutoIndentation = false;

            langDef.mName = "NEONscript";

            inited = true;
        }
        return langDef;
    }

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

        // Add icons to the ui font
        auto config = ImFontConfig{};
        config.MergeMode = true;
        config.PixelSnapH = true;
        config.GlyphOffset = { 0, 4 };
        config.GlyphMinAdvanceX = 18;
        ImWchar const icon_range[] = { ICON_MIN_MD, ICON_MAX_16_MD, 0 };
        io.Fonts->AddFontFromFileTTF("data/MaterialIcons-Regular.ttf" , 18, &config, icon_range);
        io.Fonts->Build();

        _monospaced_font = io.Fonts->AddFontFromFileTTF("data/CascadiaCode.ttf", 18);
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

            _location_browser.display();

            _scene_editor.use(_game_data,
                _location_browser.get_selected_location_index(),
                _location_browser.get_selected_scene());

            _scene_editor.display();

            _script_editor.display(_script, _monospaced_font);

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

                if (ImGui::MenuItem("Open", "Ctrl+O")) 
                {
                    auto file = open_file_dialog("json");
                    if (file)
                    {
                        _game_data = game_data::deserialize(file.value().data());
                        _location_browser.use(_game_data);
                    }
                }

                if (!_game_data) ImGui::BeginDisabled();
                if (ImGui::MenuItem("Save", "Ctrl+S")) 
                {
                    auto file = save_file_dialog("json");
                    if (file.has_value())
                    {
                        _game_data->serialize(file.value().data());
                    }
                }
                if (!_game_data) ImGui::EndDisabled();

                ImGui::Separator();
                if (ImGui::MenuItem("Export NEON file..."))
                {
                    try
                    {
                        auto result = _script_editor.compile();
                        auto file = save_file_dialog("neon");
                        if (file.has_value())
                        {
                            serialize_to_neon_pak(file.value(), _game_data, result);
                        }
                    }
                    catch (std::exception const& ex)
                    {
                        std::cout << ex.what();
                    }
                }

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