#pragma once
#include "core_game/window/window.h"
#include "layer.h"

struct DebugLayerConfig
{
    DebugLayerConfig(Window *window);
    Window *window;
};

class DebugLayer : public Layer
{
    public:
    DebugLayer(DebugLayerConfig *config);
    ~DebugLayer();

    void init() override;
    bool render() override;

    private:
    DebugLayerConfig *config;
};
