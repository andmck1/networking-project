#include "texturemanager.h"

#include <spdlog/spdlog.h>

TextureManagerConfig::TextureManagerConfig(std::string defaultShipTexturePath)
{
    this->defaultShipTexturePath = defaultShipTexturePath;
}

TextureManager::TextureManager(TextureManagerConfig *config)
{
    this->config = config;
}

bool TextureManager::init()
{
    spdlog::debug("Initialising texture manager");

    if (!this->defaultShipTexture.loadFromFile(
            this->config->defaultShipTexturePath
        ))
    {
        spdlog::error("Failed to load texture");
        return false;
    }
    return true;
};
