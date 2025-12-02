#pragma once
#include "core_game/window/window.h"
#include "base.h"

struct DebugLayerConfig : BaseLayerConfig
{
    DebugLayerConfig(Window *window);
    Window *window;
};

class DebugLayer : public BaseLayer
{
    public:
    DebugLayer() = default;
    ~DebugLayer();

    void init(BaseLayerConfig *config) override;
    bool render() override;

    private:
    Window *window;
};
