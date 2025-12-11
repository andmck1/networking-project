#pragma once

#include "base.h"

#include "../networking/messages.h"
#include "core_game/texturemanager/texturemanager.h"

enum ShipTurnDirection : int32_t
{
    LEFT = 0,
    RIGHT = 1,
    NONE = 2
};

struct ShipActorBuilderContext : public BaseActorBuilderContext
{
    ShipActorBuilderContext(
        TextureManager::ShipTextures *shipTextures,
        sf::Vector2f initialPosition, sf::Vector2f initialForwardVector,
        int32_t initialTurnDirection, int32_t serverId
    );
    ~ShipActorBuilderContext() = default;

    TextureManager::ShipTextures *shipTextures;
    int32_t initialTurnDirection;
    float initialTurningCircleRadius;
    int32_t serverId;
};

class ShipActor : public BaseActor
{
    public:
    ShipActor() = default;
    ~ShipActor() = default;
    void init(BaseActorBuilderContext *context) override;
    void update(float deltaTime) override;

    void networkUpdate(ShipMessage *updateMessage);

    int32_t serverId;
    int32_t currentTurnDirection;
    int32_t health = 100.f;
    // TODO - should have a player class that this lives in
    bool isPlayerControlling = false;
    bool isSpawning = true;
    float spawnTimer = 0.f;
    TextureManager::ShipTextures *shipTextures;

    private:
    float turningCircleRadius;
};
