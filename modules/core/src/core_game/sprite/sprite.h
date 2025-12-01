#pragma once

#include <SFML/Graphics.hpp>

struct SpriteConfig
{
    SpriteConfig(
        sf::Texture *texture, sf::Vector2f startingWorldPosition,
        sf::Vector2f spriteScaling
    );
    sf::Texture *texture;
    sf::Vector2f startingWorldPosition;
    sf::Vector2f spriteScaling;
};

class Sprite
{
    public:
    Sprite(SpriteConfig *config);
    std::unique_ptr<sf::Sprite> sprite;
    void init();

    private:
    SpriteConfig *config;
};
