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
        sf::IpAddress::LocalHost
    );
    Client client = Client();
    client.init(&clientConfig);

    while (!client.connectToServer())
    {
        state.eventHandler.handle();
    }

    sf::Vector2f playerStartingWorldPosition =
        static_cast<sf::Vector2f>(state.window.getWindowCentre());
    state.initPlayer(playerStartingWorldPosition);

    while (state.window.window.isOpen())
    {
        client.sendNetworkUpdate(state.appLayer.get());

        state.window.clear();
        state.window.drawBackground();
        state.eventHandler.handle();
        state.appLayer->render();
        state.debugLayer->render();
        state.window.present();
    }
    return 0;
}
