#include <BlurpEngine.h>
#include <Settings.h>
#include <glm/glm.hpp>
#include <Window.h>

#include "RenderResourceManager.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <MaterialFile.h>



#include "LightTestScene.h"
#include "MaterialTestScene.h"
#include "Scene.h"
#include "ShadowTestScene.h"
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
    windowSettings.swapChainSettings.vsync = true ;
    windowSettings.swapChainSettings.renderTargetSettings.viewPort = { 0, 0, windowSettings.dimensions };

    windowSettings.swapChainSettings.renderTargetSettings.depthBits = 32;
    windowSettings.swapChainSettings.renderTargetSettings.stencilBits = 0;

    blurpSettings.windowSettings = windowSettings;
    engine.Init(blurpSettings);
    auto window = engine.GetWindow();


    /*
     * Material compilation
     */

    std::string fileName = "Material";

    MaterialInfo info;
    info.path = "materials/";
    info.mask.EnableAttribute(MaterialAttribute::DIFFUSE_TEXTURE);
    info.mask.EnableAttribute(MaterialAttribute::NORMAL_TEXTURE);
    info.mask.EnableAttribute(MaterialAttribute::METALLIC_TEXTURE);
    info.mask.EnableAttribute(MaterialAttribute::ROUGHNESS_TEXTURE);
    info.mask.EnableAttribute(MaterialAttribute::OCCLUSION_TEXTURE);
    info.mask.EnableAttribute(MaterialAttribute::HEIGHT_TEXTURE);
    info.diffuse.textureName = "diffuse.jpg";
    info.ao.textureName = "ao.jpg";
    info.height.textureName = "height.jpg";
    info.normal.textureName = "normal.jpg";
    info.metallic.textureName = "metallic.jpg";
    info.roughness.textureName = "roughness.jpg";
    info.emissive.textureName = "emissive.jpg"; //Disabled because it's not present for all textures.
    

    
    std::vector<std::string> toBeCompiled
    {
        "stone",
        "eggs",
        "roof1",
        "barrels",
        "broken_ice",
        "grass"
    };

    constexpr bool rebuild = false;

    if(rebuild)
    {
        auto timepoint = std::chrono::high_resolution_clock::now();
        for (auto& path : toBeCompiled)
        {
            info.path = "materials/" + path + "/";
            const std::string fullPath = info.path + fileName;
            bool sucess = CreateMaterialFile(info, fullPath);
            if(!sucess)
            {
                std::cout << "Sad :(" << std::endl;
            }
            else
            {
                std::cout << "Yay Good!" << std::endl;
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - timepoint).count();
        std::cout << "Time to convert " << toBeCompiled.size() << " materials: " << ms << " milliseconds." << std::endl;
    }


    //RENDERING


    //Load one of the scenes.
    //std::unique_ptr<Scene> scene = std::make_unique<UniverseScene>(engine, window);
    std::unique_ptr<Scene> scene = std::make_unique<MaterialTestScene>(engine, window);
    //std::unique_ptr<Scene> scene = std::make_unique<LightTestScene>(engine, window);
    //std::unique_ptr<Scene> scene = std::make_unique<ShadowTestScene>(engine, window);
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

        //Clear all resources that have gone out of scope.
        engine.GetResourceManager().CleanUpUnused();
    }

    return 0;
}
