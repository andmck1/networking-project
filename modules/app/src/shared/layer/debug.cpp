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
    Window *window, AppLayer *appLayer, bool isServer
)
{
    this->window = window;
    this->appLayer = appLayer;
    this->isServer = isServer;
}

void DebugLayer::init(BaseLayerConfig *config)
{
    spdlog::debug("Initialising debug layer");

    DebugLayerConfig *_config = static_cast<DebugLayerConfig *>(config);
    this->window = _config->window;
    this->appLayer = _config->appLayer;
    this->isServer = _config->isServer;

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
    {
        ImGui::Begin("Networking Project Debug: CLIENT");
        ImGui::TextWrapped(
            "To control the client, enable controls below and use WASD + "
            "Spacebar"
        );
    }

    ImGui::TextWrapped(
        "Current Game Time: %.2fs", this->appLayer->totalTime.asSeconds()
    );
    ImGui::TextWrapped(
        "Delta Time: %.4fs", this->appLayer->deltaTime.asSeconds()
    );
    ImGui::TextWrapped("N Actors: %d", (int) this->appLayer->actors.size());
    ImGui::TextWrapped("N Ships: %d", (int) this->appLayer->ships.size());
    ImGui::TextWrapped(
        "N Projectiles: %d", (int) this->appLayer->projectiles.size()
    );
    ImGui::Checkbox(
        "Enabled Boundary Boxes?", &this->appLayer->isBoundaryBoxesEnabled
    );
    if (this->player)
    {
        ImGui::Checkbox("Enable controls?", &this->player->isPlayerControlling);
    }
    if (this->isServer)
    {
        ImGui::Checkbox(
            "Enable Interpolation?", &this->server->isInterpolationEnabled
        );
        ImGui::Checkbox(
            "Enable Prediction?", &this->server->isPredictionEnabled
        );
    }
    else
    {
        ImGui::Checkbox(
            "Enable Client Connection?", this->isClientConnectionEnabled
        );
        ImGui::SliderFloat(
            "Client Send Interval (s/update)", &this->client->updateInterval,
            0.01f, 4.f
        );
    }
    if (ImGui::BeginTable("Ship Information", 6, flags))
    {
        ImGui::TableSetupColumn("Ship Handle");
        ImGui::TableSetupColumn("Position");
        ImGui::TableSetupColumn("Forward Vector");
        ImGui::TableSetupColumn("Health");
        ImGui::TableSetupColumn("Kills");
        ImGui::TableSetupColumn("Deaths");
        ImGui::TableHeadersRow();

        std::string cellText;
        for (const auto &ship : this->appLayer->ships)
        {
            if (ship)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextWrapped("%s", ship->handle.c_str());

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

                ImGui::TableSetColumnIndex(4);
                ImGui::TextWrapped("%d", ship->kills);

                ImGui::TableSetColumnIndex(5);
                ImGui::TextWrapped("%d", ship->deaths);
            }
        }
        ImGui::EndTable();
    }

    ImGui::Separator();

    if (this->clientConnections)
    {
        if (ImGui::BeginTable("Client Information", 8, flags))
        {
            ImGui::TableSetupColumn("Handle");
            ImGui::TableSetupColumn("Connection Details");
            ImGui::TableSetupColumn("Is Connected?");
            ImGui::TableSetupColumn("Ping (ms)");
            ImGui::TableSetupColumn("Newest game t (s)");
            ImGui::TableSetupColumn("Newest dt (s)");
            ImGui::TableSetupColumn("Estimated game time (s)");
            ImGui::TableSetupColumn("Estimated clock drift (s)");
            ImGui::TableHeadersRow();

            std::string cellText;
            for (const auto &clientConnection : *this->clientConnections)
            {
                if (clientConnection)
                {
                    ImGui::TableNextRow();
                    int columnIndex = 0;

                    ImGui::TableSetColumnIndex(columnIndex);
                    ImGui::TextWrapped("%s", clientConnection->handle.c_str());
                    columnIndex++;

                    ImGui::TableSetColumnIndex(columnIndex);
                    ImGui::TextWrapped(
                        "%s", clientConnection->connectionDetails.c_str()
                    );
                    columnIndex++;

                    ImGui::TableSetColumnIndex(columnIndex);
                    if (clientConnection->connectionState.connectionState ==
                        ClientConnectionState::ConnectionState::Disconnected)
                        ImGui::TextWrapped("false");
                    else
                        ImGui::TextWrapped("true");
                    columnIndex++;

                    ImGui::TableSetColumnIndex(columnIndex);
                    ImGui::TextWrapped(
                        "%.2f", clientConnection->averagePing * 1000.f
                    );
                    columnIndex++;

                    ImGui::TableSetColumnIndex(columnIndex);
                    ImGui::TextWrapped(
                        "%.3f", clientConnection->lastRecordedGameTime
                    );
                    columnIndex++;

                    ImGui::TableSetColumnIndex(columnIndex);
                    ImGui::TextWrapped(
                        "%.4f", clientConnection->lastRecordedDeltaTime
                    );
                    columnIndex++;

                    ImGui::TableSetColumnIndex(columnIndex);
                    ImGui::TextWrapped(
                        "%.3f", clientConnection->estimatedCurrentGameTime
                    );
                    columnIndex++;

                    ImGui::TableSetColumnIndex(columnIndex);
                    ImGui::TextWrapped(
                        "%.3f", clientConnection->lastRecordedClockDrift
                    );
                    columnIndex++;
                }
            }
            ImGui::EndTable();
        }
    }
    else
    {
        if (ImGui::BeginTable("Client Information", 7, flags))
        {
            ImGui::TableSetupColumn("Handle");
            ImGui::TableSetupColumn("Local Port");
            ImGui::TableSetupColumn("Server Details");
            ImGui::TableSetupColumn("Ping (ms)");
            ImGui::TableSetupColumn("Newest server t (s)");
            ImGui::TableSetupColumn("Newest server dt (s)");
            ImGui::TableSetupColumn("Estimated server t (s)");
            ImGui::TableHeadersRow();

            ImGui::TableNextRow();
            int columnIndex = 0;

            ImGui::TableSetColumnIndex(columnIndex);
            ImGui::TextWrapped("%s", this->client->handle.c_str());
            columnIndex++;

            ImGui::TableSetColumnIndex(columnIndex);
            ImGui::TextWrapped("%d", this->client->clientPort);
            columnIndex++;

            ImGui::TableSetColumnIndex(columnIndex);
            ImGui::TextWrapped(
                "%s:%d", this->client->serverIPAddress.toString().c_str(),
                this->client->serverPort
            );
            columnIndex++;

            ImGui::TableSetColumnIndex(columnIndex);
            ImGui::TextWrapped("%.2f", this->client->ping * 1000.f);
            columnIndex++;

            ImGui::TableSetColumnIndex(columnIndex);
            ImGui::TextWrapped("%.2f", this->client->lastRecordedServerTime);
            columnIndex++;

            ImGui::TableSetColumnIndex(columnIndex);
            ImGui::TextWrapped(
                "%.4f", this->client->lastRecordedServerDeltaTime
            );
            columnIndex++;

            ImGui::TableSetColumnIndex(columnIndex);
            ImGui::TextWrapped("%.4f", this->client->estimatedServerTime);
            columnIndex++;

            ImGui::EndTable();
        }
    }
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
};
