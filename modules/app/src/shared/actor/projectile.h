#pragma once

#include "base.h"

struct ProjectileActorBuilderContext : public BaseActorBuilderContext
{
    ProjectileActorBuilderContext(
        sf::Texture *initialTexture, sf::Vector2f initialPosition,
        sf::Vector2f initialForwardVector, std::string shooterHandle,
        float lifetime = 0.f
    );
    ~ProjectileActorBuilderContext() = default;
    std::string shooterHandle;
    float lifetime = 0.f;
};

class ProjectileActor : public BaseActor
{
    public:
    ProjectileActor() = default;
    ~ProjectileActor() = default;
    void init(BaseActorBuilderContext *context) override;
    void update(float deltaTime) override;

    std::string shooterHandle;

    private:
    float lifetime = 0.f;
};
