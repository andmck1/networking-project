#pragma once

#include <SFML/Graphics.hpp>

struct BaseLayerConfig
{
};

class BaseLayer
{
    public:
    virtual void init(BaseLayerConfig *config) = 0;
    virtual void render() = 0;
};
