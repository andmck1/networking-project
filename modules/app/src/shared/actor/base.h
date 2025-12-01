#pragma once

#include "core_game/sprite/sprite.h"
#include "core_game/window/window.h"

struct BaseActorState
{
    virtual ~BaseActorState() = default;

    std::unique_ptr<Sprite> sprite;
    sf::Vector2f position;
    sf::Vector2f forwardVector;
    float speed;
    float rotation;
};

class BaseActor
{
    public:
    virtual ~BaseActor() = default;

    void moveBy(sf::Vector2f positionDelta);
    void moveTo(sf::Vector2f newPosition);
    void draw(Window *window);

    virtual void update(float deltaTime);

    protected:
    BaseActorState state;
};
