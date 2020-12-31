#include <BlurpEngine.h>
#include <Settings.h>
#include <glm/glm.hpp>
#include <Window.h>
#include <RenderResourceManager.h>
#include "Game.h"
#include "GameLoop.h"

#include <iostream>

int main()
{
    using namespace blurp;

    //Window and rendering system setup.

    std::cout << "Starting application" << std::endl;

    BlurpEngine engine;
    BlurpSettings blurpSettings;
    blurpSettings.graphicsAPI = GraphicsAPI::OPENGL;
    blurpSettings.shadersPath = "../Output/shaders/";

    WindowSettings windowSettings;
    windowSettings.dimensions = glm::vec2{ 800, 800 };
    windowSettings.type = WindowType::WINDOW_WIN32;
    windowSettings.name = "My lovely little window";
    windowSettings.flags = WindowFlags::CAPTURE_CURSOR | WindowFlags::HIDE_CURSOR;
    windowSettings.swapChainSettings.vsync = true;
    windowSettings.swapChainSettings.renderTargetSettings.viewPort = { 0, 0, windowSettings.dimensions };

    windowSettings.swapChainSettings.renderTargetSettings.depthBits = 32;
    windowSettings.swapChainSettings.renderTargetSettings.stencilBits = 0;

    blurpSettings.windowSettings = windowSettings;
    engine.Init(blurpSettings);
    auto window = engine.GetWindow();

    std::cout << "Setting up game." << std::endl;

    //Set up the game and main game loop.
    Game game(engine);
    game.Init();

    std::cout << "Game setup completed. Now starting game loop" << std::endl;

    utilities::GameLoop loop(120, 60);

    /*
     * Main loop. Render as long as the window remains open.
     */
    while (!window->IsClosed())
    {
        auto data = loop.update();

        if(data.tick)
        {
            //Update the controls.
            game.UpdateInput(window);

            //Update game logic
            game.UpdateGame(data.deltaTick);
        }

        if(data.frame)
        {
            //Draw the next frame.
            game.Render();

            //Finally display on the screen.
            window->Present();
        }

         if(data.currentTick % 20 == 0)
         {
             std::cout << "FPS: " << data.fps << std::endl;
         }

        //Clean up after everything else happened.
        if(data.tick)
        {
            //Clear all resources that have gone out of scope.
            engine.GetResourceManager().CleanUpUnused();
        }
    }

    std::cout << "Closing down." << std::endl;
    return 0;
}
