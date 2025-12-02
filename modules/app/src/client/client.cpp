#include "../shared/layer/app.h"
#include "../shared/layer/debug.h"
#include "core_game/texturemanager/texturemanager.h"
#include "core_game/window/window.h"

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

    std::unique_ptr<WindowConfig> windowConfig = std::make_unique<WindowConfig>(
        sf::Vector2u{1600, 1280}, sf::Vector2u{0, 0}, gACTOR_NAME
    );
    Window window = Window();
    window.init(std::move(windowConfig));

    sf::Vector2f playerStartingWorldPosition =
        static_cast<sf::Vector2f>(window.getWindowCentre());

    TextureManagerConfig textureManagerConfig = TextureManagerConfig();
    TextureManager textureManager = TextureManager(&textureManagerConfig);
    textureManager.init();

    AppLayerConfig appLayerConfig = AppLayerConfig(&window);
    AppLayer appLayer = AppLayer();
    appLayer.init(&appLayerConfig);

    ShipActorBuilderContext initialPlayerState = ShipActorBuilderContext(
        &textureManager.defaultShipTexture, {2.f, 2.f},
        playerStartingWorldPosition, {0, -1}, 200.f, 0.f,
        ShipTurnDirection::NONE, 10.f
    );
    appLayer.addPlayer(&initialPlayerState);

    DebugLayerConfig debugLayerConfig = DebugLayerConfig(&window);
    DebugLayer debugLayer = DebugLayer();
    debugLayer.init(&debugLayerConfig);

    std::vector<BaseLayer *> layers = {&appLayer, &debugLayer};

    spdlog::info("Running app");
    while (window.window.isOpen())
    {
        window.clear(sf::Color::Black);
        for (BaseLayer *layer : layers)
            layer->render();
        window.present();
    }
    return 0;
}
