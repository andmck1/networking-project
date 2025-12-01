#pragma once

#include <string>
#include <SFML/Graphics.hpp>

struct TextureManagerConfig
{
    TextureManagerConfig(
        std::string defaultShipTexturePath = "assets/DefaultShip.png"
    );
    std::string defaultShipTexturePath;
};

class TextureManager
{
    public:
    TextureManager(TextureManagerConfig *config);

    sf::Texture defaultShipTexture;

    bool init();

    private:
    TextureManagerConfig *config;
};
