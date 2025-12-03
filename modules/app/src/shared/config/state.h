#pragma once

#include "../layer/app.h"
#include "../layer/debug.h"
#include "../eventhandler/eventhandler.h"
#include "core_game/texturemanager/texturemanager.h"
#include "core_game/window/window.h"
#include <SFML/Network.hpp>

struct StateConfig
{
    public:
    StateConfig(bool isServer);
    ~StateConfig() = default;

    bool isServer;
};

class State
{
    public:
    State() = default;
    ~State() = default;
    bool isServer;

    Window window = Window();
    EventHandler eventHandler = EventHandler();
    TextureManager textureManager = TextureManager();
    std::unique_ptr<AppLayer> appLayer;
    std::unique_ptr<DebugLayer> debugLayer;

    void init(StateConfig *config);
    void initPlayer(sf::Vector2f startingPosition);

    private:
    void initWindow();
    void initTextureManager();
    void initAppLayer();
    void initDebugLayer();
    void initLayers();
    void initEventHandler();
};
