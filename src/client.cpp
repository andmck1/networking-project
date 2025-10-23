#include <SFML/System/Vector2.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <array>
#include <optional>
#include <vector>
#include <spdlog/spdlog.h>

std::string gPROJECT_NAME = "Networking Project";
std::string gACTOR_NAME = "CLIENT";
sf::Vector2u gDEFAULT_WINDOW_SIZE = {800, 600};
sf::Vector2f gDEFAULT_SPRITE_SCALING = {5.0f, 5.0f};
sf::Vector2f gDEFAULT_PLAYER_POSITION = {0.0f, 0.0f};
sf::Vector2f gDEFAULT_SPRITE_ORIGIN = {0.0f, 0.0f};
float PLAYER_SPEED = 50.f;

struct Player
{
    Player(
        const sf::Texture &texture, sf::Vector2f startingWorldPosition
    );
    sf::Vector2f forwardVector = {1.f, 0.f};
    sf::Vector2f position;
    sf::Sprite sprite;
};

Player::Player(
    const sf::Texture &texture, sf::Vector2f startingWorldPosition
)
    : sprite(texture)
{
    spdlog::info("Creating new player");
    this->position = startingWorldPosition;
    this->forwardVector = sf::Vector2f(1.0, 0.0).rotatedBy(
        sf::Angle(sf::radians(std::rand() * 2 * 3.141))
    );
    this->sprite = sf::Sprite(texture);
    this->sprite.setOrigin(sprite.getLocalBounds().getCenter());
    this->sprite.setPosition(startingWorldPosition);
    this->sprite.scale(gDEFAULT_SPRITE_SCALING);
}

struct WindowInfo
{
    sf::Vector2u windowSize;
    sf::Vector2u windowOrigin;
};

class App
{
  public:
    sf::RenderWindow window;
    WindowInfo windowInfo;

    sf::Clock clock;
    sf::Time deltaTime;

    App()
    {
        spdlog::debug("Initialising app");
        spdlog::debug("Creating window");
        this->window.create(sf::VideoMode(gDEFAULT_WINDOW_SIZE), gACTOR_NAME);
        this->windowInfo.windowSize = this->window.getSize();
        this->windowInfo.windowOrigin = {
            this->windowInfo.windowSize.x / 2, this->windowInfo.windowSize.y / 2
        };
    }

    void closeWindow()
    {
        this->window.close();
    }

    void frame()
    {
        this->deltaTime = clock.getElapsedTime();
        this->clock.restart();
    }
};

int main()
{
    std::srand(time(0));
    spdlog::set_level(spdlog::level::debug);

    spdlog::info("Running " + gPROJECT_NAME);
    App app = App();

    sf::Texture texture;
    if (!texture.loadFromFile("assets/DefaultShip.png"))
    {
        spdlog::error("Failed to load texture");
        return -1;
    }

    sf::Vector2f playerStartingWorldPosition = {
        app.windowInfo.windowSize.x / 2.f, app.windowInfo.windowSize.y / 2.f
    };
    Player player = Player(texture, playerStartingWorldPosition);

    while (app.window.isOpen())
    {
        app.frame();
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Escape))
        {
            app.closeWindow();
        }

        sf::Vector2f playerMoveAmount =
            - player.forwardVector * app.deltaTime.asSeconds() * PLAYER_SPEED;
        player.sprite.move(playerMoveAmount);

        app.window.clear(sf::Color::Black);
        app.window.draw(player.sprite);
        app.window.display();
    }
    return 0;
}
