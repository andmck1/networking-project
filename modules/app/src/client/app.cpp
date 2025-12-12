#include "utils/cli.h"
#include "networking/client.h"
#include "../shared/config/settings.h"
#include "../shared/config/state.h"

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <spdlog/spdlog.h>


// TODO - tidy up
int main(int, char *argv[])
{
    spdlog::set_level(spdlog::level::info);

    Arguments arguments = Arguments();
    if (!arguments.init(argv))
        return -1;

    bool isServer = false;

    spdlog::info("Running " + Settings::App::appName);

    StateConfig stateConfig = StateConfig(isServer);
    State state = State();
    state.init(&stateConfig);

    ClientConfig clientConfig = ClientConfig(
        Settings::Networking::serverPort, Settings::Networking::playerPort,
        Settings::Networking::clientUpdateInterval, &state.appLayer->deltaTime,
        &state.appLayer->totalTime, arguments.handle, arguments.skinIndex,
        sf::IpAddress::LocalHost
    );
    Client client = Client();
    client.init(&clientConfig);

    state.appLayer->isConnectedToServer = &client.isConnectedToServer;
    state.debugLayer->isClientConnectionEnabled = &client.isConnectionEnabled;
    state.debugLayer->client = &client;

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
    spdlog::info("Starting at time: {}", state.appLayer->totalTime.asSeconds());

    sf::Vector2f playerStartingWorldPosition =
        state.appLayer->getRespawnPoint();
    state.initPlayer(
        playerStartingWorldPosition, client.handle, client.skinIndex
    );
    state.debugLayer->player = state.appLayer->player;

    while (state.window.window.isOpen())
    {
        client.frameServerTime();
        state.appLayer->totalTime = sf::seconds(client.estimatedServerTime);
        if (!client.isConnectionEnabled)
            client.disconnectFromServer();

        if (client.isConnectedToServer)
        {
            // TODO - have client.frame function
            client.sendNetworkUpdates(state.appLayer.get());
            client.sendNonBlockingPackets();
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
                if (client.nServerConnectTries >
                    Settings::Networking::maxClientConnectToServerTries)
                {
                    spdlog::error(
                        "Failed to connect to server, max retries reached"
                    );
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
            state.appLayer->clock.restart();
            state.appLayer->totalTime = sf::seconds(client.estimatedServerTime);
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
