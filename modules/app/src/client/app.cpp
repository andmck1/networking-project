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

    while (!client.connectToServer())
    {
        state.eventHandler.handle();
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
        client.sendNetworkUpdates(state.appLayer.get());
        std::vector<std::unique_ptr<Message>> networkUpdates =
            client.receiveNetworkUpdates();
        state.appLayer->networkUpdate(networkUpdates);

        state.window.clear();
        state.window.drawBackground();
        state.eventHandler.handle();
        state.appLayer->render();
        state.debugLayer->render();
        state.window.present();
    }
    return 0;
}
