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

    this->initTextureManager();
    this->initWindow();
    this->initLayers();
    this->initEventHandler();
}

void State::initWindow()
{
    WindowConfig windowConfig = WindowConfig(
        Settings::Window::windowSize, Settings::Window::windowOrigin,
        Settings::App::appName, &this->textureManager.backgroundTexture,
        Settings::Window::clearColour
    );
    this->window.init(&windowConfig);
}

void State::initTextureManager()
{
    TextureManagerConfig textureManagerConfig = TextureManagerConfig();
    this->textureManager.init(&textureManagerConfig);
}

void State::initAppLayer()
{
    AppLayerConfig appLayerConfig =
        AppLayerConfig(&this->window, &this->textureManager);
    std::unique_ptr<AppLayer> appLayer = std::make_unique<AppLayer>();
    appLayer->init(&appLayerConfig);
    this->appLayer = std::move(appLayer);
}

void State::initPlayer(sf::Vector2f startingPosition)
{
    ShipActorBuilderContext initialPlayerState = ShipActorBuilderContext(
        &textureManager.defaultShipTexture, startingPosition, {0, -1},
        ShipTurnDirection::NONE
    );
    this->appLayer->addPlayer(&initialPlayerState);
    this->eventHandler.addPlayerHandle(appLayer->getPlayer());

    if (!isServer)
        this->appLayer->initPlayerCamera();
}

void State::initDebugLayer()
{
    DebugLayerConfig debugLayerConfig = DebugLayerConfig(&window);
    std::unique_ptr<DebugLayer> debugLayer = std::make_unique<DebugLayer>(
            this->appLayer->actors.size(),
            this->appLayer->ships.size(),
            this->appLayer->projectiles.size()
            );
    debugLayer->init(&debugLayerConfig);
    this->debugLayer = std::move(debugLayer);
}

void State::initLayers()
{
    this->initAppLayer();
    this->initDebugLayer();
}

void State::initEventHandler()
{
    EventHandlerConfig eventHandlerConfig = EventHandlerConfig(
        &this->window, &this->textureManager, this->appLayer.get(),
        this->isServer
    );
    eventHandler.init(&eventHandlerConfig);
}
