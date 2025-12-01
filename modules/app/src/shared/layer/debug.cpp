#include "debug.h"

#include <spdlog/spdlog.h>
#include <windows.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>

DebugLayerConfig::DebugLayerConfig(Window *window)
{
    this->window = window;
}

DebugLayer::DebugLayer(DebugLayerConfig *config)
{
    this->config = config;
}

DebugLayer::~DebugLayer()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void DebugLayer::init()
{
    spdlog::debug("Initialising debug layer");
    ImGui_ImplWin32_EnableDpiAwareness();
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(
        ::MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY)
    );

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    ImGui_ImplWin32_InitForOpenGL(
        this->config->window->window.getNativeHandle()
    );
    ImGui_ImplOpenGL3_Init();
}

bool DebugLayer::render()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Hello, world!");
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    return true;
};
