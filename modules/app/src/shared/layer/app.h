#pragma once

#include "core_game/window/window.h"
#include "base.h"

#include "../actor/base.h"
#include "../actor/ship.h"

#include <vector>
#include <SFML/Graphics.hpp>

struct AppLayerConfig : BaseLayerConfig
{
    AppLayerConfig(Window *window);
    Window *window;
};

class AppLayer : public BaseLayer
{
    public:
    AppLayer() = default;
    ~AppLayer() = default;

    void init(BaseLayerConfig *config) override;
    bool render() override;

    void addPlayer(ShipActorBuilderContext *initialPlayerState);
    template <typename ActorType, typename ActorStateType>
    void addActor(ActorStateType *initialActorState);

    private:
    std::vector<std::unique_ptr<BaseActor>> actors;
    ShipActor *player;
    sf::Clock clock = sf::Clock();
    sf::Time deltaTime;
    Window *window;

    void startFrame();
    bool handleInput();
    void updateActors();
    void drawActors();
};
