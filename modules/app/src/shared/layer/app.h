#pragma once

#include "base.h"
#include "core_game/texturemanager/texturemanager.h"
#include "core_game/window/window.h"
#include "../actor/base.h"
#include "../actor/ship.h"
#include "../actor/projectile.h"
#include "../networking/messages.h"

#include <SFML/Graphics.hpp>

#include <unordered_map>
#include <vector>
#include <optional>

struct AppLayerConfig : BaseLayerConfig
{
    AppLayerConfig(Window *window, TextureManager *textureManager);
    Window *window;
    TextureManager *textureManager;
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
    void addServerShip(
        int32_t serverId, sf::Vector2f initialPosition,
        sf::Vector2f intialForwardVector
    );
    void addProjectile(ProjectileActorBuilderContext *initialProjectileContext);
    float getGameTime();
    float getDeltaTime();
    void networkUpdate(std::vector<std::unique_ptr<Message>> networkUpdates);

    ShipActor *getPlayer();

    private:
    ShipActor *player;
    std::vector<std::unique_ptr<BaseActor>> actors;
    std::unordered_map<int32_t, ShipActor *> shipActorMap;
    sf::Clock clock = sf::Clock();
    sf::Time deltaTime;
    sf::Time totalTime;
    Window *window;
    std::optional<sf::View> playerView;
    TextureManager *textureManager;

    void startFrame();
    void updateActors();
    void updatePlayerCamera();
    void drawActors();
    template <typename ActorType, typename ActorStateType>
    void addActor(ActorStateType *initialActorState);
};
