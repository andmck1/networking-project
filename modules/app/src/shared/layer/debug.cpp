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
    std::vector<ProjectileActor *> *projectiles, bool *isBoundaryBoxesEnabled,
    float *clientUpdateRate, bool isServer, sf::Time *currentGameTime
)
{
    this->window = window;
    this->actors = actors;
    this->ships = ships;
    this->projectiles = projectiles;
    this->isBoundaryBoxesEnabled = isBoundaryBoxesEnabled;
    this->clientUpdateRate = clientUpdateRate;
    this->isServer = isServer;
    this->currentGameTime = currentGameTime;
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
    this->clientUpdateRate = _config->clientUpdateRate;
    this->isServer = _config->isServer;
    this->currentGameTime = _config->currentGameTime;

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
    if (this->isServer)
        ImGui::Begin("Networking Project Debug: SERVER");
    else
        ImGui::Begin("Networking Project Debug: CLIENT");

    ImGui::TextWrapped(
        "Current Game Time: %.2fs", this->currentGameTime->asSeconds()
    );
    ImGui::TextWrapped("N Actors: %d", (int) this->actors->size());
    ImGui::TextWrapped("N Ships: %d", (int) this->ships->size());
    ImGui::TextWrapped("N Projectiles: %d", (int) this->projectiles->size());
    ImGui::Checkbox("Enabled Boundary Boxes?", this->isBoundaryBoxesEnabled);
    if (this->player)
    {
        ImGui::Checkbox("Enable WASD?", &this->player->isPlayerControlling);
    }
    if (!this->isServer)
    {
        ImGui::Checkbox(
            "Enable Client Connection?", this->isClientConnectionEnabled
        );
        ImGui::SliderFloat(
            "Client Send Rate (s/update)", this->clientUpdateRate, 0.f, 2.f
        );
    }
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
                ImGui::TextWrapped("%d", ship->serverId);

                ImGui::TableSetColumnIndex(1);
                ImGui::TextWrapped(
                    "%.2f, %.2f", ship->position.x, ship->position.y
                );

                ImGui::TableSetColumnIndex(2);
                ImGui::TextWrapped(
                    "%.2f, %.2f", ship->forwardVector.x, ship->forwardVector.y
                );

                ImGui::TableSetColumnIndex(3);
                ImGui::TextWrapped("%d", ship->health);
            }
        }
        ImGui::EndTable();
    }

    ImGui::Separator();

    if (this->clients)
    {
        if (ImGui::BeginTable("Client Information", 5, flags))
        {
            ImGui::TableSetupColumn("ID");
            ImGui::TableSetupColumn("Connection Details");
            ImGui::TableSetupColumn("Ping (ms)");
            ImGui::TableSetupColumn("Client Time delta with Server Time");
            ImGui::TableSetupColumn("Is Connected?");
            ImGui::TableHeadersRow();

            std::string cellText;
            for (const auto &clientConnection : *this->clients)
            {
                if (clientConnection)
                {
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextWrapped("%d", clientConnection->id);

                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextWrapped(
                        "%s", clientConnection->connectionDetails.c_str()
                    );

                    ImGui::TableSetColumnIndex(2);
                    ImGui::TextWrapped(
                        "%.2f", clientConnection->averagePing * 1000.f
                    );

                    ImGui::TableSetColumnIndex(3);
                    ImGui::TextWrapped(
                        "%.3f", clientConnection->deltaWithServer
                    );

                    ImGui::TableSetColumnIndex(4);
                    if (clientConnection->connectionState.connectionState ==
                        ClientConnectionState::ConnectionState::Disconnected)
                        ImGui::TextWrapped("false");
                    else
                        ImGui::TextWrapped("true");
                }
            }
        }
        ImGui::EndTable();
    }
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
};
