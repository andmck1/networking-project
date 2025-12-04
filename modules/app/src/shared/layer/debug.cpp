#include "debug.h"

#include <spdlog/spdlog.h>
#include <windows.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>

DebugLayer::DebugLayer()
{
}

DebugLayerConfig::DebugLayerConfig(
    Window *window, std::vector<std::unique_ptr<BaseActor>> *actors,
    std::vector<ShipActor *> *ships,
    std::vector<ProjectileActor *> *projectiles, bool *isBoundaryBoxesEnabled
)
{
    this->window = window;
    this->actors = actors;
    this->ships = ships;
    this->projectiles = projectiles;
    this->isBoundaryBoxesEnabled = isBoundaryBoxesEnabled;
}

void DebugLayer::init(BaseLayerConfig *config)
{
    spdlog::debug("Initialising debug layer");

    DebugLayerConfig *_config = static_cast<DebugLayerConfig *>(config);
    this->window = _config->window;
    this->actors = _config->actors;
    this->ships = _config->ships;
    this->projectiles = _config->projectiles;
    this->isBoundaryBoxesEnabled = _config->isBoundaryBoxesEnabled;

    ImGui_ImplWin32_EnableDpiAwareness();
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(
        ::MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY)
    );

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    ImGui::StyleColorsDark();
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    ImGui_ImplWin32_InitForOpenGL(this->window->window.getNativeHandle());
    ImGui_ImplOpenGL3_Init();
}

DebugLayer::~DebugLayer()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void DebugLayer::render()
{
    static ImGuiTableFlags flags =
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Networking Project Debug");
    ImGui::Text("N Actors: %d", (int) this->actors->size());
    ImGui::Text("N Ships: %d", (int) this->ships->size());
    ImGui::Text("N Projectiles: %d", (int) this->projectiles->size());
    ImGui::Checkbox("Enabled Boundary Boxes?", this->isBoundaryBoxesEnabled);
    if (ImGui::BeginTable("Ship Information", 4, flags))
    {
        ImGui::TableSetupColumn("Ship Server ID");
        ImGui::TableSetupColumn("Position");
        ImGui::TableSetupColumn("Forward Vector");
        ImGui::TableSetupColumn("Health");
        ImGui::TableHeadersRow();

        std::string cellText;
        for (const auto &ship : *this->ships)
        {
            if (ship)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                cellText = std::format("{}", ship->serverId);
                ImGui::TextUnformatted(cellText.c_str());

                ImGui::TableSetColumnIndex(1);
                cellText = std::format(
                    "{:.2f}, {:.2f}", ship->position.x, ship->position.y
                );
                ImGui::TextUnformatted(cellText.c_str());

                ImGui::TableSetColumnIndex(2);
                cellText = std::format(
                    "{:.2f}, {:.2f}", ship->forwardVector.x,
                    ship->forwardVector.y
                );
                ImGui::TextUnformatted(cellText.c_str());

                ImGui::TableSetColumnIndex(3);
                cellText = std::format("{}", ship->health);
                ImGui::TextUnformatted(cellText.c_str());
            }
        }
        ImGui::EndTable();
    }
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
};
