#include "window.h"
#include <spdlog/spdlog.h>

WindowConfig::WindowConfig(
    sf::Vector2u windowSize, sf::Vector2u windowOrigin, std::string appName
)
{
    this->windowSize = windowSize;
    this->windowOrigin = windowOrigin;
    this->appName = appName;
}

bool Window::init(std::unique_ptr<WindowConfig> config)
{
    spdlog::debug("Initialising window");
    if (!config)
    {
        spdlog::error("Config not defined");
        return false;
    }

    this->config = std::move(config);
    this->window.create(
        sf::VideoMode(this->config->windowSize), this->config->appName
    );
    this->config->windowSize = this->window.getSize();
    this->config->windowOrigin = {
        this->config->windowSize.x / 2, this->config->windowSize.y / 2
    };

    return true;
}

void Window::clear(sf::Color clearColour)
{
    this->window.clear(clearColour);
}

void Window::present()
{
    this->window.display();
}

sf::Vector2u Window::getWindowSize()
{
    return this->config->windowSize;
}

sf::Vector2u Window::getWindowCentre()
{
    return {this->config->windowSize.x / 2, this->config->windowSize.y / 2};
}
