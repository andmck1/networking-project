#include "eventhandler.h"

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
            if (!this->handleInput(event))
                return false;
        }

        event = this->window->window.pollEvent();
    }
    return true;
}

bool EventHandler::handleInput(std::optional<sf::Event> event)
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
