#pragma once

#include "base.h"
#include "core_game/window/window.h"
#include "../actor/base.h"
#include "../actor/ship.h"
#include "../actor/projectile.h"

#include <SFML/Graphics.hpp>

#include <vector>
#include <optional>

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
    void render() override;

    void initPlayerCamera();
    void addPlayer(ShipActorBuilderContext *initialPlayerState);
    void addProjectile(ProjectileActorBuilderContext *initialProjectileContext);

    ShipActor *getPlayer();

    private:
    ShipActor *player;
    std::vector<std::unique_ptr<BaseActor>> actors;
    sf::Clock clock = sf::Clock();
    sf::Time deltaTime;
    Window *window;
    std::optional<sf::View> playerView;

    void startFrame();
    void updateActors();
    void updatePlayerCamera();
    void drawActors();
    template <typename ActorType, typename ActorStateType>
    void addActor(ActorStateType *initialActorState);
};
