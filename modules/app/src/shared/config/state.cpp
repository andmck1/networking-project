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
    this->clientUpdateRate = Settings::Networking::clientUpdateRate;

    this->initTextureManager();
    this->initWindow();
    this->initLayers();
    this->initEventHandler();
    if (this->isServer)
        this->appLayer->initServerCamera();
}

void State::initWindow()
{
    sf::Vector2u windowSize;
    if (this->isServer)
        windowSize = Settings::Window::serverWindowSize;
    else
        windowSize = Settings::Window::windowSize;
    WindowConfig windowConfig = WindowConfig(
        windowSize, Settings::Window::windowOrigin, Settings::App::appName,
        &this->textureManager.backgroundTexture, Settings::Window::clearColour
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
        AppLayerConfig(&this->window, &this->textureManager, this->isServer);
    std::unique_ptr<AppLayer> appLayer = std::make_unique<AppLayer>();
    appLayer->init(&appLayerConfig);
    this->appLayer = std::move(appLayer);
}

void State::initPlayer(sf::Vector2f startingPosition, int32_t serverId)
{

    int randomTextureIndex = this->textureManager.getRandomTextureIndex();

    ShipActorBuilderContext initialPlayerState = ShipActorBuilderContext(
        &textureManager.shipTextures[randomTextureIndex], startingPosition,
        {0, -1}, ShipTurnDirection::NONE, serverId
    );
    this->appLayer->addPlayer(&initialPlayerState);
    this->eventHandler.addPlayerHandle(appLayer->player);

    if (!isServer)
        this->appLayer->initPlayerCamera();
}

void State::initDebugLayer()
{
    DebugLayerConfig debugLayerConfig = DebugLayerConfig(
        &window, &this->appLayer->actors, &this->appLayer->ships,
        &this->appLayer->projectiles, &this->appLayer->isBoundaryBoxesEnabled,
        &this->clientUpdateRate, this->isServer, &this->appLayer->totalTime
    );
    std::unique_ptr<DebugLayer> debugLayer = std::make_unique<DebugLayer>();
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
