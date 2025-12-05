#pragma once

#include <random>
#include <array>
#include <optional>
#include <string>
#include <SFML/Graphics.hpp>

struct TextureManagerConfig
{
    TextureManagerConfig(
        std::string backgroundTexturePath = "assets/Background.png",
        std::string projectileTexturePath = "assets/DefaultProjectile.png",

        std::string greyShipTexturePath = "assets/GreyShip.png",
        std::string greyShipLeftTexturePath = "assets/GreyShipLeft.png",
        std::string greyShipRightTexturePath = "assets/GreyShipRight.png",

        std::string greenShipTexturePath = "assets/GreenShip.png",
        std::string greenShipLeftTexturePath = "assets/GreenShipLeft.png",
        std::string greenShipRightTexturePath = "assets/GreenShipRight.png",

        std::string orangeShipTexturePath = "assets/OrangeShip.png",
        std::string orangeShipLeftTexturePath = "assets/OrangeShipLeft.png",
        std::string orangeShipRightTexturePath = "assets/OrangeShipRight.png",

        std::string purpleShipTexturePath = "assets/PurpleShip.png",
        std::string purpleShipLeftTexturePath = "assets/PurpleShipLeft.png",
        std::string purpleShipRightTexturePath = "assets/PurpleShipRight.png",

        std::string redShipTexturePath = "assets/RedShip.png",
        std::string redShipLeftTexturePath = "assets/RedShipLeft.png",
        std::string redShipRightTexturePath = "assets/RedShipRight.png"
    );
    std::string backgroundTexturePath;
    std::string greyShipTexturePath;
    std::string greyShipLeftTexturePath;
    std::string greyShipRightTexturePath;
    std::string greenShipTexturePath;
    std::string greenShipLeftTexturePath;
    std::string greenShipRightTexturePath;
    std::string orangeShipTexturePath;
    std::string orangeShipLeftTexturePath;
    std::string orangeShipRightTexturePath;
    std::string purpleShipTexturePath;
    std::string purpleShipLeftTexturePath;
    std::string purpleShipRightTexturePath;
    std::string redShipTexturePath;
    std::string redShipLeftTexturePath;
    std::string redShipRightTexturePath;
    std::string projectileTexturePath;
};

class TextureManager
{

    public:
    struct ShipTextures
    {
        sf::Texture forward;
        sf::Texture left;
        sf::Texture right;
    };

    TextureManager() = default;
    ~TextureManager() = default;

    sf::Texture backgroundTexture;
    sf::Texture projectileTexture;
    std::array<ShipTextures, 5> shipTextures;


    bool init(TextureManagerConfig *config);
    int getRandomTextureIndex();

    private:
    std::random_device randomDevice;
    std::mt19937 randomNumberGenerator;
    std::uniform_int_distribution<int> randomNumberDistribution;
    static std::optional<sf::Texture> loadTexture(std::string texturePath);
};
