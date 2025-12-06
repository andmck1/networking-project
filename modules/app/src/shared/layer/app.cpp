#include "app.h"

#include "../../shared/config/settings.h"
#include <algorithm>
#include <spdlog/spdlog.h>
#include <SFML/Window.hpp>

AppLayerConfig::AppLayerConfig(
    Window *window, TextureManager *textureManager, bool isServer
)
{
    this->window = window;
    this->textureManager = textureManager;
    this->isServer = isServer;
}

void AppLayer::init(BaseLayerConfig *config)
{
    AppLayerConfig *_config = static_cast<AppLayerConfig *>(config);
    this->window = _config->window;
    this->textureManager = _config->textureManager;
    this->isServer = _config->isServer;
}

void AppLayer::initPlayerCamera()
{
    this->playerView = sf::View();
    this->playerView->setSize(Settings::Camera::playerViewSize);
    this->playerView->setCenter(this->player->position);
    this->playerView->setViewport(sf::FloatRect({0.f, 0.f}, {1.f, 1.f}));
    this->window->window.setView(this->playerView.value());
}

void AppLayer::initServerCamera()
{
    this->serverView = sf::View();
    this->serverView->setSize(Settings::Camera::serverViewSize);
    sf::Vector2f viewCentre = {
        Settings::Camera::serverViewSize.x / 2.f,
        Settings::Camera::serverViewSize.y / 2.f
    };
    this->serverView->setCenter(viewCentre);
    this->window->window.setView(this->serverView.value());
}

void AppLayer::render()
{
    this->startFrame();
    this->cleanUpActors();
    this->updateActors();
    this->cleanUpActors();
    this->drawActors();

    if (!this->isServer)
    {
        this->updatePlayerCamera();
    }
    this->nFrames++;
}

void AppLayer::startFrame()
{
    this->deltaTime = this->clock.restart();
    this->totalTime += this->deltaTime;

    // for physics make sure delta time isn't zero or too high
    this->deltaTime = this->deltaTime.asSeconds() > 0.f ? this->deltaTime
                                                        : sf::milliseconds(1.f);
    this->deltaTime =
        this->deltaTime.asSeconds() > 1.f ? sf::seconds(1.f) : this->deltaTime;
}

void AppLayer::updateActors()
{
    if (this->player)
    {
        if (this->isConnectedToServer)
            this->player->sprite->setColor(Settings::Actor::connectedMask);
        else
            this->player->sprite->setColor(Settings::Actor::disconnectedMask);
    }
    for (const std::unique_ptr<BaseActor> &actor : this->actors)
    {
        actor->update(this->deltaTime.asSeconds());
    }

    for (ShipActor *ship : this->ships)
    {
        if (ship->isEnabled)
        {
            ship->sprite->setColor(Settings::Actor::connectedMask);
            switch (ship->currentTurnDirection)
            {
                case (ShipTurnDirection::LEFT):
                {
                    ship->createSprite(ship->shipTextures->left);
                    break;
                }
                case (ShipTurnDirection::RIGHT):
                {
                    ship->createSprite(ship->shipTextures->right);
                    break;
                }
                default:
                {
                    ship->createSprite(ship->shipTextures->forward);
                    break;
                }
            }
            for (ProjectileActor *projectile : this->projectiles)
            {
                if ((ship->serverId != projectile->shooterServerId) &&
                    (!projectile->isDeleted))
                {
                    if (ship->sprite->getGlobalBounds().findIntersection(
                            projectile->sprite->getGlobalBounds()
                        ))
                    {
                        if (this->isServer)
                        {
                            ship->health -= 5.f;
                            std::unique_ptr<HitMessage> hitEvent =
                                std::make_unique<HitMessage>();
                            hitEvent->hitShipId = ship->serverId;
                            hitEvent->time = this->totalTime.asSeconds();
                            this->hitEvents.push_back(std::move(hitEvent));
                        }
                        // TODO if the projectile doesn't hit on the server this
                        // could be strange
                        projectile->isDeleted = true;
                        break;
                    }
                }
            }
        }
        else
        {
            ship->sprite->setColor(Settings::Actor::disconnectedMask);
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
    // TODO - add ship should be a function to handle map updates
    this->ships.push_back(this->player);
    this->shipActorMap.insert({initalPlayerState->serverId, this->player});
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

    int randomTextureIndex = this->textureManager->getRandomTextureIndex();
    ShipActorBuilderContext initialShipState = ShipActorBuilderContext(
        &this->textureManager->shipTextures[randomTextureIndex],
        initialPosition, initialForwardVector, ShipTurnDirection::NONE, serverId
    );
    this->addActor<ShipActor>(&initialShipState);
    ShipActor *newServerShip =
        static_cast<ShipActor *>(this->actors.back().get());
    this->shipActorMap.insert({serverId, newServerShip});
    this->ships.push_back(static_cast<ShipActor *>(this->actors.back().get()));
}

void AppLayer::networkUpdate(
    const std::vector<std::unique_ptr<Message>> &networkUpdates,
    const std::vector<std::unique_ptr<ClientConnection>> &clients
)
{
    for (const std::unique_ptr<Message> &networkUpdate : networkUpdates)
    {
        int32_t clientId = networkUpdate->id;
        switch (networkUpdate->type)
        {
            case (MessageType::Ship):
            {
                ShipMessage *shipMessage =
                    static_cast<ShipMessage *>(networkUpdate.get());
                if (this->shipActorMap.find(clientId) !=
                    this->shipActorMap.end())
                {
                    this->shipActorMap[clientId]->networkUpdate(shipMessage);
                }
                else
                {
                    this->addServerShip(
                        clientId, shipMessage->position,
                        shipMessage->forwardVector
                    );
                }
                break;
            }
            case (MessageType::Shoot):
            {
                ShootMessage *shootMessage =
                    static_cast<ShootMessage *>(networkUpdate.get());

                if (this->shipActorMap.find(clientId) !=
                    this->shipActorMap.end())
                {
                    sf::Vector2f estimatedPosition;
                    float estimatedLifetime =
                        this->totalTime.asSeconds() - networkUpdate->time;
                    estimatedPosition = shootMessage->position +
                                        shootMessage->forwardVector *
                                            (Settings::Actor::projectileSpeed *
                                             estimatedLifetime);
                    ProjectileActorBuilderContext projectileBuilderContext =
                        ProjectileActorBuilderContext(
                            &this->textureManager->projectileTexture,
                            estimatedPosition, shootMessage->forwardVector,
                            clientId, estimatedLifetime
                        );
                    this->addProjectile(&projectileBuilderContext);
                }
                else
                {
                    spdlog::error(
                        "Tried to add projectile from non-existant player"
                    );
                }
                break;
            }
            case (MessageType::Hit):
            {
                HitMessage *hitMessage =
                    static_cast<HitMessage *>(networkUpdate.get());
                int32_t hitShipId = hitMessage->hitShipId;
                if (this->shipActorMap.find(hitShipId) !=
                    this->shipActorMap.end())
                {
                    this->shipActorMap[hitShipId]->health -= 5;
                }
                break;
            }
            default:
            {
                spdlog::warn("Unhandled message type sent");
                break;
            }
        }
    }

    for (const std::unique_ptr<ClientConnection> &client : clients)
    {
        int32_t clientId = client->id;
        if (this->shipActorMap.find(clientId) != this->shipActorMap.end())
        {
            ShipActor *ship = shipActorMap[clientId];
            if (client->connectionState.isDisconnected())
            {
                ship->isEnabled = false;
            }
            else
            {
                ship->isEnabled = true;
            }
        }
    }
}

void AppLayer::resetTime(float newStartTime)
{
    this->totalTime = sf::seconds(newStartTime);
}
