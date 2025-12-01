#include "../shared/layer/app.h"
#include "../shared/layer/debug.h"
#include "core_game/sprite/sprite.h"
#include "core_game/texturemanager/texturemanager.h"
#include "core_game/window/window.h"

#include <memory>
#include <spdlog/spdlog.h>

std::string gPROJECT_NAME = "Networking Project";
std::string gACTOR_NAME = "CLIENT";
sf::Vector2f gDEFAULT_SPRITE_SCALING = {5.0f, 5.0f};
sf::Vector2f gDEFAULT_PLAYER_POSITION = {0.0f, 0.0f};
sf::Vector2f gDEFAULT_SPRITE_ORIGIN = {0.0f, 0.0f};
float PLAYER_SPEED = 50.f;

int main()
{
    spdlog::set_level(spdlog::level::debug);

    spdlog::info("Running " + gPROJECT_NAME);

    WindowConfig windowConfig = WindowConfig({800, 400}, {0, 0}, gACTOR_NAME);
    Window window = Window(&windowConfig);
    window.init();

    sf::Vector2f playerStartingWorldPosition = {
        window.config->windowSize.x / 2.f, window.config->windowSize.y / 2.f
    };

    TextureManagerConfig textureManagerConfig = TextureManagerConfig();
    TextureManager textureManager = TextureManager(&textureManagerConfig);
    textureManager.init();

    SpriteConfig defaultShipSpriteConfig = SpriteConfig(
        &textureManager.defaultShipTexture, playerStartingWorldPosition,
        {5.f, 5.f}
    );
    std::unique_ptr<Sprite> defaultShipSprite =
        std::make_unique<Sprite>(&defaultShipSpriteConfig);
    defaultShipSprite->init();

    AppLayerConfig appLayerConfig = AppLayerConfig(&window);
    AppLayer appLayer = AppLayer(&appLayerConfig);

    ShipActorState initialPlayerState = ShipActorState(
        std::move(defaultShipSprite), {1.f, 0.f}, {0.f, 0.f}, 10.f, 0.f, 10.f,
        ShipTurnDirection::NONE
    );
    appLayer.addPlayer(std::move(initialPlayerState));

    DebugLayerConfig debugLayerConfig = DebugLayerConfig(&window);
    DebugLayer debugLayer = DebugLayer(&debugLayerConfig);

    std::vector<Layer *> layers = {&appLayer, &debugLayer};
    for (Layer *layer : layers)
        layer->init();

    spdlog::info("Running app");
    while (window.window.isOpen())
    {
        window.clear(sf::Color::Black);
        for (Layer *layer : layers)
            layer->render();
        window.present();
    }
    return 0;
}
