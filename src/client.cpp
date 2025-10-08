#include <SFML/System/Vector2.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <array>
#include <vector>
#include <spdlog/spdlog.h>

std::string gPROJECT_NAME = "Networking Project";
std::string gACTOR_NAME = "CLIENT";
sf::Vector2<unsigned int> gDEFAULT_WINDOW_SIZE = {800, 600};
sf::Vector2<float> gDEFAULT_SPRITE_SCALING = {5.0f, 5.0f};
sf::Vector2<float> gDEFAULT_PLAYER_POSITION = {0.0f, 0.0f};
sf::Vector2<float> gDEFAULT_SPRITE_ORIGIN = {0.0f, 0.0f};

struct Player
{
    Player(sf::Texture texture, sf::Vector2u startingWorldPosition);
    sf::Vector2<float> forwardVector;
    sf::Vector2u position;
    sf::Sprite sprite;
};

Player::Player(sf::Texture texture, sf::Vector2u startingWorldPosition)
    : sprite(texture)
{
    spdlog::info("Creating new player");
    this->position = startingWorldPosition;
    this->forwardVector = sf::Vector2<float>(1.0, 0.0).rotatedBy(
        sf::Angle(sf::radians(std::rand() * 2 * 3.141))
    );
    this->sprite = sf::Sprite(texture);
}

int main()
{
    std::srand(time(0));
    spdlog::set_level(spdlog::level::debug);

    spdlog::info("Running " + gPROJECT_NAME);

    spdlog::debug("Creating window");
    sf::RenderWindow window;
    window.create(sf::VideoMode(gDEFAULT_WINDOW_SIZE), gACTOR_NAME);
    sf::Vector2u windowSize = window.getSize();
    sf::Vector2u windowOrigin = {windowSize.x / 2, windowSize.y / 2};

    sf::Texture texture;
    if (!texture.loadFromFile("assets/DefaultShip.png"))
    {
        spdlog::error("Failed to load texture");
        return -1;
    }
    // Player player = Player(texture, windowOrigin);
    sf::Sprite sprite = sf::Sprite(texture);
    sprite.setOrigin(sprite.getLocalBounds().getCenter());
    sprite.setPosition({windowSize.x / 2.0f, windowSize.y / 2.0f});
    sprite.scale(gDEFAULT_SPRITE_SCALING);

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }
        window.clear(sf::Color::Black);

        window.draw(sprite);

        window.display();
    }
    return 0;
}
