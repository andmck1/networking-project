#pragma once

#include "base.h"

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

    ShipTurnDirection currentTurnDirection;

    private:
    float turningCircleRadius;
};
