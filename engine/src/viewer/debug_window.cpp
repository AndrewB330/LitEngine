#include <backends/imgui_impl_sdl.h>
#include <backends/imgui_impl_opengl3.h>
#include <lit/viewer/debug_window.hpp>
#include <lit/viewer/debug_options.hpp>

using namespace lit::viewer;

bool DebugUI::Init() {
    imgui_context = ImGui::CreateContext();

    IMGUI_CHECKVERSION();
    ImGuiIO &io = ImGui::GetIO();
    //io.Fonts->AddFontFromFileTTF("consolas.ttf", 14.0f);

    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(WindowRenderer::m_sdl_window, WindowRenderer::m_context);
    ImGui_ImplOpenGL3_Init();

    return true;
}

bool DebugUI::ProcessEvent(const SDL_Event &event) {
    if (event.type == SDL_MOUSEMOTION ||
        event.type == SDL_MOUSEBUTTONDOWN ||
        event.type == SDL_MOUSEBUTTONUP ||
        event.type == SDL_MOUSEWHEEL) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        return ImGui::GetIO().WantCaptureMouse;
    }
    if (event.type == SDL_KEYDOWN ||
        event.type == SDL_KEYUP ||
        event.type == SDL_TEXTINPUT) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        return ImGui::GetIO().WantCaptureKeyboard;
    }
    return false;
}

void DebugUI::Redraw() {
    fps_meter.FrameStart();

    int width, height;
    SDL_GetWindowSize(WindowRenderer::m_sdl_window, &width, &height);

    ImGui::SetCurrentContext(imgui_context);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(WindowRenderer::m_sdl_window);
    ImGui::NewFrame();

    // Logs
    ImGui::SetNextWindowSize({width - 16.f, 0});
    ImGui::SetNextWindowSizeConstraints({width - 16.f, 32}, {width - 16.f, 256});
    ImGui::SetNextWindowPos({8, height - 8.0f}, 0, {0.f, 1.f});
    ImGui::Begin("Logs", nullptr, ImGuiWindowFlags_NoResize);
    /*for (const auto &s : Logger::Instance().GetLastLogs(128)) {
        if (s.type == LogType::LOG_INFO)
            ImGui::Text("[INFO] %s", s.message.c_str());
        if (s.type == LogType::LOG_WARNING)
            ImGui::Text("[WARNING] %s", s.message.c_str());
        if (s.type == LogType::LOG_ERROR)
            ImGui::Text("[ERROR] %s", s.message.c_str());
    }*/
    ImGui::SetItemDefaultFocus();
    ImGui::End();

    // Fps
    ImGui::SetNextWindowSize({0, 0});
    ImGui::SetNextWindowSizeConstraints({128, 64}, {256, 512});
    ImGui::SetNextWindowPos({8, 8.0f}, 0, {0.f, 0.f});

    ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("FPS: %.02f", fps_meter.GetAverageFPS());
        ImGui::Text("Time: %.02f ms", fps_meter.GetAverageMS());
    }

    auto & dbg = DebugOptions::Instance();

    if (ImGui::Button("Recompile Shader")) {
        dbg.recompile_shaders = true;
    }

    /*

    if (ImGui::CollapsingHeader("Buffers", ImGuiTreeNodeFlags_DefaultOpen)) {
        int * option = (int*)&(options.draw_buffer_option);
        ImGui::RadioButton("Final", option, (int)DrawBufferOption::Final);
        ImGui::RadioButton("TAA", option, (int)DrawBufferOption::TAAOutput);
        ImGui::RadioButton("Color", option, (int)DrawBufferOption::Color);
        ImGui::RadioButton("Depth", option, (int)DrawBufferOption::Depth);
        ImGui::RadioButton("Normal", option, (int)DrawBufferOption::Normal);
        ImGui::RadioButton("Lighting", option, (int)DrawBufferOption::Lighting);
        ImGui::RadioButton("Denoised", option, (int)DrawBufferOption::DenoisedLighting);
        ImGui::RadioButton("Noise", option, (int)DrawBufferOption::Noise);
    }

    if (ImGui::CollapsingHeader("SkyBoxes", ImGuiTreeNodeFlags_DefaultOpen)) {
        int * option = (int*)&(options.sky_box_option);
        ImGui::RadioButton("Standard", option, (int)SkyBoxOption::Standard);
        ImGui::RadioButton("Sunset", option, (int)SkyBoxOption::Sunset);
        ImGui::RadioButton("Pink", option, (int)SkyBoxOption::Pink);
        ImGui::RadioButton("Deep Dusk", option, (int)SkyBoxOption::DeepDusk);
        ImGui::RadioButton("Space", option, (int)SkyBoxOption::Space);
    }

    if (ImGui::CollapsingHeader("Channels")) {
        int * option = (int*)&(options.draw_channel);
        ImGui::RadioButton("RGB", option, 0);
        ImGui::RadioButton("Red", option, 1);
        ImGui::RadioButton("Green", option, 2);
        ImGui::RadioButton("Blue", option, 3);
    }

    if (ImGui::CollapsingHeader("Other", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderInt("Zoom", &(options.zoom), 1, 8);
        ImGui::Checkbox("Jitter", &(options.jitter));
        ImGui::SliderFloat("Magic", &(options.magic), 0.1f, 3.0f);
        ImGui::SliderFloat("Gamma", &(options.gamma), 0.5f, 4.0f);
    }*/

    ImGui::End();

    ImGui::SetItemDefaultFocus();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void DebugUI::StartFrameEvent() {}
