#include "ship.h"

#include "core_game/util/constants.h"
#include "../config/settings.h"

#include <cmath>
#include <spdlog/spdlog.h>

ShipActorBuilderContext::ShipActorBuilderContext(
    TextureManager::ShipTextures *shipTextures, sf::Vector2f initialPosition,
    sf::Vector2f initialForwardVector, int32_t initialTurnDirection,
    std::string handle, sf::Font *font
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
    this->font = font;
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
    this->font = _context->font;
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
    this->isEnabled = networkMessage->isConnected;
}

void ShipActor::drawInfo(Window *window)
{
    sf::Text text(*this->font);
    std::string displayText =
        std::format("{}\nK:{} D:{}", this->handle, this->kills, this->deaths);
    text.setString(displayText);
    text.setCharacterSize(Settings::ShipInfo::fontSize);
    text.setFillColor(Settings::ShipInfo::fontColour);
    sf::Vector2f textPosition = this->position;
    textPosition.x -= text.getLocalBounds().size.x / 2.f;
    textPosition.y -= Settings::ShipInfo::textYOffset;
    text.setPosition(textPosition);
    window->window.draw(text);

    int totalHealth = Settings::Game::shipHealth;
    int healthRemaining = this->health;
    int healthLost = totalHealth - healthRemaining;

    float remainingHealthBarWidth = (float) Settings::ShipInfo::healthBarWidth /
                                    totalHealth * healthRemaining;
    float lostHealthBarWidth =
        (float) Settings::ShipInfo::healthBarWidth / totalHealth * healthLost;

    sf::RectangleShape remainingHealthRectangle;
    sf::Vector2f remainingHealthRectanglePosition = this->position;
    remainingHealthRectanglePosition.x -=
        (float) Settings::ShipInfo::healthBarWidth / 2.f;
    remainingHealthRectanglePosition.y -= Settings::ShipInfo::healthBarYOffset;
    remainingHealthRectangle.setPosition(remainingHealthRectanglePosition);
    remainingHealthRectangle.setSize(
        {(float) remainingHealthBarWidth,
         (float) Settings::ShipInfo::healthBarHeight}
    );
    remainingHealthRectangle.setFillColor(sf::Color::Green);
    window->window.draw(remainingHealthRectangle);

    if (healthLost > 0)
    {
        sf::RectangleShape lostHealthRectangle;
        sf::Vector2f lostHealthRectanglePosition = this->position;
        lostHealthRectanglePosition.x -=
            (Settings::ShipInfo::healthBarWidth / 2.f) -
            remainingHealthBarWidth;
        lostHealthRectanglePosition.y -= Settings::ShipInfo::healthBarYOffset;
        lostHealthRectangle.setPosition(lostHealthRectanglePosition);
        lostHealthRectangle.setSize(
            {(float) lostHealthBarWidth,
             (float) Settings::ShipInfo::healthBarHeight}
        );
        lostHealthRectangle.setFillColor(sf::Color::Red);
        window->window.draw(lostHealthRectangle);
    }
}
