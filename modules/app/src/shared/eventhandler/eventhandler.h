#pragma once

// TODO - get rid of relative imports using include directories in cmake
#include "core_game/texturemanager/texturemanager.h"
#include "core_game/window/window.h"
#include "../../shared/actor/ship.h"
#include "../../shared/layer/app.h"

#include <SFML/Window/Event.hpp>
#include <optional>

struct EventHandlerConfig
{
    EventHandlerConfig(
        Window *window, TextureManager *textureManager, AppLayer *appLayer,
        bool isServer
    );
    ~EventHandlerConfig() = default;

    Window *window;
    TextureManager *textureManager;
    AppLayer *appLayer;
    bool isServer;
};

class EventHandler
{
    public:
    EventHandler() = default;
    ~EventHandler() = default;

    void init(EventHandlerConfig *config);
    void addPlayerHandle(ShipActor *player);
    bool handle();

    private:
    Window *window;
    std::optional<ShipActor *> player;
    TextureManager *textureManager;
    AppLayer *appLayer;
    bool isServer;
    float fireTimer = 0.f;

    bool handleInput();
    void handleDebugInput(std::optional<sf::Event> event);
};
