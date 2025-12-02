#include "texturemanager.h"

#include <spdlog/spdlog.h>

TextureManagerConfig::TextureManagerConfig(
    std::string backgroundTexturePath, std::string defaultShipTexturePath,
    std::string projectileTexturePath
)
{
    this->backgroundTexturePath = backgroundTexturePath;
    this->defaultShipTexturePath = defaultShipTexturePath;
    this->projectileTexturePath = projectileTexturePath;
}

TextureManager::TextureManager(TextureManagerConfig *config)
{
    this->config = config;
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

bool TextureManager::init()
{
    std::optional<sf::Texture> texture;

    spdlog::debug("Loading background texture");
    texture = this->loadTexture(this->config->backgroundTexturePath);
    if (!texture)
        return false;
    this->backgroundTexture = texture.value();

    spdlog::debug("Loading ship texture");
    texture = this->loadTexture(this->config->defaultShipTexturePath);
    if (!texture)
        return false;
    this->defaultShipTexture = texture.value();

    spdlog::debug("Loading projectile texture");
    texture = this->loadTexture(this->config->projectileTexturePath);
    if (!texture)
        return false;
    this->projectileTexture = texture.value();

    return true;
};
