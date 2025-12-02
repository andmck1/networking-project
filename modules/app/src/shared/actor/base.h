#pragma once

#include "core_game/window/window.h"

struct BaseActorBuilderContext
{
    BaseActorBuilderContext() = default;
    virtual ~BaseActorBuilderContext() = default;

    sf::Texture *initialTexture;
    sf::Vector2f initialScaling;
    sf::Vector2f initialPosition;
    sf::Vector2f initialForwardVector;
    float initialSpeed;
    float initialRotation;
};

class BaseActor
{
    public:
    BaseActor() = default;
    virtual ~BaseActor() = default;

    sf::Vector2f position;
    sf::Vector2f forwardVector;
    float rotation;
    float speed;
    bool isDeleted = false;

    void moveBy(sf::Vector2f positionDelta);
    void moveTo(sf::Vector2f newPosition);
    void draw(Window *window);
    void createSprite(const sf::Texture &texture);

    virtual void update(float deltaTime);
    virtual void init(BaseActorBuilderContext *context) = 0;

    protected:
    std::unique_ptr<sf::Sprite> sprite;
    sf::Vector2f spriteScaling;
};
