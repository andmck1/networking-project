#pragma once

// TODO - get rid of relative imports using include directories in cmake
#include "core_game/texturemanager/texturemanager.h"
#include "core_game/window/window.h"
#include "../../shared/actor/ship.h"
#include "../../shared/actor/projectile.h"
#include "../../shared/layer/app.h"

#include <SFML/Window/Event.hpp>
#include <optional>

struct EventHandlerConfig
{
    EventHandlerConfig(
        Window *window, ShipActor *player, TextureManager *textureManager,
        AppLayer *appLayer
    );
    ~EventHandlerConfig() = default;

    Window *window;
    ShipActor *player;
    TextureManager *textureManager;
    AppLayer *appLayer;
};

class EventHandler
{
    public:
    EventHandler() = default;
    ~EventHandler() = default;

    void init(EventHandlerConfig *config);
    bool handle();

    private:
    Window *window;
    ShipActor *player;
    TextureManager *textureManager;
    AppLayer *appLayer;
    bool handleInput(std::optional<sf::Event>);
    ProjectileActorBuilderContext createProjecileBuildContext(ShipActor *ship);
};
