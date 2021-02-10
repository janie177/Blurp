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
#include "TriangleScene.h"

int main()
{
    using namespace blurp;
    //SETUP

    BlurpEngine engine;
    BlurpSettings blurpSettings;

    //Choose the graphics API.
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



    //Material batch settings.
    MaterialBatchInfo batch;
    batch.path = "materials/Batch/";
    batch.mask.EnableAttribute(MaterialAttribute::DIFFUSE_TEXTURE);
    batch.mask.EnableAttribute(MaterialAttribute::OCCLUSION_TEXTURE);
    batch.mask.EnableAttribute(MaterialAttribute::EMISSIVE_CONSTANT_VALUE);

    //Batch dimensions.
    batch.textureSettings.dimensions.x = 64;
    batch.textureSettings.dimensions.y = 64;
    batch.materialCount = 3;

    //The textures for each material and the constant data per material.
    batch.diffuse.textureNames = {"testmat1/diffuse.jpg", "testmat2/diffuse.jpg", "testmat3/diffuse.jpg" };
    batch.ao.textureNames = { "testmat1/ao.jpg", "testmat2/ao.jpg", "testmat3/ao.jpg" };
    batch.emissive.constantData = { glm::vec3(0.1f, 0.2f, 0.1f), glm::vec3(0.5f, 0.3f, 0.f), glm::vec3(0.f, 0.f, 0.f) };
    

    constexpr bool rebuild = false;
    constexpr bool rebuildBatch = false;

    if(rebuild)
    {
        auto timepoint = std::chrono::high_resolution_clock::now();
        for (auto& path : toBeCompiled)
        {
            std::cout << "Creating file at " << path << fileName << std::endl;
            info.path = "materials/" + path + "/";
            bool sucess = CreateMaterialFile(info, info.path, fileName, false);
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

    if(rebuildBatch)
    {
        auto timepoint = std::chrono::high_resolution_clock::now();

        bool sucess = CreateMaterialBatchFile(batch, batch.path, "MaterialBatch", false);
        if (!sucess)
        {
            std::cout << "Sad :(" << std::endl;
        }
        else
        {
            std::cout << "Yay Good!" << std::endl;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - timepoint).count();
        std::cout << "Time to compile material batch: " << ms << " milliseconds." << std::endl;
    }


    //RENDERING


    //Load one of the scenes.
    //std::unique_ptr<Scene> scene = std::make_unique<UniverseScene>(engine, window);
    //std::unique_ptr<Scene> scene = std::make_unique<MaterialTestScene>(engine, window);
    std::unique_ptr<Scene> scene = std::make_unique<TriangleScene>(engine, window);
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
