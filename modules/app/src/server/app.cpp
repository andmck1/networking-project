#include "networking/server.h"

#include "../shared/config/settings.h"
#include "../shared/config/state.h"

#include <SFML/Network.hpp>
#include <spdlog/spdlog.h>

int main()
{
    // Init
    spdlog::set_level(spdlog::level::info);
    spdlog::info("Running " + Settings::App::appName);

    bool isServer = true;
    StateConfig stateConfig = StateConfig(isServer);
    State state = State();
    state.init(&stateConfig);

    ServerConfig serverConfig =
        ServerConfig(Settings::Networking::serverPort, state.appLayer.get());
    Server server = Server();
    server.init(&serverConfig);
    if (!server.checkListenAvailable())
        return -1;
    state.debugLayer->server = &server;
    state.debugLayer->clientConnections = &server.clientConnections;

    // Main loop
    while (state.window.window.isOpen())
    {
        // Client checks
        // TODO - consolidate as server.frame
        // TODO - server should have handle to app layer, or ships should have
        server.frameClientConnections();
        server.acceptPlayerConnections();
        server.sendPlayerPingCheckMessages();
        server.sendScoreUpdateMessages();
        server.scanSelector();
        server.checkForSyncs();

        // Network update
        std::vector<std::unique_ptr<Message>> clientUpdates =
            server.receiveDataFromClients();
        clientUpdates = server.processClientUpdates(std::move(clientUpdates));
        state.appLayer->networkUpdate(clientUpdates);
        server.updateShipStatesBasedOnConnectionStatus();
        std::vector<std::unique_ptr<Message>> gameUpdates =
            server.prepareGameUpdates();
        for (std::unique_ptr<Message> &gameUpdate : gameUpdates)
        {
            clientUpdates.push_back(std::move(gameUpdate));
        }

        // Network send
        server.sendRelayNetworkUpdates(clientUpdates);
        server.sendNonBlockingPackets();

        // App update
        // TODO - consolidate as state.frame
        state.window.clear();
        state.window.drawBackground();
        state.eventHandler.handle();
        state.appLayer->render();
        state.debugLayer->render();
        state.window.present();
    }
    return 0;
}
