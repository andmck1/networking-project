#pragma once

#include <SFML/Graphics.hpp>

class Layer
{
    public:
    virtual void init() = 0;
    virtual bool render() = 0;
};

