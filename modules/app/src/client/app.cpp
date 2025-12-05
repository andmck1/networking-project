#include "networking/client.h"
#include "../shared/config/settings.h"
#include "../shared/config/state.h"

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <spdlog/spdlog.h>

int main()
{
    spdlog::set_level(spdlog::level::info);
    bool isServer = false;

    spdlog::info("Running " + Settings::App::appName);

    StateConfig stateConfig = StateConfig(isServer);
    State state = State();
    state.init(&stateConfig);

    ClientConfig clientConfig = ClientConfig(
        Settings::Networking::serverPort, Settings::Networking::playerPort,
        &state.clientUpdateRate, &state.appLayer->deltaTime,
        &state.appLayer->totalTime, sf::IpAddress::LocalHost
    );
    Client client = Client();
    client.init(&clientConfig);

    state.appLayer->isConnectedToServer = &client.isConnectedToServer;
    state.debugLayer->isClientConnectionEnabled = &client.isConnectionEnabled;

    while (!client.connectToServer())
    {
        state.eventHandler.handle();
        if (client.nServerConnectTries >
            Settings::Networking::maxClientConnectToServerTries)
        {
            spdlog::error("Failed to connect to server, max retries reached");
            return -1;
        }
        sf::Clock connectionTimer = sf::Clock();
        connectionTimer.start();
        while (connectionTimer.getElapsedTime().asSeconds() <
               Settings::Networking::clientConnectToServerWaitTime)
        {
            if (!state.eventHandler.handle())
                return 0;
        }
    }
    while (!client.syncWithServer())
    {
        state.eventHandler.handle();
    }
    state.appLayer->serverOffsetTime =
        sf::seconds(client.serverTime + client.ping);
    spdlog::info(
        std::format("Synced with server time: {:.2f}", client.serverTime)
    );

    sf::Vector2f playerStartingWorldPosition =
        static_cast<sf::Vector2f>(state.window.getRandomWindowPosition());
    playerStartingWorldPosition.x *=
        Settings::Camera::serverViewSize.x / Settings::Window::windowSize.x;
    playerStartingWorldPosition.y *=
        Settings::Camera::serverViewSize.y / Settings::Window::windowSize.y;
    state.initPlayer(playerStartingWorldPosition, client.serverId);
    state.debugLayer->player = state.appLayer->player;

    while (state.window.window.isOpen())
    {
        if (!client.isConnectionEnabled)
            client.disconnectFromServer();

        if (client.isConnectedToServer)
        {
            // TODO - have client.frame function
            client.sendNetworkUpdates(state.appLayer.get());
            std::vector<std::unique_ptr<Message>> networkUpdates =
                client.receiveNetworkUpdates();
            state.appLayer->networkUpdate(networkUpdates);
        }
        else
        {
            state.appLayer->clock.stop();
        }
        if (client.isConnectionEnabled && !client.isConnectedToServer)
        {
            // TODO - repeated code
            while (!client.connectToServer())
            {
                state.eventHandler.handle();
                if (client.nServerConnectTries >
                    Settings::Networking::maxClientReconnectToServerTries)
                {
                    spdlog::error(
                        "Failed to connect to server, max retries reached"
                    );
                    return -1;
                }
                sf::Clock connectionTimer = sf::Clock();
                connectionTimer.start();
                while (connectionTimer.getElapsedTime().asSeconds() <
                       Settings::Networking::clientReconnectToServerWaitTime)
                {
                    if (!state.eventHandler.handle())
                        return 0;
                }
            }
            while (!client.syncWithServer())
            {
                state.eventHandler.handle();
            }
            state.appLayer->serverOffsetTime =
                sf::seconds(client.serverTime + client.ping);
            spdlog::info(
                std::format(
                    "Synced with server time: {:.2f}", client.serverTime
                )
            );
            state.appLayer->clock.start();
        }

        if (client.isConnectedToServer)
        {
            state.window.clear();
            state.window.drawBackground();
        }
        state.eventHandler.handle();
        if (client.isConnectedToServer)
        {
            state.appLayer->render();
        }
        state.debugLayer->render();
        state.window.present();
    }
    return 0;
}
