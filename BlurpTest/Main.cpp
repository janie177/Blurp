#include <BlurpEngine.h>
#include <Settings.h>
#include <glm/glm.hpp>
#include <Window.h>

#include "RenderResourceManager.h"
#include <glm/gtc/type_ptr.hpp>


#include "MaterialTestScene.h"
#include "Scene.h"
#include "UniverseScene.h"

int main()
{
    using namespace blurp;
    //SETUP

    BlurpEngine engine;
    BlurpSettings blurpSettings;
    blurpSettings.graphicsAPI = GraphicsAPI::OPENGL;
    blurpSettings.shadersPath = "../Output/shaders/";

    WindowSettings windowSettings;
    windowSettings.dimensions = glm::vec2{ 800, 800 };
    windowSettings.type = WindowType::WINDOW_WIN32;
    windowSettings.name = "My lovely little window";
    windowSettings.flags = WindowFlags::CAPTURE_CURSOR | WindowFlags::HIDE_CURSOR;
    windowSettings.swapChainSettings.vsync = false ;
    windowSettings.swapChainSettings.renderTargetSettings.viewPort = { 0, 0, windowSettings.dimensions };

    blurpSettings.windowSettings = windowSettings;
    engine.Init(blurpSettings);
    auto window = engine.GetWindow();

    //RENDERING

    //Load one of the scenes.
    //std::unique_ptr<Scene> scene = std::make_unique<UniverseScene>(engine, window);
    std::unique_ptr<Scene> scene = std::make_unique<MaterialTestScene>(engine, window);
    scene->Init();

    /*
     * Main loop. Render as long as the window remains open.
     */
    while (!window->IsClosed())
    {
        //Update the scene.
        scene->Update();

        //Finally display on the screen.
        window->Present();
    }

    return 0;
}
