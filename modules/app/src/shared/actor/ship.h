#include "base.h"

enum class ShipTurnDirection
{
    LEFT,
    RIGHT,
    NONE
};

struct ShipActorState : public BaseActorState
{
    ShipActorState(
        std::unique_ptr<Sprite> sprite, sf::Vector2f position,
        sf::Vector2f forwardVector, float speed, float rotation,
        float turningCircleRadius, ShipTurnDirection turnDirection
    );

    float turningCircleRadius;
    ShipTurnDirection currentTurnDirection;
};

class ShipActor : public BaseActor
{
    public:
    ShipActor(ShipActorState initialState);
    void setTurnDirection(ShipTurnDirection newTurnDirection);
    void update(float deltaTime) override;

    private:
    ShipActorState state;
};
