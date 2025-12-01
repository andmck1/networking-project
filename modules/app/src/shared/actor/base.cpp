#include "base.h"

enum class MoveDirection
{
    LEFT,
    RIGHT,
    NONE
};

void BaseActor::moveBy(sf::Vector2f positionDelta)
{
    this->state.position += positionDelta;
}

void BaseActor::moveTo(sf::Vector2f newPosition)
{
    this->state.position = newPosition;
}

void BaseActor::draw(Window *window)
{
    window->window.draw(*this->state.sprite->sprite);
}

void BaseActor::update(float deltaTime)
{
    this->moveBy(this->state.forwardVector * this->state.speed * deltaTime);
}
