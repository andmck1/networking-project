#pragma once
#include "base.h"
#include "core_game/window/window.h"
#include "../actor/base.h"
#include "../actor/ship.h"
#include "../actor/projectile.h"

struct DebugLayerConfig : BaseLayerConfig
{
    DebugLayerConfig(
        Window *window, std::vector<std::unique_ptr<BaseActor>> *actors,
        std::vector<ShipActor *> *ships,
        std::vector<ProjectileActor *> *projectiles,
        bool *isBoundaryBoxesEnabled
    );
    Window *window;
    std::vector<std::unique_ptr<BaseActor>> *actors;
    std::vector<ShipActor *> *ships;
    std::vector<ProjectileActor *> *projectiles;
    bool *isBoundaryBoxesEnabled;
};

class DebugLayer : public BaseLayer
{
    public:
    DebugLayer();
    ~DebugLayer();

    void init(BaseLayerConfig *config) override;
    void render() override;

    private:
    Window *window;
    std::vector<std::unique_ptr<BaseActor>> *actors;
    std::vector<ShipActor *> *ships;
    std::vector<ProjectileActor *> *projectiles;
    bool *isBoundaryBoxesEnabled;
};
