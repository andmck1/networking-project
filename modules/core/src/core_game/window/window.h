#pragma once

#include <SFML/Graphics.hpp>

struct WindowConfig
{
    WindowConfig(
        sf::Vector2u windowSize, sf::Vector2u windowOrigin, std::string appName
    );

    sf::Vector2u windowSize;
    sf::Vector2u windowOrigin;
    std::string appName;
};

class Window
{
    public:
    Window() = default;
    ~Window() = default;

    sf::RenderWindow window;
    bool init(std::unique_ptr<WindowConfig> config);
    void clear(sf::Color);
    void present();

    sf::Vector2u getWindowSize();
    sf::Vector2u getWindowCentre();

    private:
    std::unique_ptr<WindowConfig> config;
};
