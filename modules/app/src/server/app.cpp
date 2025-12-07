#include "networking/server.h"

#include "../shared/config/settings.h"
#include "../shared/config/state.h"

#include <SFML/Network.hpp>
#include <spdlog/spdlog.h>

int main()
{
    spdlog::set_level(spdlog::level::info);
    bool isServer = true;

    spdlog::info("Running " + Settings::App::appName);

    StateConfig stateConfig = StateConfig(isServer);
    State state = State();
    state.init(&stateConfig);

    ServerConfig serverConfig = ServerConfig(
        Settings::Networking::serverPort, &state.appLayer->deltaTime,
        &state.appLayer->totalTime
    );
    Server server = Server();
    server.init(&serverConfig);
    if (!server.checkListenAvailable())
        return -1;
    state.debugLayer->server = &server;

    state.debugLayer->clientConnections = &server.clientConnections;
    // server specific networking
    while (state.window.window.isOpen())
    {
        // network
        // TODO - consolidate as server.frame
        server.frameClientConnections();
        server.acceptPlayerConnections();
        server.sendPlayerPingCheckMessages();
        server.scanSelector();
        server.checkForSyncs();

        std::vector<std::unique_ptr<Message>> clientUpdates =
            server.receiveDataFromClients();
        clientUpdates = server.processClientUpdates(std::move(clientUpdates));
        state.appLayer->networkUpdate(clientUpdates, server.clientConnections);

        std::vector<std::unique_ptr<Message>> gameUpdates =
            server.prepareGameUpdates(state.appLayer.get());
        for (std::unique_ptr<Message> &gameUpdate : gameUpdates)
        {
            clientUpdates.push_back(std::move(gameUpdate));
        }
        server.sendRelayNetworkUpdates(clientUpdates);
        server.sendNonBlockingPackets();

        // app
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
