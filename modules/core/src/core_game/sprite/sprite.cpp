#include "sprite.h"

#include <spdlog/spdlog.h>

SpriteConfig::SpriteConfig(
    sf::Texture *texture, sf::Vector2f startingWorldPosition,
    sf::Vector2f spriteScaling
)
{
    this->texture = texture;
    this->startingWorldPosition = startingWorldPosition;
    this->spriteScaling = spriteScaling;
};

Sprite::Sprite(SpriteConfig *config)
{
    this->config = config;
};

void Sprite::init()
{
    spdlog::debug("Intialising sprite");
    this->sprite =
        std::make_unique<sf::Sprite>(sf::Sprite(*this->config->texture));
    this->sprite->setOrigin(this->sprite->getLocalBounds().getCenter());
    this->sprite->setPosition(this->config->startingWorldPosition);
    this->sprite->scale(this->config->spriteScaling);
}
