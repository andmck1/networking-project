#include "eventhandler.h"
#include "../../shared/config/settings.h"

EventHandlerConfig::EventHandlerConfig(
    Window *window, ShipActor *player, TextureManager *textureManager,
    AppLayer *appLayer
)
{
    this->window = window;
    this->player = player;
    this->textureManager = textureManager;
    this->appLayer = appLayer;
}

void EventHandler::init(EventHandlerConfig *config)
{
    this->window = config->window;
    this->player = config->player;
    this->textureManager = config->textureManager;
    this->appLayer = config->appLayer;
}

bool EventHandler::handle()
{
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
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A))
    {
        this->player->currentTurnDirection = ShipTurnDirection::LEFT;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D))
    {
        this->player->currentTurnDirection = ShipTurnDirection::RIGHT;
    }
    else
    {
        this->player->currentTurnDirection = ShipTurnDirection::NONE;
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Space))
    {
        ProjectileActorBuilderContext projectileBuilderContext =
            this->createProjecileBuildContext(this->player);
        this->appLayer->addProjectile(&projectileBuilderContext);
    }

    return true;
}

ProjectileActorBuilderContext
EventHandler::createProjecileBuildContext(ShipActor *ship)
{
    return ProjectileActorBuilderContext(
        &this->textureManager->projectileTexture, ship->position,
        ship->forwardVector
    );
}
