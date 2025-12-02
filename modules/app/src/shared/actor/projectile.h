#pragma once

#include "base.h"

struct ProjectileActorBuilderContext : public BaseActorBuilderContext
{
    ProjectileActorBuilderContext(
        sf::Texture *initialTexture, sf::Vector2f initialPosition,
        sf::Vector2f initialForwardVector
    );
    ~ProjectileActorBuilderContext() = default;
};

class ProjectileActor : public BaseActor
{
    public:
    ProjectileActor() = default;
    ~ProjectileActor() = default;
    void init(BaseActorBuilderContext *context) override;
    void update(float deltaTime) override;

    private:
    float lifetime = 0.f;
};
