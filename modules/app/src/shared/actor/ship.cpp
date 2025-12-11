#include "ship.h"

#include "core_game/util/constants.h"
#include "../config/settings.h"

#include <cmath>
#include <spdlog/spdlog.h>

ShipActorBuilderContext::ShipActorBuilderContext(
    TextureManager::ShipTextures *shipTextures, sf::Vector2f initialPosition,
    sf::Vector2f initialForwardVector, int32_t initialTurnDirection,
    std::string handle
)
{
    this->initialTexture = &shipTextures->forward;
    this->initialScaling = Settings::Actor::shipScaling;
    this->initialPosition = initialPosition;
    this->initialForwardVector = initialForwardVector;
    this->initialSpeed = Settings::Game::shipSpeed;
    this->initialRotation = initialForwardVector.angle().asRadians();
    this->initialTurnDirection = initialTurnDirection;
    this->initialTurningCircleRadius = Settings::Actor::shipTurningRadius;
    this->handle = handle;
    this->shipTextures = shipTextures;
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
    this->handle = _context->handle;
    this->shipTextures = _context->shipTextures;
    this->createSprite(*_context->initialTexture);
}

void ShipActor::update(float deltaTime)
{
    int32_t currentTurnDirection = this->currentTurnDirection;
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

void ShipActor::networkUpdate(ShipMessage *networkMessage)
{
    this->position = networkMessage->position;
    this->forwardVector = networkMessage->forwardVector;
    this->rotation = std::atan2(this->forwardVector.y, this->forwardVector.x);
    this->rotation =
        this->rotation < 0 ? this->rotation + Constants::PI2 : this->rotation;
    this->currentTurnDirection = networkMessage->turnDirection;
}
