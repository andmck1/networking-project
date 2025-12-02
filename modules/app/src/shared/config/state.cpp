#include "state.h"

#include "settings.h"
#include "../layer/app.h"
#include "../layer/debug.h"
#include "../eventhandler/eventhandler.h"
#include "core_game/texturemanager/texturemanager.h"
#include "core_game/window/window.h"
#include <SFML/Network.hpp>

#include <SFML/Network.hpp>
#include <spdlog/spdlog.h>

StateConfig::StateConfig(bool isServer)
{
    this->isServer = isServer;
}

void State::init(StateConfig *config)
{
    this->isServer = config->isServer;

    sf::Vector2f playerStartingWorldPosition =
        static_cast<sf::Vector2f>(window.getWindowCentre());

    DebugLayerConfig debugLayerConfig = DebugLayerConfig(&window);
    DebugLayer debugLayer = DebugLayer();
    debugLayer.init(&debugLayerConfig);

    EventHandlerConfig eventHandlerConfig = EventHandlerConfig(
        &window, appLayer.getPlayer(), &textureManager, &appLayer, isServer
    );
    EventHandler eventHandler = EventHandler();
    eventHandler.init(&eventHandlerConfig);

    std::vector<BaseLayer *> layers = {&appLayer, &debugLayer};
}

void State::initWindow()
{
    std::unique_ptr<WindowConfig> windowConfig = std::make_unique<WindowConfig>(
        Settings::Window::windowSize, Settings::Window::windowOrigin,
        Settings::App::appName
    );
    window.init(std::move(windowConfig));
}

void State::initTextureManager()
{
    TextureManagerConfig textureManagerConfig = TextureManagerConfig();
    this->textureManager.init(&textureManagerConfig);
}

void State::initAppLayer()
{
    AppLayerConfig appLayerConfig = AppLayerConfig(&window);
    std::unique_ptr<AppLayer> appLayer = std::make_unique<AppLayer>();
    appLayer->init(&appLayerConfig);
    this->appLayer = std::move(appLayer);

    this->layers.push_back(this->appLayer.get());
}

void State::initPlayer(sf::Vector2f startingPosition)
{
    ShipActorBuilderContext initialPlayerState = ShipActorBuilderContext(
        &textureManager.defaultShipTexture, startingPosition, {0, -1},
        ShipTurnDirection::NONE
    );
    appLayer.addPlayer(&initialPlayerState);

    if (!isServer)
        appLayer.initPlayerCamera();
}

void State::initDebugLayer()
{
}

void State::initLayers()
{
}

void State::initActors()
{
}

void State::initEventHandler()
{
}
