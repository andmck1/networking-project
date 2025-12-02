#include "ship.h"

#include "core_game/util/constants.h"
#include "../config/settings.h"

#include <cmath>
#include <spdlog/spdlog.h>

ShipActorBuilderContext::ShipActorBuilderContext(
    sf::Texture *initialTexture, sf::Vector2f initialPosition,
    sf::Vector2f initialForwardVector, ShipTurnDirection initialTurnDirection
)
{
    this->initialTexture = initialTexture;
    this->initialScaling = Settings::Actor::shipScaling;
    this->initialPosition = initialPosition;
    this->initialForwardVector = initialForwardVector;
    this->initialSpeed = Settings::Actor::shipSpeed;
    this->initialRotation = initialForwardVector.angle().asRadians();
    this->initialTurnDirection = initialTurnDirection;
    this->initialTurningCircleRadius = Settings::Actor::shipTurningRadius;
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
    this->turningCircleRadius = _context->initialTurningCircleRadius;
    this->createSprite(*_context->initialTexture);
}

void ShipActor::update(float deltaTime)
{
    ShipTurnDirection currentTurnDirection = this->currentTurnDirection;
    float deltaTheta = 0;
    switch (currentTurnDirection)
    {
        case ShipTurnDirection::LEFT:
            deltaTheta = -this->speed * deltaTime / this->turningCircleRadius;
            break;
        case ShipTurnDirection::RIGHT:
            deltaTheta = this->speed * deltaTime / this->turningCircleRadius;
            break;
        default:
            break;
    }
    deltaTheta /= Constants::PI2;
    this->rotation += deltaTheta;
    if (this->rotation > Constants::PI2)
        this->rotation -= Constants::PI2;
    else if (this->rotation < 0)
        this->rotation += Constants::PI2;

    float cosTheta = std::cos(this->rotation);
    float sinTheta = std::sin(this->rotation);
    this->forwardVector = {cosTheta, sinTheta};

    this->moveBy(this->forwardVector * this->speed * deltaTime);
}
