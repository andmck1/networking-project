#include "window.h"
#include <spdlog/spdlog.h>

WindowConfig::WindowConfig(
    sf::Vector2u windowSize, sf::Vector2u windowOrigin, std::string appName,
    sf::Texture *backgroundTexture, sf::Color clearColour
)
{
    this->windowSize = windowSize;
    this->windowOrigin = windowOrigin;
    this->appName = appName;
    this->backgroundTexture = backgroundTexture;
    this->clearColour = clearColour;
}

bool Window::init(WindowConfig *config)
{
    spdlog::debug("Initialising window");
    if (!config)
    {
        spdlog::error("Config not defined");
        return false;
    }

    this->windowSize = config->windowSize;
    this->windowOrigin = config->windowOrigin;
    this->appName = config->appName;
    this->clearColour = config->clearColour;

    this->window.create(sf::VideoMode(this->windowSize), this->appName);
    this->backgroundSprite =
        std::make_unique<sf::Sprite>(*config->backgroundTexture);

    // Some copilot help for this one
    this->randomNumberGenerator = std::mt19937(this->randomDevice());
    this->randomNumberDistributionWindowWidth =
        std::uniform_int_distribution<unsigned int>(
            this->windowSize.x / 10, this->windowSize.x * 9 / 10
        );
    this->randomNumberDistributionWindowLength =
        std::uniform_int_distribution<unsigned int>(
            this->windowSize.y / 10, this->windowSize.y * 9 / 10
        );

    return true;
}

void Window::clear()
{
    this->window.clear(clearColour);
}

void Window::drawBackground()
{
    this->window.draw(*this->backgroundSprite);
}

void Window::present()
{
    this->window.display();
}

sf::Vector2u Window::getWindowSize()
{
    return this->windowSize;
}

sf::Vector2u Window::getWindowCentre()
{
    return {this->windowSize.x / 2, this->windowSize.y / 2};
}

sf::Vector2u Window::getRandomWindowPosition()
{
    return {
        this->randomNumberDistributionWindowWidth(this->randomNumberGenerator),
        this->randomNumberDistributionWindowLength(this->randomNumberGenerator)
    };
}
