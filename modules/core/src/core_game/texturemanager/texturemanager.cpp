#include "texturemanager.h"

#include <spdlog/spdlog.h>

TextureManagerConfig::TextureManagerConfig(
    std::string backgroundTexturePath, std::string projectileTexturePath,

    std::string greyShipTexturePath, std::string greyShipLeftTexturePath,
    std::string greyShipRightTexturePath,

    std::string greenShipTexturePath, std::string greenShipLeftTexturePath,
    std::string greenShipRightTexturePath,

    std::string orangeShipTexturePath, std::string orangeShipLeftTexturePath,
    std::string orangeShipRightTexturePath,

    std::string purpleShipTexturePath, std::string purpleShipLeftTexturePath,
    std::string purpleShipRightTexturePath,

    std::string redShipTexturePath, std::string redShipLeftTexturePath,
    std::string redShipRightTexturePath
)
{
    this->backgroundTexturePath = backgroundTexturePath;
    this->projectileTexturePath = projectileTexturePath;

    this->greyShipTexturePath = greyShipTexturePath;
    this->greyShipLeftTexturePath = greyShipLeftTexturePath;
    this->greyShipRightTexturePath = greyShipRightTexturePath;

    this->greenShipTexturePath = greenShipTexturePath;
    this->greenShipLeftTexturePath = greenShipLeftTexturePath;
    this->greenShipRightTexturePath = greenShipRightTexturePath;

    this->orangeShipTexturePath = orangeShipTexturePath;
    this->orangeShipLeftTexturePath = orangeShipLeftTexturePath;
    this->orangeShipRightTexturePath = orangeShipRightTexturePath;

    this->purpleShipTexturePath = purpleShipTexturePath;
    this->purpleShipLeftTexturePath = purpleShipLeftTexturePath;
    this->purpleShipRightTexturePath = purpleShipRightTexturePath;

    this->redShipTexturePath = redShipTexturePath;
    this->redShipLeftTexturePath = redShipLeftTexturePath;
    this->redShipRightTexturePath = redShipRightTexturePath;
}

std::optional<sf::Texture> TextureManager::loadTexture(std::string texturePath)
{
    std::optional<sf::Texture> texture = sf::Texture();
    if (!texture.value().loadFromFile(texturePath))
    {
        spdlog::error("Failed to load texture");
        return std::nullopt;
    }
    return texture;
}

// TODO - yessss I know this could be automated
bool TextureManager::init(TextureManagerConfig *config)
{
    std::optional<sf::Texture> texture;

    spdlog::debug("Loading background texture");
    texture = this->loadTexture(config->backgroundTexturePath);
    if (!texture)
        return false;
    this->backgroundTexture = texture.value();

    spdlog::debug("Loading projectile texture");
    texture = this->loadTexture(config->projectileTexturePath);
    if (!texture)
        return false;
    this->projectileTexture = texture.value();

    spdlog::debug("Loading grey ship texture");
    ShipTextures *greyShipTextures = &this->shipTextures[0];
    texture = this->loadTexture(config->greyShipTexturePath);
    if (!texture)
        return false;
    greyShipTextures->forward = texture.value();
    texture = this->loadTexture(config->greyShipLeftTexturePath);
    if (!texture)
        return false;
    greyShipTextures->left = texture.value();
    texture = this->loadTexture(config->greyShipRightTexturePath);
    if (!texture)
        return false;
    greyShipTextures->right = texture.value();

    spdlog::debug("Loading green ship texture");
    ShipTextures *greenShipTextures = &this->shipTextures[1];
    texture = this->loadTexture(config->greenShipTexturePath);
    if (!texture)
        return false;
    greenShipTextures->forward = texture.value();
    texture = this->loadTexture(config->greenShipLeftTexturePath);
    if (!texture)
        return false;
    greenShipTextures->left = texture.value();
    texture = this->loadTexture(config->greenShipRightTexturePath);
    if (!texture)
        return false;
    greenShipTextures->right = texture.value();

    spdlog::debug("Loading orange ship texture");
    ShipTextures *orangeShipTextures = &this->shipTextures[2];
    texture = this->loadTexture(config->orangeShipTexturePath);
    if (!texture)
        return false;
    orangeShipTextures->forward = texture.value();
    texture = this->loadTexture(config->orangeShipLeftTexturePath);
    if (!texture)
        return false;
    orangeShipTextures->left = texture.value();
    texture = this->loadTexture(config->orangeShipRightTexturePath);
    if (!texture)
        return false;
    orangeShipTextures->right = texture.value();

    spdlog::debug("Loading purple ship texture");
    ShipTextures *purpleShipTextures = &this->shipTextures[3];
    texture = this->loadTexture(config->purpleShipTexturePath);
    if (!texture)
        return false;
    purpleShipTextures->forward = texture.value();
    texture = this->loadTexture(config->purpleShipLeftTexturePath);
    if (!texture)
        return false;
    purpleShipTextures->left = texture.value();
    texture = this->loadTexture(config->purpleShipRightTexturePath);
    if (!texture)
        return false;
    purpleShipTextures->right = texture.value();

    spdlog::debug("Loading red ship texture");
    ShipTextures *redShipTextures = &this->shipTextures[4];
    texture = this->loadTexture(config->redShipTexturePath);
    if (!texture)
        return false;
    redShipTextures->forward = texture.value();
    texture = this->loadTexture(config->redShipLeftTexturePath);
    if (!texture)
        return false;
    redShipTextures->left = texture.value();
    texture = this->loadTexture(config->redShipRightTexturePath);
    if (!texture)
        return false;
    redShipTextures->right = texture.value();

    // Some copilot help for this one
    this->randomNumberGenerator = std::mt19937(this->randomDevice());
    this->randomNumberDistribution =
        std::uniform_int_distribution<int>(0, this->shipTextures.size() - 1);

    return true;
};

int TextureManager::getRandomTextureIndex()
{
    return this->randomNumberDistribution(this->randomNumberGenerator);
}
