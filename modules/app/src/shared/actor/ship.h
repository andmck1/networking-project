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
        sf::Texture *initialTexture, sf::Vector2f initialScaling,
        sf::Vector2f initialPosition, sf::Vector2f initialForwardVector,
        float initialSpeed, float initialRotation,
        ShipTurnDirection initialTurnDirection, float initialTurningCircleRadius
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

    void setTurnDirection(ShipTurnDirection newTurnDirection);

    float turningCircleRadius;

    private:
    ShipTurnDirection currentTurnDirection;
};
