#pragma once
#include "base.h"
#include "core_game/window/window.h"
#include "../actor/base.h"
#include "../actor/ship.h"
#include "../actor/projectile.h"
#include "../../server/networking/server.h"

struct DebugLayerConfig : BaseLayerConfig
{
    DebugLayerConfig(
        Window *window, std::vector<std::unique_ptr<BaseActor>> *actors,
        std::vector<ShipActor *> *ships,
        std::vector<ProjectileActor *> *projectiles,
        bool *isBoundaryBoxesEnabled, float *clientUpdateRate, bool isServer,
        sf::Time *currentGameTime
    );
    Window *window;
    std::vector<std::unique_ptr<BaseActor>> *actors;
    std::vector<ShipActor *> *ships;
    std::vector<ProjectileActor *> *projectiles;
    bool *isBoundaryBoxesEnabled;
    float *clientUpdateRate = nullptr;
    bool isServer;
    sf::Time *currentGameTime;
};

class DebugLayer : public BaseLayer
{
    public:
    DebugLayer();
    ~DebugLayer();

    void init(BaseLayerConfig *config) override;
    void render() override;
    std::vector<std::unique_ptr<ClientConnection>> *clients = nullptr;

    private:
    Window *window;
    std::vector<std::unique_ptr<BaseActor>> *actors;
    std::vector<ShipActor *> *ships;
    std::vector<ProjectileActor *> *projectiles;
    bool *isBoundaryBoxesEnabled;
    float *clientUpdateRate = nullptr;
    bool isServer;
    sf::Time *currentGameTime;
};
