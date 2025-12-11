#include "app.h"

#include "../../shared/config/settings.h"
#include <algorithm>
#include <cmath>
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
            // Sprite updates
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

            // Projectile collisions
            for (ProjectileActor *projectile : this->projectiles)
            {
                if ((ship->handle != projectile->shooterHandle) &&
                    (!projectile->isDeleted))
                {
                    if (ship->sprite->getGlobalBounds().findIntersection(
                            projectile->sprite->getGlobalBounds()
                        ))
                    {
                        if (this->isServer)
                        {
                            ship->health -= Settings::Game::projectileDamage;
                            std::unique_ptr<HitMessage> hitEvent =
                                std::make_unique<HitMessage>();
                            hitEvent->hitShipHandle = ship->handle;
                            hitEvent->time = this->totalTime.asSeconds();
                            hitEvent->newHealth = ship->health;
                            this->hitEvents.push_back(std::move(hitEvent));
                        }
                        // TODO if the projectile doesn't hit on the server this
                        // could be strange
                        projectile->isDeleted = true;
                        break;
                    }
                }
            }

            // Ship collisions
            if (this->isServer)
            {
                for (ShipActor *otherShip : this->ships)
                {
                    if (otherShip != ship)
                    {
                        if (otherShip->isEnabled)
                        {
                            if (ship->sprite->getGlobalBounds()
                                    .findIntersection(
                                        otherShip->sprite->getGlobalBounds()
                                    ))
                            {
                                // TODO - This will double up ship hits but fine
                                // for now
                                ship->health = 0.f;
                                ship->isEnabled = false;
                                otherShip->health = 0.f;
                                otherShip->isEnabled = false;

                                std::unique_ptr<HitMessage> shipHitEvent =
                                    std::make_unique<HitMessage>();
                                shipHitEvent->hitShipHandle = ship->handle;
                                shipHitEvent->time =
                                    this->totalTime.asSeconds();
                                shipHitEvent->newHealth = ship->health;
                                this->hitEvents.push_back(
                                    std::move(shipHitEvent)
                                );

                                std::unique_ptr<HitMessage> otherShipHitEvent =
                                    std::make_unique<HitMessage>();
                                otherShipHitEvent->hitShipHandle =
                                    otherShip->handle;
                                otherShipHitEvent->time =
                                    this->totalTime.asSeconds();
                                otherShipHitEvent->newHealth =
                                    otherShip->health;
                                this->hitEvents.push_back(
                                    std::move(otherShipHitEvent)
                                );
                            }
                        }
                    }
                    if (!ship->isEnabled)
                        break;
                }
            }
        }
        else
        {
            ship->sprite->setColor(Settings::Actor::disconnectedMask);
        }

        // Respawn
        if (ship->health <= 0.f)
        {
            ship->isEnabled = false;
            ship->isSpawning = true;
            if (ship == this->player)
                // Chance the player can respawn on another player but very
                // small and they have invincibility
                this->player->position = this->getRespawnPoint();
        }

        if (ship->isSpawning)
        {
            ship->spawnTimer += this->deltaTime.asSeconds();
            ship->health = 100.f;
            if (ship->spawnTimer > Settings::Game::respawnTime)
            {
                ship->spawnTimer = 0.f;
                ship->isSpawning = false;
                ship->isEnabled = true;
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
    // TODO - add ship should be a function to handle map updates
    this->ships.push_back(this->player);
    this->shipActorMap.insert({initalPlayerState->handle, this->player});
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
        ship->forwardVector, ship->handle
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
    std::string handle, sf::Vector2f initialPosition,
    sf::Vector2f initialForwardVector
)
{
    spdlog::debug("Adding ship");

    int randomTextureIndex = this->textureManager->getRandomTextureIndex();
    ShipActorBuilderContext initialShipState = ShipActorBuilderContext(
        &this->textureManager->shipTextures[randomTextureIndex],
        initialPosition, initialForwardVector, ShipTurnDirection::NONE, handle
    );
    this->addActor<ShipActor>(&initialShipState);
    ShipActor *newServerShip =
        static_cast<ShipActor *>(this->actors.back().get());
    newServerShip->isSpawning = true;
    newServerShip->isEnabled = false;
    this->shipActorMap.insert({handle, newServerShip});
    this->ships.push_back(static_cast<ShipActor *>(this->actors.back().get()));
}

void AppLayer::networkUpdate(
    const std::vector<std::unique_ptr<Message>> &networkUpdates,
    const std::vector<std::unique_ptr<ClientConnection>> &clients
)
{
    for (const std::unique_ptr<Message> &networkUpdate : networkUpdates)
    {
        std::string clientHandle = networkUpdate->handle;
        switch (networkUpdate->type)
        {
            case (MessageType::Ship):
            {
                ShipMessage *shipMessage =
                    static_cast<ShipMessage *>(networkUpdate.get());
                if (this->shipActorMap.find(clientHandle) !=
                    this->shipActorMap.end())
                {
                    this->shipActorMap[clientHandle]->networkUpdate(
                        shipMessage
                    );
                }
                else
                {
                    this->addServerShip(
                        clientHandle, shipMessage->position,
                        shipMessage->forwardVector
                    );
                }
                break;
            }
            case (MessageType::Shoot):
            {
                ShootMessage *shootMessage =
                    static_cast<ShootMessage *>(networkUpdate.get());

                if (this->shipActorMap.find(clientHandle) !=
                    this->shipActorMap.end())
                {
                    sf::Vector2f estimatedPosition;
                    float estimatedLifetime =
                        this->totalTime.asSeconds() - networkUpdate->time;
                    estimatedPosition = shootMessage->position +
                                        shootMessage->forwardVector *
                                            (Settings::Game::projectileSpeed *
                                             estimatedLifetime);
                    ProjectileActorBuilderContext projectileBuilderContext =
                        ProjectileActorBuilderContext(
                            &this->textureManager->projectileTexture,
                            estimatedPosition, shootMessage->forwardVector,
                            clientHandle, estimatedLifetime
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
                std::string hitShipHandle = hitMessage->hitShipHandle;
                if (this->shipActorMap.find(hitShipHandle) !=
                    this->shipActorMap.end())
                {
                    this->shipActorMap[hitShipHandle]->health =
                        hitMessage->newHealth;
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
        std::string clientHandle = client->handle;
        if (this->shipActorMap.find(clientHandle) != this->shipActorMap.end())
        {
            ShipActor *ship = shipActorMap[clientHandle];
            if (client->connectionState.isDisconnected())
            {
                ship->isEnabled = false;
            }
            else if (!ship->isSpawning)
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

sf::Vector2f AppLayer::getRespawnPoint()
{
    std::vector<sf::Vector2f> allowedRespawnPoints = {};
    sf::Vector2u windowSize = this->window->getWindowSize();
    for (unsigned int xGridIndex = 0; xGridIndex < windowSize.x;
         xGridIndex += Settings::Game::respawnGridStep)
    {
        for (unsigned int yGridIndex = 0; yGridIndex < windowSize.y;
             yGridIndex += Settings::Game::respawnGridStep)
        {
            sf::Vector2f respawnPoint = {
                (float) xGridIndex, (float) yGridIndex
            };
            respawnPoint.x *= Settings::Camera::serverViewSize.x /
                              Settings::Window::windowSize.x;
            respawnPoint.y *= Settings::Camera::serverViewSize.y /
                              Settings::Window::windowSize.y;

            bool isRespawnPointFarFromOtherPlayers = true;
            for (ShipActor *ship : this->ships)
            {
                if (ship != this->player)
                {
                    sf::Vector2f distanceVector = respawnPoint - ship->position;
                    float distance = std::sqrt(
                        distanceVector.x * distanceVector.x +
                        distanceVector.y * distanceVector.y
                    );
                    if (distance < Settings::Game::minRespawnDistance)
                    {
                        isRespawnPointFarFromOtherPlayers = false;
                    }
                }
                if (!isRespawnPointFarFromOtherPlayers)
                    break;
            }

            if (isRespawnPointFarFromOtherPlayers)
                allowedRespawnPoints.push_back(respawnPoint);
        }
    }

    if (allowedRespawnPoints.size() == 0)
    {
        spdlog::warn("No safe respawn points");
        sf::Vector2f respawnPoint =
            static_cast<sf::Vector2f>(this->window->getRandomWindowPosition());
        respawnPoint.x *=
            Settings::Camera::serverViewSize.x / Settings::Window::windowSize.x;
        respawnPoint.y *=
            Settings::Camera::serverViewSize.y / Settings::Window::windowSize.y;
        return respawnPoint;
    }

    std::random_device randomDevice;
    std::mt19937 randomNumberGenerator = std::mt19937(randomDevice());
    std::uniform_int_distribution<unsigned int> randomNumberDistribution =
        std::uniform_int_distribution<unsigned int>(
            0, allowedRespawnPoints.size() - 1
        );
    sf::Vector2f respawnPoint = allowedRespawnPoints.at(
        randomNumberDistribution(randomNumberGenerator)
    );

    return respawnPoint;
}
