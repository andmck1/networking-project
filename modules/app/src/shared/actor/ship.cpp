#include "ship.h"
#include <cmath>

ShipActorState::ShipActorState(
    std::unique_ptr<Sprite> sprite, sf::Vector2f position,
    sf::Vector2f forwardVector, float speed, float rotation,
    float turningCircleRadius, ShipTurnDirection turnDirection
)
{
    this->sprite = std::move(sprite);
    this->position = position;
    this->forwardVector = forwardVector;
    this->speed = speed;
    this->rotation = rotation;
    this->turningCircleRadius = turningCircleRadius;
    this->currentTurnDirection = turnDirection;
}

ShipActor::ShipActor(ShipActorState initialState)
    : state(
          std::move(initialState.sprite), initialState.position,
          initialState.forwardVector, initialState.speed, initialState.rotation,
          initialState.turningCircleRadius, initialState.currentTurnDirection
      )
{
}

void ShipActor::setTurnDirection(ShipTurnDirection newTurnDirection)
{
    this->state.currentTurnDirection = newTurnDirection;
}

void ShipActor::update(float deltaTime)
{
    ShipTurnDirection currentTurnDirection = this->state.currentTurnDirection;
    float deltaTheta = 0;
    switch (currentTurnDirection)
    {
    case ShipTurnDirection::LEFT:
        deltaTheta = this->state.speed * deltaTime;
    case ShipTurnDirection::RIGHT:
        deltaTheta = -this->state.speed * deltaTime;
    default:
        return;
    }

    float x = this->state.forwardVector.x;
    float y = this->state.forwardVector.y;
    float cosDTheta = std::cos(deltaTheta);
    float sinDTheta = std::sin(deltaTheta);
    this->state.forwardVector = {
        x * cosDTheta - y * sinDTheta, x * sinDTheta + y * cosDTheta
    };
    this->state.rotation += deltaTheta;

    this->moveBy(this->state.forwardVector * this->state.speed * deltaTime);
}
