#include "app.h"

#include "../../shared/config/settings.h"
#include <algorithm>
#include <spdlog/spdlog.h>
#include <SFML/Window.hpp>

AppLayerConfig::AppLayerConfig(Window *window, TextureManager *textureManager)
{
    this->window = window;
    this->textureManager = textureManager;
}

void AppLayer::init(BaseLayerConfig *config)
{
    AppLayerConfig *_config = static_cast<AppLayerConfig *>(config);
    this->window = _config->window;
    this->textureManager = _config->textureManager;
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
    sf::Time newTotalTime = this->clock.getElapsedTime();
    this->deltaTime = newTotalTime - this->totalTime;
    this->deltaTime = this->deltaTime.asSeconds() > 0.f ? this->deltaTime
                                                        : sf::milliseconds(1.f);
    this->totalTime = newTotalTime;
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

void AppLayer::addServerShip(
    int32_t serverId, sf::Vector2f initialPosition,
    sf::Vector2f initialForwardVector
)
{
    spdlog::debug("Adding ship");
    ShipActorBuilderContext initialShipState = ShipActorBuilderContext(
        &this->textureManager->defaultShipTexture, initialPosition,
        initialForwardVector, ShipTurnDirection::NONE
    );
    this->addActor<ShipActor>(&initialShipState);
    ShipActor *newServerShip =
        static_cast<ShipActor *>(this->actors.back().get());
    this->shipActorMap.insert({serverId, newServerShip});
}

void AppLayer::networkUpdate(
    std::vector<std::unique_ptr<Message>> networkUpdates
)
{
    for (std::unique_ptr<Message> &networkUpdate : networkUpdates)
    {
        switch (networkUpdate->type)
        {
            case (MessageType::Ship):
            {
                int32_t shipActorServerId = networkUpdate->id;
                ShipMessage *shipMessage =
                    static_cast<ShipMessage *>(networkUpdate.get());
                if (this->shipActorMap.find(shipActorServerId) !=
                    this->shipActorMap.end())
                {
                    this->shipActorMap[shipActorServerId]->networkUpdate(
                        shipMessage
                    );
                }
                else
                {
                    this->addServerShip(
                        shipActorServerId, shipMessage->position,
                        shipMessage->forwardVector
                    );
                }
                break;
            }
        }
    }
}

ShipActor *AppLayer::getPlayer()
{
    return this->player;
}

float AppLayer::getGameTime()
{
    return this->totalTime.asSeconds();
}

float AppLayer::getDeltaTime()
{
    return this->deltaTime.asSeconds();
}
