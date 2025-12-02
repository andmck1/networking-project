#include "base.h"

#include <spdlog/spdlog.h>

void BaseActor::moveBy(sf::Vector2f positionDelta)
{
    this->position += positionDelta;
}

void BaseActor::moveTo(sf::Vector2f newPosition)
{
    this->position = newPosition;
}

void BaseActor::draw(Window *window)
{
    if (sprite)
    {
        this->sprite->setPosition(this->position);
        this->sprite->setRotation(sf::radians(this->rotation));
        window->window.draw(*this->sprite);
    }
    else
        spdlog::error("Sprite not defined");
}

void BaseActor::update(float deltaTime)
{
    this->moveBy(this->forwardVector * this->speed * deltaTime);
}

void BaseActor::createSprite(const sf::Texture &texture)
{
    this->sprite = std::make_unique<sf::Sprite>(sf::Sprite(texture));
    this->sprite->setOrigin(this->sprite->getLocalBounds().getCenter());
    this->sprite->setPosition(this->position);
    this->sprite->scale(this->spriteScaling);
}
