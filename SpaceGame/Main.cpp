#include <BlurpEngine.h>
#include <Settings.h>
#include <glm/glm.hpp>
#include <Window.h>
#include <RenderResourceManager.h>
#include "Game.h"
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

    //Fps measuring
    auto timeStamp = std::chrono::high_resolution_clock::now();

    /*
     * Main loop. Render as long as the window remains open.
     */
    while (!window->IsClosed())
    {
        //Update the controls.
        game.UpdateInput(window);

        //Update game logic
        game.UpdateGame();

        //Draw the next frame.
        game.Render();

        //Finally display on the screen.
        window->Present();

        //Clear all resources that have gone out of scope.
        engine.GetResourceManager().CleanUpUnused();

        //Print FPS
        
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - timeStamp);
        float seconds = 1000000.f / static_cast<float>(duration.count());
        static float secs = 0;
        secs += seconds;
        static int counter = 0;
        ++counter;

        if(counter > 50)
        {
            float fps = secs / (float)counter;
            counter = 0;
            std::cout << "FPS: " << fps << std::endl;
            secs = 0.f;
            
        }
        timeStamp = now;
    }

    std::cout << "Closing down." << std::endl;
    return 0;
}
