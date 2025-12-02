#include "app.h"

#include <optional>
#include <spdlog/spdlog.h>
#include <SFML/Window.hpp>

AppLayerConfig::AppLayerConfig(Window *window)
{
    this->window = window;
}

void AppLayer::init(BaseLayerConfig *config)
{
    AppLayerConfig *_config = static_cast<AppLayerConfig *>(config);
    this->window = _config->window;
}

bool AppLayer::render()
{
    this->startFrame();
    if (!this->handleInput())
        return false;
    this->updateActors();
    this->drawActors();

    return true;
}

void AppLayer::startFrame()
{
    this->deltaTime = this->clock.getElapsedTime();
    this->clock.restart();
}

bool AppLayer::handleInput()
{
    std::optional<sf::Event> event = this->window->window.pollEvent();
    while (event)
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Escape))
        {
            this->window->window.close();
            return false;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A))
        {
            this->player->setTurnDirection(ShipTurnDirection::LEFT);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D))
        {
            this->player->setTurnDirection(ShipTurnDirection::RIGHT);
        }
        else
        {
            this->player->setTurnDirection(ShipTurnDirection::NONE);
        }

        event = this->window->window.pollEvent();
    }
    return true;
}

void AppLayer::updateActors()
{
    for (const std::unique_ptr<BaseActor> &actor : this->actors)
    {
        actor->update(this->deltaTime.asSeconds());
    }
}

void AppLayer::drawActors()
{
    for (const std::unique_ptr<BaseActor> &actor : this->actors)
    {
        actor->draw(this->window);
    }
}

template <typename ActorType, typename ActorStateType>
void AppLayer::addActor(ActorStateType *initialActorState)
{
    spdlog::debug("Adding actor");
    std::unique_ptr<ActorType> actor = std::make_unique<ActorType>();
    actor->init(initialActorState);
    this->actors.push_back(std::move(actor));
}

void AppLayer::addPlayer(ShipActorBuilderContext *initalPlayerState)
{
    spdlog::debug("Adding player");
    this->addActor<ShipActor>(initalPlayerState);
    this->player = dynamic_cast<ShipActor *>(this->actors.back().get());
}
