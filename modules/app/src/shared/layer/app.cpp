#include "app.h"

#include "../../shared/config/settings.h"
#include <algorithm>
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

void AppLayer::initPlayerCamera()
{
    this->playerView = sf::View();
    this->playerView->setSize(Settings::Camera::viewSize);
    this->playerView->setCenter(this->player->position);
    this->playerView->setViewport(sf::FloatRect({0.f, 0.f}, {1.f, 1.f}));
    this->window->window.setView(this->playerView.value());
}

void AppLayer::render()
{
    this->startFrame();
    this->updateActors();
    this->drawActors();
    if (this->playerView)
        this->updatePlayerCamera();
}

void AppLayer::startFrame()
{
    this->deltaTime = this->clock.getElapsedTime();
    this->clock.restart();
}

void AppLayer::updateActors()
{
    for (const std::unique_ptr<BaseActor> &actor : this->actors)
    {
        actor->update(this->deltaTime.asSeconds());
    }

    std::erase_if(
        this->actors, [](const auto &actor) { return actor->isDeleted; }
    );
}

void AppLayer::drawActors()
{
    for (const std::unique_ptr<BaseActor> &actor : this->actors)
    {
        actor->draw(this->window);
    }
}

void AppLayer::updatePlayerCamera()
{
    this->playerView->setCenter(this->player->position);
    this->window->window.setView(this->playerView.value());
}

template <typename ActorType, typename ActorStateType>
void AppLayer::addActor(ActorStateType *initialActorState)
{
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

void AppLayer::addProjectile(
    ProjectileActorBuilderContext *initialProjectileState
)
{
    this->addActor<ProjectileActor>(initialProjectileState);
}

ShipActor *AppLayer::getPlayer()
{
    return this->player;
}
