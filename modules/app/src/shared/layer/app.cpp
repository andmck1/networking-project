#include "app.h"

#include <optional>
#include <spdlog/spdlog.h>
#include <SFML/Window.hpp>

AppLayerConfig::AppLayerConfig(Window *window)
{
    this->window = window;
}

AppLayer::AppLayer(AppLayerConfig *config) : state()
{
    this->config = config;
}

void AppLayer::init()
{
    spdlog::debug("Initialising app");
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
    this->state.deltaTime = this->state.clock.getElapsedTime();
    this->state.clock.restart();
}

bool AppLayer::handleInput()
{
    std::optional<sf::Event> event = this->config->window->window.pollEvent();
    while (event)
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Escape))
        {
            this->config->window->window.close();
            return false;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A))
        {
            this->state.player->setTurnDirection(ShipTurnDirection::LEFT);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D))
        {
            this->state.player->setTurnDirection(ShipTurnDirection::RIGHT);
        }

        event = this->config->window->window.pollEvent();
    }
    return true;
}

void AppLayer::updateActors()
{
    for (const std::unique_ptr<BaseActor> &actor : this->state.actors)
    {
        actor->update(this->state.deltaTime.asSeconds());
    }
}

void AppLayer::drawActors()
{
    for (const std::unique_ptr<BaseActor> &actor : this->state.actors)
    {
        actor->draw(this->config->window);
    }
}

template <typename ActorType, typename ActorStateType>
void AppLayer::addActor(ActorStateType &&initialActorState)
{
    spdlog::debug("Adding actor");
    this->state.actors.push_back(
        std::make_unique<ActorType>(std::move(initialActorState))
    );
}

void AppLayer::addPlayer(std::unique_ptr<ShipActorState> initalPlayerState)
{
    spdlog::debug("Adding player");
    this->addActor<ShipActor>(std::move(initalPlayerState));
    this->state.player =
        dynamic_cast<ShipActor *>(this->state.actors.back().get());
}
