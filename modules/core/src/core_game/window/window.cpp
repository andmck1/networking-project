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

Window::Window(WindowConfig *config)
{
    this->config = config;
}

void Window::init()
{
    spdlog::debug("Initialising window");
    this->window.create(
        sf::VideoMode(this->config->windowSize), this->config->appName
    );
    this->config->windowSize = this->window.getSize();
    this->config->windowOrigin = {
        this->config->windowSize.x / 2, this->config->windowSize.y / 2
    };
}

void Window::clear(sf::Color clearColour)
{
    this->window.clear(clearColour);
}

void Window::present()
{
    this->window.display();
}
