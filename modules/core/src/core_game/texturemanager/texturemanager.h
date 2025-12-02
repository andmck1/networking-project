#pragma once

#include <optional>
#include <string>
#include <SFML/Graphics.hpp>

struct TextureManagerConfig
{
    TextureManagerConfig(
        std::string backgroundTexturePath = "assets/Background.png",
        std::string defaultShipTexturePath = "assets/DefaultShip.png",
        std::string projectileTexturePath = "assets/DefaultProjectile.png"
    );
    std::string backgroundTexturePath;
    std::string defaultShipTexturePath;
    std::string projectileTexturePath;
};

class TextureManager
{
    public:
    TextureManager(TextureManagerConfig *config);

    sf::Texture backgroundTexture;
    sf::Texture defaultShipTexture;
    sf::Texture projectileTexture;

    bool init();

    private:
    TextureManagerConfig *config;

    static std::optional<sf::Texture> loadTexture(std::string texturePath);
};
