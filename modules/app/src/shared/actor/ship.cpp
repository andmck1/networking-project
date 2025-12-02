#include "ship.h"

#include "core_game/util/constants.h"

#include <cmath>
#include <spdlog/spdlog.h>

ShipActorBuilderContext::ShipActorBuilderContext(
    sf::Texture *initialTexture, sf::Vector2f initialScaling,
    sf::Vector2f initialPosition, sf::Vector2f initialForwardVector,
    float initialSpeed, float initialRotation,
    ShipTurnDirection initialTurnDirection, float initialTurningCircleRadius
)
{
    this->initialTexture = initialTexture;
    this->initialScaling = initialScaling;
    this->initialPosition = initialPosition;
    this->initialForwardVector = initialForwardVector;
    this->initialSpeed = initialSpeed;
    this->initialRotation = initialRotation;
    this->initialTurnDirection = initialTurnDirection;
    this->initialTurningCircleRadius = initialTurningCircleRadius;
}

void ShipActor::init(BaseActorBuilderContext *context)
{
    ShipActorBuilderContext *_context =
        static_cast<ShipActorBuilderContext *>(context);

    this->position = _context->initialPosition;
    this->forwardVector = _context->initialForwardVector;
    this->rotation = _context->initialRotation;
    this->speed = _context->initialSpeed;
    this->spriteScaling = _context->initialScaling;
    this->currentTurnDirection = _context->initialTurnDirection;
    this->createSprite(*_context->initialTexture);
}

void ShipActor::update(float deltaTime)
{
    ShipTurnDirection currentTurnDirection = this->currentTurnDirection;
    float deltaTheta = 0;
    switch (currentTurnDirection)
    {
    case ShipTurnDirection::LEFT:
        deltaTheta = -this->speed * deltaTime / 10.f;
        break;
    case ShipTurnDirection::RIGHT:
        deltaTheta = this->speed * deltaTime / 10.f;
        break;
    default:
        break;
    }
    deltaTheta /= Constants::PI2;

    float x = this->forwardVector.x;
    float y = this->forwardVector.y;
    float cosDTheta = std::cos(deltaTheta);
    float sinDTheta = std::sin(deltaTheta);
    this->forwardVector = {
        x * cosDTheta - y * sinDTheta, x * sinDTheta + y * cosDTheta
    };
    this->forwardVector /= this->forwardVector.length();
    this->rotation += deltaTheta;

    this->moveBy(this->forwardVector * this->speed * deltaTime);
}

void ShipActor::setTurnDirection(ShipTurnDirection newTurnDirection)
{
    this->currentTurnDirection = newTurnDirection;
}
