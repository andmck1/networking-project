#pragma once

#include <SFML/Graphics.hpp>

struct WindowConfig
{
    WindowConfig(
        sf::Vector2u windowSize, sf::Vector2u windowOrigin, std::string appName,
        sf::Texture *backgroundTexture, sf::Color clearColour
    );

    sf::Vector2u windowSize;
    sf::Vector2u windowOrigin;
    std::string appName;
    sf::Texture *backgroundTexture;
    sf::Color clearColour;
};

class Window
{
    public:
    Window() = default;
    ~Window() = default;

    sf::RenderWindow window;
    bool init(WindowConfig *config);
    void clear();
    void drawBackground();
    void present();

    sf::Vector2u getWindowSize();
    sf::Vector2u getWindowCentre();

    private:
    sf::Vector2u windowSize;
    sf::Vector2u windowOrigin;
    std::string appName;
    std::unique_ptr<sf::Sprite> backgroundSprite;
    sf::Color clearColour;
};
