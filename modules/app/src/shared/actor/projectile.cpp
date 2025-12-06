#include "projectile.h"

#include "../config/settings.h"

#include <spdlog/spdlog.h>

ProjectileActorBuilderContext::ProjectileActorBuilderContext(
    sf::Texture *initialTexture, sf::Vector2f initialPosition,
    sf::Vector2f initialForwardVector, int32_t shooterServerId, float lifetime
)
{
    this->initialTexture = initialTexture;
    this->initialScaling = Settings::Actor::projectileScaling;
    this->initialPosition = initialPosition;
    this->initialForwardVector = initialForwardVector;
    this->initialSpeed = Settings::Actor::projectileSpeed;
    this->initialRotation = initialForwardVector.angle().asRadians();
    this->shooterServerId = shooterServerId;
    this->lifetime = lifetime;
}

void ProjectileActor::init(BaseActorBuilderContext *context)
{
    ProjectileActorBuilderContext *_context =
        static_cast<ProjectileActorBuilderContext *>(context);

    this->position = _context->initialPosition;
    this->forwardVector = _context->initialForwardVector;
    this->rotation = _context->initialRotation;
    this->speed = _context->initialSpeed;
    this->spriteScaling = _context->initialScaling;
    this->createSprite(*_context->initialTexture);
    this->shooterServerId = _context->shooterServerId;
    this->lifetime = _context->lifetime;
}

void ProjectileActor::update(float deltaTime)
{
    sf::Vector2f positionDelta = this->forwardVector * this->speed * deltaTime;
    this->moveBy(positionDelta);
    this->lifetime += deltaTime;
    if (this->lifetime > Settings::Actor::projectileLifetime)
    {
        this->isDeleted = true;
    }
}
