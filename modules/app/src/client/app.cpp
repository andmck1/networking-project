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
    spdlog::info("Running " + Settings::App::appName);

    // CLI
    Arguments arguments = Arguments();
    if (!arguments.init(argv))
        return -1;

    // Init
    bool isServer = false;

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

    // Connect to server
    client.connectAndSync(
        &state.eventHandler,
        Settings::Networking::maxClientConnectToServerTries,
        Settings::Networking::clientConnectToServerWaitTime
    );

    spdlog::info("Starting at time: {}", state.appLayer->totalTime.asSeconds());

    // Init player
    sf::Vector2f playerStartingWorldPosition =
        state.appLayer->getRespawnPoint();
    state.initPlayer(
        playerStartingWorldPosition, client.handle, client.skinIndex
    );
    state.debugLayer->player = state.appLayer->player;

    // Main loop
    while (state.window.window.isOpen())
    {
        // Update time
        client.frameServerTime();
        state.appLayer->totalTime = sf::seconds(client.estimatedServerTime);

        // Disconnection debug flag
        if (!client.isConnectionEnabled)
            client.disconnectFromServer();

        // If connected to server, run network update, otherwise stop running
        // time
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

        // If expect to be connected but aren't, try and reconnect
        if (client.isConnectionEnabled && !client.isConnectedToServer)
        {

            client.connectAndSync(
                &state.eventHandler,
                Settings::Networking::maxClientReconnectToServerTries,
                Settings::Networking::clientReconnectToServerWaitTime
            );
            state.appLayer->clock.restart();
            state.appLayer->totalTime = sf::seconds(client.estimatedServerTime);
        }

        // Only draw and update if connected to server, but
        // always handle keyboard input, draw debug layer and present window
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
