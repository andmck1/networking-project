#pragma once

#include "base.h"

#include "../networking/messages.h"

enum ShipTurnDirection : int32_t
{
    LEFT = 0,
    RIGHT = 1,
    NONE = 2
};

struct ShipActorBuilderContext : public BaseActorBuilderContext
{
    ShipActorBuilderContext(
        sf::Texture *initialTexture, sf::Vector2f initialPosition,
        sf::Vector2f initialForwardVector,
        int32_t initialTurnDirection, int32_t serverId
    );
    ~ShipActorBuilderContext() = default;

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

    private:
    float turningCircleRadius;
};
