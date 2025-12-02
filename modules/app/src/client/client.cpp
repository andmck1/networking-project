#include "../shared/layer/app.h"
#include "../shared/layer/debug.h"
#include "../shared/eventhandler/eventhandler.h"
#include "../shared/config/settings.h"
#include "core_game/texturemanager/texturemanager.h"
#include "core_game/window/window.h"
#include <SFML/Network.hpp>

#include <SFML/Network.hpp>
#include <spdlog/spdlog.h>

int main()
{
    // Likely abstracted start
    spdlog::set_level(spdlog::level::debug);
    bool isServer = false;

    spdlog::info("Running " + Settings::App::appName);

    spdlog::info("Running app");
    sf::Sprite backgroundSprite(textureManager.backgroundTexture);

    // client specific networking
    bool isConnected = false;
    unsigned short port = isServer ? Settings::Networking::serverPort
                                   : Settings::Networking::playerPort;
    sf::TcpListener listener;
    listener.setBlocking(false);
    if (listener.listen(port) == sf::Socket::Status::Error)
    {
        spdlog::error("Failed to connect to server");
    }
    std::unique_ptr<sf::TcpSocket> playerSocket =
        std::make_unique<sf::TcpSocket>();
    while (!isConnected)
    {
        spdlog::info("Connecting to server...");
        sf::Socket::Status status = playerSocket->connect(
            sf::IpAddress::LocalHost, Settings::Networking::serverPort
        );
        switch (status)
        {
            case (sf::Socket::Status::Done):
            {
                spdlog::debug("Connected to server!");
                isConnected = true;
                break;
            }
            case (sf::Socket::Status::NotReady):
                break;
            default:
            {
                spdlog::error("Unexpected error connecting to server");
            }
        }
        eventHandler.handle();
    }

    while (window.window.isOpen())
    {
        eventHandler.handle();
        window.clear(Settings::Window::clearColour);
        window.window.draw(backgroundSprite);
        for (BaseLayer *layer : layers)
            layer->render();
        window.present();
    }
    return 0;
}
