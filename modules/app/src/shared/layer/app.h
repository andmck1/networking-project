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

    // TODO - this is a queue not a vector?
    std::vector<std::unique_ptr<ShootMessage>> shootEvents = {};

    void initPlayerCamera();

    void addPlayer(ShipActorBuilderContext *initialPlayerState);
    void addServerShip(
        int32_t serverId, sf::Vector2f initialPosition,
        sf::Vector2f intialForwardVector
    );
    void addProjectile(ProjectileActorBuilderContext *initialProjectileContext);
    ProjectileActorBuilderContext createProjecileBuildContext(ShipActor *ship);
    void addShootEvent(sf::Vector2f position, sf::Vector2f forwardVector);

    void cleanUpActors();

    float getGameTime();
    float getDeltaTime();
    ShipActor *getPlayer();

    void
    networkUpdate(const std::vector<std::unique_ptr<Message>> &networkUpdates);

    std::vector<std::unique_ptr<BaseActor>> actors;
    // TODO - shouldn't use raw refs here
    std::vector<ShipActor *> ships;
    std::vector<ProjectileActor *> projectiles;

    private:
    ShipActor *player;
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
