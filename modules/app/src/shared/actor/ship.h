#pragma once

#include "base.h"

#include "../../shared/config/settings.h"
#include "../networking/messages.h"
#include "core_game/texturemanager/texturemanager.h"

enum ShipTurnDirection : int32_t
{
    LEFT = 0,
    RIGHT = 1,
    NONE = 2
};

struct ShipActorBuilderContext : public BaseActorBuilderContext
{
    ShipActorBuilderContext(
        TextureManager::ShipTextures *shipTextures,
        sf::Vector2f initialPosition, sf::Vector2f initialForwardVector,
        int32_t initialTurnDirection, std::string handle, sf::Font *font
    );
    ~ShipActorBuilderContext() = default;

    TextureManager::ShipTextures *shipTextures;
    sf::Font *font;
    int32_t initialTurnDirection;
    float initialTurningCircleRadius;
    std::string handle;
};

class ShipActor : public BaseActor
{
    public:
    ShipActor() = default;
    ~ShipActor() = default;
    void init(BaseActorBuilderContext *context) override;
    void update(float deltaTime) override;

    void drawInfo(Window *window);
    void networkUpdate(ShipMessage *updateMessage);

    std::string handle;
    int32_t currentTurnDirection;
    int32_t health = Settings::Game::shipHealth;
    // TODO - should have a player class that this lives in
    bool isPlayerControlling = false;
    bool isSpawning = true;
    float spawnTimer = 0.f;
    TextureManager::ShipTextures *shipTextures;
    int32_t kills = 0;
    int32_t deaths = 0;
    bool statsUpdated = false;
    sf::Font *font;

    private:
    float turningCircleRadius;
};
