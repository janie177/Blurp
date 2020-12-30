#include "Game.h"
#include <KeyCodes.h>
#include <BlurpEngine.h>
#include <RenderResourceManager.h>
#include <Window.h>
#include <RenderPipeline.h>
#include <GpuBuffer.h>
#include <MeshFile.h>

#include "CubeMapLoader.h"
#include "MeshLoader.h"
#include <iostream>

#define SHADOW_MAP_DIMENSION 2048
#define NUM_CASCADES 6
#define FAR_PLANE 2500.f
#define NEAR_PLANE 0.1f
#define NUM_POINT_LIGHT_SHADOWS 2


#define RAND_FLOAT() (static_cast<float>(rand()) / static_cast<float>(RAND_MAX))

Game::Game(blurp::BlurpEngine& a_RenderEngine) : m_Engine(a_RenderEngine),
    m_SpaceShips(10000, sizeof(SpaceShip)),
    m_Asteroids(100000, sizeof(Asteroid)),
    m_Lasers(10000, sizeof(Laser)),
    m_KillBots(10000, sizeof(KillBot)),
    m_Planets(9, sizeof(Planet)), //Reserve space for 9 planets since planet 9 can be found any day now.
    m_Lights(1000, sizeof(Light))
{

}

void Game::Init()
{
    using namespace blurp;
    //Create the pipeline object.
    PipelineSettings pSettings;
    pSettings.waitForGpu = true;
    m_Pipeline = m_Engine.GetResourceManager().CreatePipeline(pSettings);

    //Set the clear color.
    const auto window = m_Engine.GetWindow();
    window->GetRenderTarget()->SetClearColor({ 0.f, 0.f, 0.f, 1.f });

    m_ClearPass = m_Pipeline->AppendRenderPass<RenderPass_Clear>(RenderPassType::RP_CLEAR);
    m_ClearPass->AddRenderTarget(window->GetRenderTarget());

    //Create a camera to use.
    CameraSettings camSettings;
    camSettings.width = window->GetDimensions().x;
    camSettings.height = window->GetDimensions().y;
    camSettings.fov = 90.f;
    camSettings.nearPlane = NEAR_PLANE;
    camSettings.farPlane = FAR_PLANE;
    m_Camera = m_Engine.GetResourceManager().CreateCamera(camSettings);

    //Create some lights and the sun.
    LightSettings lSettings;

    lSettings.type = LightType::LIGHT_DIRECTIONAL;
    lSettings.shadowMapIndex = 0;
    lSettings.intensity = 0.8f;
    lSettings.color = { 1.f, 1.f, 1.f };
    lSettings.directionalLight.direction = glm::vec3(0.f, 0.f, 1.f);
    m_Sun = std::reinterpret_pointer_cast<DirectionalLight>(m_Engine.GetResourceManager().CreateLight(lSettings));

    //Create shadow map generation passes.
    //Generate shadowmaps before doing the forward rendering.
    m_ShadowGenerationPass = m_Pipeline->AppendRenderPass<RenderPass_ShadowMap>(RenderPassType::RP_SHADOWMAP);
    m_ShadowGenerationPass->SetCamera(m_Camera);

    //Positional shadow array.
    TextureSettings shadowPosSettings;
    shadowPosSettings.dimensions = glm::vec3(SHADOW_MAP_DIMENSION, SHADOW_MAP_DIMENSION, NUM_POINT_LIGHT_SHADOWS * 6);
    shadowPosSettings.generateMipMaps = false;
    shadowPosSettings.dataType = DataType::FLOAT;
    shadowPosSettings.pixelFormat = PixelFormat::DEPTH;
    shadowPosSettings.memoryAccess = AccessMode::READ_WRITE;
    shadowPosSettings.memoryUsage = MemoryUsage::GPU;
    shadowPosSettings.textureType = TextureType::TEXTURE_CUBEMAP_ARRAY;
    m_PosShadowArray = m_Engine.GetResourceManager().CreateTexture(shadowPosSettings);

    //Directional shadow array.
    TextureSettings shadowDirSettings;
    shadowDirSettings.dimensions = glm::vec3(SHADOW_MAP_DIMENSION, SHADOW_MAP_DIMENSION, NUM_CASCADES * 1); //1 directional shadow from the sun.
    shadowDirSettings.generateMipMaps = false;
    shadowDirSettings.dataType = DataType::FLOAT;
    shadowDirSettings.pixelFormat = PixelFormat::DEPTH;
    shadowDirSettings.memoryAccess = AccessMode::READ_WRITE;
    shadowDirSettings.memoryUsage = MemoryUsage::GPU;
    shadowDirSettings.textureType = TextureType::TEXTURE_2D_ARRAY;
    m_DirShadowArray = m_Engine.GetResourceManager().CreateTexture(shadowDirSettings);

    //Generate cascade distances for the sun shadow.
    std::vector<float> cascadeDistances;
    cascadeDistances.resize(NUM_CASCADES);
    float cd = 2.f;
    float total = 0.f;
    for (int c = 0; c < NUM_CASCADES; ++c)
    {
        cascadeDistances[c] = cd;
        total += cd;
        cd *= 2.f;
        if (c == NUM_CASCADES - 2)
        {
            cd = FAR_PLANE - total;
        }
    }

    //Clear the dir shadow buffer.
    ClearData dirClear;
    dirClear.size = glm::vec3(SHADOW_MAP_DIMENSION, SHADOW_MAP_DIMENSION, NUM_CASCADES * 1);
    dirClear.clearValue.floats[0] = 1.f;
    m_ClearPass->AddTexture(m_DirShadowArray, dirClear);

    //Clear the shadow textures every frame.
    ClearData posShadowClear;
    posShadowClear.size = glm::vec3(SHADOW_MAP_DIMENSION, SHADOW_MAP_DIMENSION, NUM_POINT_LIGHT_SHADOWS * 6);
    posShadowClear.clearValue.floats[0] = 1.f;
    m_ClearPass->AddTexture(m_PosShadowArray, posShadowClear);

    //Load a skybox;
    m_SkyBoxTexture = LoadCubeMap(m_Engine.GetResourceManager(), CubeMapSettings{
        "materials/skybox1/",
        "up.png",
        "down.png",
        "left.png",
        "right.png",
        "front.png",
        "back.png"
        });

    //Add a skybox pass to the render pipeline.
    m_SkyboxPass = m_Pipeline->AppendRenderPass<RenderPass_Skybox>(RenderPassType::RP_SKYBOX);
    m_SkyboxPass->SetCamera(m_Camera);
    m_SkyboxPass->SetTarget(window->GetRenderTarget());
    m_SkyboxPass->SetTexture(m_SkyBoxTexture);


    //Create a forward renderpass that draws directly to the screen.
    m_ForwardPass = m_Pipeline->AppendRenderPass<RenderPass_Forward>(RenderPassType::RP_FORWARD);
    m_ForwardPass->SetCamera(m_Camera);
    m_ForwardPass->SetTarget(window->GetRenderTarget());

    //Resize callback.
    window->SetResizeCallback([&](int w, int h)
    {
        //Update camera.
        CameraSettings camS;
        camS.width = w;
        camS.height = h;
        camS.fov = 90.f;
        camS.nearPlane = NEAR_PLANE;
        camS.farPlane = FAR_PLANE;
        m_Camera->UpdateSettings(camS);

    });

    //Create the GPU buffer used to put dynamic data in.
    GpuBufferSettings gpuBufferSettings;
    gpuBufferSettings.size = std::pow(2, 15);
    gpuBufferSettings.resizeWhenFull = true;
    gpuBufferSettings.memoryUsage = MemoryUsage::CPU_W;
    m_GpuBuffer = m_Engine.GetResourceManager().CreateGpuBuffer(gpuBufferSettings);

    //Set up shadow map generation for the render passes using the now existing data buffers.
    //Some of these datatypes are in shared_ptr format so that they can be modified during pipeline execution (offsets into buffers).
    //This is needed because shadow map generation generates data that is required on the GPU.
    m_DirLightMatView = GpuBufferView::MakeShared();
    m_DirLightDataOffsetView = GpuBufferView::MakeShared();
    ShadowData shadowData;
    shadowData.directional.shadowMaps = m_DirShadowArray;
    shadowData.directional.numCascades = NUM_CASCADES;
    shadowData.directional.dataBuffer = m_GpuBuffer;
    shadowData.directional.dataRange = m_DirLightMatView;
    shadowData.positional.shadowMaps = m_PosShadowArray;

    //Set information required for shadow map generation specifically.
    shadowData.directional.cascadeDistances = cascadeDistances;
    shadowData.directional.startOffset = m_DirLightDataOffsetView;

    //Pass the shadow data to the forward and shadow generation passes.
    m_ShadowGenerationPass->SetOutput(shadowData);
    m_ForwardPass->SetShadowData(shadowData);


    /*
     * GAMEPLAY OBJECTS
     *
     * Set up all entities in the scene and load meshes.
     */

    const int planetId = 0;
    const int sunId = 1;
    const int shipId = 2;
    const int killBotId = 3;
    const int moonId = 4;
    const int asteroidsId = 5;
    m_Meshes.emplace_back().Load("meshes/earth/", "scene.gltf", m_Engine.GetResourceManager());
    m_Meshes.emplace_back().Load("meshes/sun/", "scene.gltf", m_Engine.GetResourceManager());
    m_Meshes.emplace_back().Load("meshes/ship/", "scene.gltf", m_Engine.GetResourceManager());
    m_Meshes.emplace_back().Load("meshes/killbot/", "scene.gltf", m_Engine.GetResourceManager());
    m_Meshes.emplace_back().Load("meshes/moon/", "scene.gltf", m_Engine.GetResourceManager());
    m_Meshes.emplace_back().Load("meshes/asteroids/", "scene.gltf", m_Engine.GetResourceManager());


    //Add the planet at the origin.
    Planet* planet = static_cast<Planet*>(CreateEntity(EntityType::PLANET, planetId));
    planet->GetTransform().Scale(10.f);
    planet->SetRotationSpeed(-0.000005f);

    //Add the sun further out.
    Planet* sun = static_cast<Planet*>(CreateEntity(EntityType::PLANET, sunId));
    sun->GetTransform().Scale(30.f);
    sun->GetTransform().SetTranslation({0.f, 0.f, -1500.f});
    sun->SetRotationSpeed(-0.00000005f);


    //Make the moon rotate around the planet.
    Planet* moon = static_cast<Planet*>(CreateEntity(EntityType::PLANET, moonId));
    moon->GetTransform().Scale(10.f);
    moon->GetTransform().SetTranslation({ -150.f, 0.f, 0.f });
    moon->SetRotationSpeed(-0.00001f);
    moon->SetOrbit(0.00001f, {0.f, 0.f, 0.f}, {0.f, 1.f, 0.f});

    //Add asteroid belt.
    const int NUM_ASTEROIDS = 1000;
    const float MIN_ASTEROID_DISTANCE = 300.f;
    const float MAX_ASTEROID_DISTANCE = 350.f;
    const float MAX_ASTEROID_HEIGHT_OFFSET = 10.f;

    for (int i = 0; i < NUM_ASTEROIDS; ++i)
    {
        float xSpeed = RAND_FLOAT() * 0.00001f;
        float ySpeed = RAND_FLOAT() * 0.00001f;
        float zSpeed = RAND_FLOAT() * 0.00001f;    //Set a limiton the rotation speed.
        float scale = RAND_FLOAT() * 2.f + 0.2f;  //Min scale is 0.2 and max is 2.2.

        //Set up rotation and scale.
        Asteroid* asteroid = static_cast<Asteroid*>(CreateEntity(EntityType::ASTEROID, asteroidsId));
        asteroid->GetTransform().Scale(scale);
        asteroid->SetRotation({ 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, {0.f, 0.f, 1.f}, {xSpeed, ySpeed, zSpeed});

        //Set position.
        const float angle = 2.f * 3.141592f * RAND_FLOAT();
        const float distance = (RAND_FLOAT() * (MAX_ASTEROID_DISTANCE - MIN_ASTEROID_DISTANCE)) + MIN_ASTEROID_DISTANCE;
        const float x = cos(angle) * distance;
        const float z = sin(angle) * distance;
        const float y = (RAND_FLOAT() * 2.f * MAX_ASTEROID_HEIGHT_OFFSET) - MAX_ASTEROID_HEIGHT_OFFSET;

        asteroid->GetTransform().SetTranslation({ x, y, z });
    }
}

void Game::UpdateInput(std::shared_ptr<blurp::Window>& a_Window)
{
    using namespace blurp;

    //Read the input from the window.
    auto input = a_Window->PollInput();

    KeyboardEvent kEvent;
    MouseEvent mEvent;

    //If escape was pressed, exit.
    while (input.getNextEvent(kEvent))
    {
        if (kEvent.keyCode == KEY_ESCAPE)
        {
            a_Window->Close();
        }
    }

    float mouseMoveX = 0;
    float mouseMoveY = 0;
    float mouseScroll = 0;

    //Read mouse input to control the camera.
    while (input.getNextEvent(mEvent))
    {
        if (mEvent.action == MouseAction::SCROLL)
        {
            mouseScroll += -mEvent.value;
        }
        else if (mEvent.action == MouseAction::MOVE_X)
        {
            mouseMoveX += mEvent.value;
        }
        else if (mEvent.action == MouseAction::MOVE_Y)
        {
            mouseMoveY += mEvent.value;
        }
    }

    //Update the camera based on input.
    {
        auto& transform = m_Camera->GetTransform();
        bool shift = input.getKeyState(KEY_SHIFT) != ButtonState::NOT_PRESSED;

        const float movespeed = (shift ? 1.0f : 0.2f);
        const float rotationSpeed = (shift ? 0.05f : 0.01f);

        if (input.getKeyState(KEY_W) != ButtonState::NOT_PRESSED)
        {
            transform.Translate(transform.GetForward() * -movespeed);
        }
        if (input.getKeyState(KEY_A) != ButtonState::NOT_PRESSED)
        {
            transform.Translate(transform.GetLeft() * movespeed);
        }
        if (input.getKeyState(KEY_S) != ButtonState::NOT_PRESSED)
        {
            transform.Translate(transform.GetBack() * -movespeed);
        }
        if (input.getKeyState(KEY_D) != ButtonState::NOT_PRESSED)
        {
            transform.Translate(transform.GetRight() * movespeed);
        }

        if (input.getKeyState(KEY_UP) != ButtonState::NOT_PRESSED)
        {
            transform.Rotate(transform.GetRight(), rotationSpeed);
        }
        if (input.getKeyState(KEY_LEFT) != ButtonState::NOT_PRESSED)
        {
            transform.Rotate(transform.GetUp(), rotationSpeed);
        }
        if (input.getKeyState(KEY_DOWN) != ButtonState::NOT_PRESSED)
        {
            transform.Rotate(transform.GetRight(), -rotationSpeed);
        }
        if (input.getKeyState(KEY_RIGHT) != ButtonState::NOT_PRESSED)
        {
            transform.Rotate(transform.GetUp(), -rotationSpeed);
        }
        if (input.getKeyState(KEY_Q) != ButtonState::NOT_PRESSED)
        {
            transform.Rotate(transform.GetForward(), rotationSpeed);
        }
        if (input.getKeyState(KEY_E) != ButtonState::NOT_PRESSED)
        {
            transform.Rotate(transform.GetForward(), -rotationSpeed);
        }

        if (input.getKeyState(KEY_3) != ButtonState::NOT_PRESSED)
        {
            m_Sun->SetDirection(glm::normalize(m_Camera->GetTransform().GetBack()));
        }

        if (input.getKeyState(KEY_C) != ButtonState::NOT_PRESSED)
        {
            std::cout << "Cam pos: " << m_Camera->GetTransform().GetTranslation().x << " " << m_Camera->GetTransform().GetTranslation().y << " " << m_Camera->GetTransform().GetTranslation().z << std::endl;
        }
    }

    //Handle alt enter to go fullscreen.
    if (input.getKeyState(KEY_ALT) != ButtonState::NOT_PRESSED && input.getKeyState(KEY_ENTER) == ButtonState::FIRST_PRESSED)
    {
        a_Window->SetFullScreen(!a_Window->IsFullScreen());
    }
}

void Game::UpdateGame(float a_DeltaTime)
{
    /*
     * Remove dead entities. This frees them in their memory pool.
     */
    auto itr = m_Entities.begin();
    while(itr != m_Entities.end())
    {
        if (itr->first->MarkedForDelete())
        {
            itr->second->free(itr->first);
            itr = m_Entities.erase(itr);
        }else
        {
            ++itr;
        }
    }

    /*
     * Update every entity.
     */
    for(auto& entity : m_Entities)
    {
        entity.first->OnUpdate(a_DeltaTime, *this);
    }
}

void Game::Render()
{
    //Reset the passes.
    m_ForwardPass->Reset();
    m_ShadowGenerationPass->Reset();

    /*
     * An incrementing value indicating the offset into the GPU Buffer.
     */
    int gpuBufferOffset = 0;

    /*
     * Allocate memory to store the draw calls, and then iterate over the entities in the scene to find their transforms per mesh.
     */
    std::vector<blurp::DrawData> drawDatas;
    std::vector<blurp::DrawData> drawDatasTransparent;
    std::vector<std::vector<glm::mat4>> transforms;
    for(int i = 0; i < m_Meshes.size(); ++i)
    {
        transforms.emplace_back();
    }
    //TODO use a data structure like an octree to reduce this costly loop.
    for(auto& entity : m_Entities)
    {
        const int id = entity.first->GetMeshId();
        if(id != -1)
        {
            transforms[id].emplace_back(entity.first->GetTransform().GetTransformation());
        }
    }

    //TODO sort transforms from front to back. How does this work with transparency because it's the other way around. Upload once to GPU then read backwards? Maybe add a setting to the renderer to flip reading direction?

    //Upload transforms to the GPU and link them to the draw call.
    for(int i = 0; i < m_Meshes.size(); ++ i)
    {
        auto& matvec = transforms[i];
        if(!matvec.empty())
        {
            auto view = m_GpuBuffer->WriteData<glm::mat4>(gpuBufferOffset, matvec.size(), 16, &matvec[0]);
            gpuBufferOffset = view.end;

            //Opaque draw calls.
            for(auto& data : m_Meshes[i].GetDrawDatas())
            {
                auto& inserted = drawDatas.emplace_back(data);
                inserted.instanceCount = matvec.size();
                inserted.transformData.dataRange = view;
                inserted.transformData.dataBuffer = m_GpuBuffer;
            }

            //Transparent draw calls (happen last).
            for (auto& data : m_Meshes[i].GetTransparentDrawDatas())
            {
                auto& inserted = drawDatasTransparent.emplace_back(data);
                inserted.instanceCount = matvec.size();
                inserted.transformData.dataRange = view;
                inserted.transformData.dataBuffer = m_GpuBuffer;
            }
        }
    }

    //Append transparent draw calls to solid ones.
    drawDatas.insert(drawDatas.end(), drawDatasTransparent.begin(), drawDatasTransparent.end());

    //Tell the shadow pass to use all geometry as shadow casters.
    //This looks complex, but all it does is specify an array of lights indexes for every DrawData object.
    //When drawing that geometry, all lights specified will be considered for shadow generation.
    
    //Every light affects every geometry, so make a collection of each index.
    std::vector<int> dirLightShadowIndices;
    std::vector<int> posLightShadowIndices;

    //Setup the shadow mapping for this frame. Exclude the last Drawdata which is the light mesh.
    //TODO find the lights to use for shadows! Closest to camera probably.
    //for (int i = 0; i < m_Lights.size(); ++i)
    //{
    //    m_ShadowGenerationPass->AddLight(m_Lights[i], i);
    //    posLightShadowIndices.push_back(i);
    //}

    m_ShadowGenerationPass->AddLight(m_Sun, 0);
    dirLightShadowIndices.push_back(0);

    //TODO set up which objects cast shadows for each shadow casting light.
    //TODO This means building a new drawable set.
    //
    //Only take solid geometry for shadows for now.
    std::vector<blurp::LightIndexData> lIndexData;
    //lIndexData.reserve(numdrawdatastocastshadowsfor);

    //for (int i = 0; i < static_cast<int>(numdrawdatastocastshadowsfor); ++i)
    //{
    //    //0 and 0 indicates that each piece of geometry is affected by the dir light at index 0, and the pos light at index 0.
    //    lIndexData.emplace_back(blurp::LightIndexData{ dirLightShadowIndices, posLightShadowIndices });
    //}

    //TODO find lights to use (also non-shadow).
    //Upload light data
    blurp::LightData lData;
    lData.ambient = { 0.01f, 0.01f, 0.01f };
    blurp::LightUploadData lud;
    lud.lightData = &lData;
    //lud.point.count = 0;
    //lud.point.lights = &m_Lights[0];
    lud.directional.lights = &m_Sun;
    lud.directional.count = 1;
    auto lightView = m_GpuBuffer->WriteData(gpuBufferOffset, lud);
    gpuBufferOffset = lightView.end;

    //Set the view used to determine the offset into the buffer to write shadow map matrices.
    //This means that the shadow generation pass will now append all shadow map matrices behind the uploaded light data in the buffer.
    *m_DirLightDataOffsetView = lightView;

    m_ForwardPass->SetLights(lData);

    //Pass a reference to all the meshes to the forward render pass.
    blurp::DrawDataSet drawableSet;
    drawableSet.drawDataCount = drawDatas.size();
    drawableSet.drawDataPtr = &drawDatas[0];

    m_ForwardPass->SetDrawData(drawableSet);

    //Update the rendering pipeline.
    m_Pipeline->Execute();
}