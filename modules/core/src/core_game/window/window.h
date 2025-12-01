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
    Window(WindowConfig *config);
    WindowConfig *config;
    sf::RenderWindow window = sf::RenderWindow();
    void init();
    void clear(sf::Color);
    void present();
};
