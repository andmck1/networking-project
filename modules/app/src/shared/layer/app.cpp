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
    this->cleanUpActors();
    this->updateActors();
    this->cleanUpActors();
    this->drawActors();
    if (this->playerView)
    {
        this->updatePlayerCamera();
    }
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

    for (ShipActor *ship : this->ships)
    {
        for (ProjectileActor *projectile : this->projectiles)
        {
            if ((ship->serverId != projectile->shooterServerId) &&
                (!projectile->isDeleted))
            {
                if (ship->sprite->getGlobalBounds().findIntersection(
                        projectile->sprite->getGlobalBounds()
                    ))
                {
                    ship->health -= 5.f;
                    projectile->isDeleted = true;
                    break;
                }
            }
        }
    }
}

void AppLayer::cleanUpActors()
{
    std::erase_if(
        this->projectiles, [](const auto &actor) { return actor->isDeleted; }
    );
    std::erase_if(
        this->ships, [](const auto &actor) { return actor->isDeleted; }
    );
    std::erase_if(
        this->actors, [](const auto &actor) { return actor->isDeleted; }
    );
}

void AppLayer::drawActors()
{
    for (const std::unique_ptr<BaseActor> &actor : this->actors)
    {
        actor->draw(this->window, this->isBoundaryBoxesEnabled);
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
    this->ships.push_back(this->player);
}

void AppLayer::addProjectile(
    ProjectileActorBuilderContext *initialProjectileState
)
{
    this->addActor<ProjectileActor>(initialProjectileState);
    this->projectiles.push_back(
        static_cast<ProjectileActor *>(this->actors.back().get())
    );
}

ProjectileActorBuilderContext
AppLayer::createProjecileBuildContext(ShipActor *ship)
{
    return ProjectileActorBuilderContext(
        &this->textureManager->projectileTexture, ship->position,
        ship->forwardVector, ship->serverId
    );
}

void AppLayer::addShootEvent(sf::Vector2f position, sf::Vector2f forwardVector)
{
    std::unique_ptr<ShootMessage> shootEvent = std::make_unique<ShootMessage>();
    shootEvent->forwardVector = forwardVector;
    shootEvent->position = position;
    this->shootEvents.push_back(std::move(shootEvent));
}

void AppLayer::addServerShip(
    int32_t serverId, sf::Vector2f initialPosition,
    sf::Vector2f initialForwardVector
)
{
    spdlog::debug("Adding ship");
    ShipActorBuilderContext initialShipState = ShipActorBuilderContext(
        &this->textureManager->defaultShipTexture, initialPosition,
        initialForwardVector, ShipTurnDirection::NONE, serverId
    );
    this->addActor<ShipActor>(&initialShipState);
    ShipActor *newServerShip =
        static_cast<ShipActor *>(this->actors.back().get());
    this->shipActorMap.insert({serverId, newServerShip});
    this->ships.push_back(static_cast<ShipActor *>(this->actors.back().get()));
}

void AppLayer::networkUpdate(
    const std::vector<std::unique_ptr<Message>> &networkUpdates
)
{
    for (const std::unique_ptr<Message> &networkUpdate : networkUpdates)
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
            case (MessageType::Shoot):
            {
                int32_t shipActorServerId = networkUpdate->id;
                ShootMessage *shootMessage =
                    static_cast<ShootMessage *>(networkUpdate.get());

                if (this->shipActorMap.find(shipActorServerId) !=
                    this->shipActorMap.end())
                {
                    ProjectileActorBuilderContext projectileBuilderContext =
                        ProjectileActorBuilderContext(
                            &this->textureManager->projectileTexture,
                            shootMessage->position, shootMessage->forwardVector,
                            shipActorServerId
                        );
                    this->addProjectile(&projectileBuilderContext);
                }
                else
                {
                    spdlog::error(
                        "Tried to add projectile from non-existant player"
                    );
                }
            }
        }
    }
}
