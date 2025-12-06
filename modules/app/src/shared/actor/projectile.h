#pragma once

#include "base.h"

struct ProjectileActorBuilderContext : public BaseActorBuilderContext
{
    ProjectileActorBuilderContext(
        sf::Texture *initialTexture, sf::Vector2f initialPosition,
        sf::Vector2f initialForwardVector, int32_t shooterServerId,
        float lifetime = 0.f
    );
    ~ProjectileActorBuilderContext() = default;
    int32_t shooterServerId;
    float lifetime = 0.f;
};

class ProjectileActor : public BaseActor
{
    public:
    ProjectileActor() = default;
    ~ProjectileActor() = default;
    void init(BaseActorBuilderContext *context) override;
    void update(float deltaTime) override;

    int32_t shooterServerId;

    private:
    float lifetime = 0.f;
};
