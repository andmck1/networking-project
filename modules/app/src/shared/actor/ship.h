#pragma once

#include "base.h"

#include "../networking/messages.h"

enum class ShipTurnDirection
{
    LEFT,
    RIGHT,
    NONE
};

struct ShipActorBuilderContext : public BaseActorBuilderContext
{
    ShipActorBuilderContext(
        sf::Texture *initialTexture, sf::Vector2f initialPosition,
        sf::Vector2f initialForwardVector,
        ShipTurnDirection initialTurnDirection
    );
    ~ShipActorBuilderContext() = default;

    ShipTurnDirection initialTurnDirection;
    float initialTurningCircleRadius;
};

class ShipActor : public BaseActor
{
    public:
    ShipActor() = default;
    ~ShipActor() = default;
    void init(BaseActorBuilderContext *context) override;
    void update(float deltaTime) override;

    void setServerId(int32_t serverId);
    void networkUpdate(ShipMessage *updateMessage);

    int32_t serverId;
    ShipTurnDirection currentTurnDirection;

    private:
    float turningCircleRadius;
};
