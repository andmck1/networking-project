#pragma once
#include "app.h"
#include "base.h"
#include "core_game/window/window.h"
#include "../../shared/networking/clientconnection.h"
#include "../../client/networking/client.h"

struct DebugLayerConfig : BaseLayerConfig
{
    DebugLayerConfig(Window *window, AppLayer *appLayer, bool isServer);
    Window *window;
    AppLayer *appLayer;
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
    std::vector<std::unique_ptr<ClientConnection>> *clientConnections = nullptr;
    Client *client = nullptr;
    ShipActor *player = nullptr;
    bool *isClientConnectionEnabled = nullptr;

    private:
    Window *window;
    AppLayer *appLayer;
    bool isServer;
};
