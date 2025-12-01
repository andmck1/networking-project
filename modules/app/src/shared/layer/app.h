#pragma once

#include "core_game/window/window.h"
#include "layer.h"

#include "../actor/base.h"
#include "../actor/ship.h"

#include <vector>
#include <SFML/Graphics.hpp>

struct AppLayerConfig
{
    AppLayerConfig(Window *window);
    Window *window;
};

struct AppLayerState
{
    std::vector<std::unique_ptr<BaseActor>> actors;
    ShipActor *player;
    sf::Clock clock = sf::Clock();
    sf::Time deltaTime;
};

class AppLayer : public Layer
{
    private:
    AppLayerConfig *config;
    void startFrame();
    bool handleInput();
    void updateActors();
    void drawActors();

    public:
    AppLayer(AppLayerConfig *config);
    AppLayerState state;

    void addPlayer(std::unique_ptr<ShipActorState> initialPlayerState);
    template <typename ActorType, typename ActorStateType>
    void addActor(ActorStateType &&initialActorState);

    void init() override;
    bool render() override;
};
