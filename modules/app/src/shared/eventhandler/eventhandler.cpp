#include "eventhandler.h"

#include "imgui.h"

#include "../config/settings.h"

EventHandlerConfig::EventHandlerConfig(
    Window *window, TextureManager *textureManager, AppLayer *appLayer,
    bool isServer
)
{
    this->window = window;
    this->textureManager = textureManager;
    this->appLayer = appLayer;
    this->isServer = isServer;
}

void EventHandler::init(EventHandlerConfig *config)
{
    this->window = config->window;
    this->textureManager = config->textureManager;
    this->appLayer = config->appLayer;
    this->isServer = config->isServer;
}

void EventHandler::addPlayerHandle(ShipActor *player)
{
    this->player = player;
}

bool EventHandler::handle()
{
    this->fireTimer += this->appLayer->getDeltaTime();
    std::optional<sf::Event> event = this->window->window.pollEvent();
    while (event)
    {
        if (event->is<sf::Event::Closed>())
        {
            if (event->is<sf::Event::Closed>())
            {
                this->window->window.close();
                return false;
            }
        }
        else
        {
            if (!this->handleInput())
                return false;
        }
        this->handleDebugInput(event);

        event = this->window->window.pollEvent();
    }
    return true;
}

bool EventHandler::handleInput()
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Escape))
    {
        this->window->window.close();
        return false;
    }
    if (this->player)
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A))
        {
            this->player.value()->currentTurnDirection =
                ShipTurnDirection::LEFT;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D))
        {
            this->player.value()->currentTurnDirection =
                ShipTurnDirection::RIGHT;
        }
        else
        {
            this->player.value()->currentTurnDirection =
                ShipTurnDirection::NONE;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Space))
        {
            if (this->fireTimer > Settings::Actor::shipFirerate)
            {
                ProjectileActorBuilderContext projectileBuilderContext =
                    this->appLayer->createProjecileBuildContext(
                        this->player.value()
                    );
                this->appLayer->addProjectile(&projectileBuilderContext);
                this->appLayer->addShootEvent(
                    projectileBuilderContext.initialPosition,
                    projectileBuilderContext.initialForwardVector
                );
                this->fireTimer = 0.f;
            }
        }
    }

    return true;
}

void EventHandler::handleDebugInput(std::optional<sf::Event> event)
{
    ImGuiIO &io = ImGui::GetIO();
    if (const sf::Event::MouseButtonPressed *mbPressed =
            event->getIf<sf::Event::MouseButtonPressed>())
    {
        switch (mbPressed->button)
        {
            case (sf::Mouse::Button::Left):
                io.MouseDown[0] = true;
                break;
            case (sf::Mouse::Button::Right):
                io.MouseDown[1] = true;
                break;
            default:
                break;
        }
    }
    if (const sf::Event::MouseButtonReleased *mbPressed =
            event->getIf<sf::Event::MouseButtonReleased>())
    {
        switch (mbPressed->button)
        {
            case (sf::Mouse::Button::Left):
                io.MouseDown[0] = false;
                break;
            case (sf::Mouse::Button::Right):
                io.MouseDown[1] = false;
                break;
            default:
                break;
        }
    }

    sf::Vector2i mousePosition = sf::Mouse::getPosition();
    io.MousePos = ImVec2(
        static_cast<float>(mousePosition.x), static_cast<float>(mousePosition.y)
    );
}
