#include "../shared/layer/app.h"
#include "../shared/layer/debug.h"
#include "../shared/config/settings.h"
#include "core_game/texturemanager/texturemanager.h"
#include "core_game/window/window.h"
#include "eventhandler/eventhandler.h"

#include <spdlog/spdlog.h>

int main()
{
    spdlog::set_level(spdlog::level::debug);

    spdlog::info("Running " + Settings::App::appName);

    std::unique_ptr<WindowConfig> windowConfig = std::make_unique<WindowConfig>(
        Settings::Window::windowSize, Settings::Window::windowOrigin,
        Settings::App::appName
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
        &textureManager.defaultShipTexture, playerStartingWorldPosition,
        {0, -1}, ShipTurnDirection::NONE
    );
    appLayer.addPlayer(&initialPlayerState);
    appLayer.initPlayerCamera();

    DebugLayerConfig debugLayerConfig = DebugLayerConfig(&window);
    DebugLayer debugLayer = DebugLayer();
    debugLayer.init(&debugLayerConfig);

    EventHandlerConfig eventHandlerConfig = EventHandlerConfig(
        &window, appLayer.getPlayer(), &textureManager, &appLayer
    );
    EventHandler eventHandler = EventHandler();
    eventHandler.init(&eventHandlerConfig);

    std::vector<BaseLayer *> layers = {&appLayer, &debugLayer};

    spdlog::info("Running app");
    sf::Sprite backgroundSprite(textureManager.backgroundTexture);
    while (window.window.isOpen())
    {
        eventHandler.handle();
        window.clear(Settings::Window::clearColour);
        window.window.draw(backgroundSprite);
        for (BaseLayer *layer : layers)
            layer->render();
        window.present();
    }
    return 0;
}
