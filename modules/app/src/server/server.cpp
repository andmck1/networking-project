#include "../shared/layer/app.h"
#include "../shared/layer/debug.h"
#include "../shared/config/settings.h"
#include "core_game/texturemanager/texturemanager.h"
#include "core_game/window/window.h"
#include "../shared/eventhandler/eventhandler.h"
#include <SFML/Network.hpp>

#include <spdlog/spdlog.h>

int main()
{
    spdlog::set_level(spdlog::level::debug);
    bool isServer = true;

    spdlog::info("Running " + Settings::App::appName);

    std::unique_ptr<WindowConfig> windowConfig = std::make_unique<WindowConfig>(
        Settings::Window::windowSize, Settings::Window::windowOrigin,
        Settings::App::appName
    );
    Window window = Window();
    window.init(std::move(windowConfig));

    sf::Vector2f playerStartingWorldPosition =
        static_cast<sf::Vector2f>(window.getWindowCentre());

    TextureManagerConfig textureManagerConfig = TextureManagerConfig();
    TextureManager textureManager = TextureManager(&textureManagerConfig);
    textureManager.init();

    AppLayerConfig appLayerConfig = AppLayerConfig(&window);
    AppLayer appLayer = AppLayer();
    appLayer.init(&appLayerConfig);

    ShipActorBuilderContext initialPlayerState = ShipActorBuilderContext(
        &textureManager.defaultShipTexture, playerStartingWorldPosition,
        {0, -1}, ShipTurnDirection::NONE
    );
    appLayer.addPlayer(&initialPlayerState);

    if (!isServer)
        appLayer.initPlayerCamera();

    DebugLayerConfig debugLayerConfig = DebugLayerConfig(&window);
    DebugLayer debugLayer = DebugLayer();
    debugLayer.init(&debugLayerConfig);

    EventHandlerConfig eventHandlerConfig = EventHandlerConfig(
        &window, appLayer.getPlayer(), &textureManager, &appLayer, isServer
    );
    EventHandler eventHandler = EventHandler();
    eventHandler.init(&eventHandlerConfig);

    std::vector<BaseLayer *> layers = {&appLayer, &debugLayer};

    spdlog::info("Running app");
    sf::Sprite backgroundSprite(textureManager.backgroundTexture);

    // server specific networking
    bool isConnected = false;
    unsigned short port = isServer ? Settings::Networking::serverPort
                                   : Settings::Networking::playerPort;
    sf::TcpListener listener;
    listener.setBlocking(false);
    if (listener.listen(port) == sf::Socket::Status::Error)
    {
        spdlog::error("Failed to connect to player");
    }
    std::unique_ptr<sf::TcpSocket> serverSocket =
        std::make_unique<sf::TcpSocket>();
    while (!isConnected)
    {
        spdlog::info("Connecting to player...");
        sf::Socket::Status status = listener.accept(*serverSocket);
        switch (status)
        {
            case (sf::Socket::Status::Done):
            {
                std::string connectionDetailsString =
                    serverSocket->getRemoteAddress()->toString() + ":" +
                    std::to_string(serverSocket->getRemotePort());
                spdlog::debug("New connection from " + connectionDetailsString);
                isConnected = true;
                break;
            }
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
