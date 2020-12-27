#include "Game.h"
#include <KeyCodes.h>
#include <BlurpEngine.h>
#include <RenderResourceManager.h>
#include <Window.h>
#include <RenderPipeline.h>
#include <GpuBuffer.h>

#include "CubeMapLoader.h"
#include "MeshLoader.h"
#include <iostream>

#define SHADOW_MAP_DIMENSION 2048
#define NUM_CASCADES 6
#define FAR_PLANE 1000.f
#define NEAR_PLANE 0.1f


Game::Game(blurp::BlurpEngine& a_RenderEngine) : m_Engine(a_RenderEngine)
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

    m_Camera->GetTransform().SetTranslation({-32.0f, 1.3f, 11.0f});

    //Create some lights and the sun.
    LightSettings lSettings;
    lSettings.type = LightType::LIGHT_POINT;
    lSettings.intensity = 5.f;
    lSettings.color = { 0.2f, 1.f, 0.7f };
    lSettings.pointLight.position = { 0.f, 40.f, 10.f };
    lSettings.shadowMapIndex = 0;
    m_Lights.push_back(std::reinterpret_pointer_cast<PointLight>(m_Engine.GetResourceManager().CreateLight(lSettings)));

    lSettings.pointLight.position = { -50.f, 40.f, 10 };
    lSettings.intensity = 5.f;
    lSettings.color = { 0.4f, 0.4f, 0.7f };
    lSettings.shadowMapIndex = 1;
    m_Lights.push_back(std::reinterpret_pointer_cast<PointLight>(m_Engine.GetResourceManager().CreateLight(lSettings)));

    lSettings.type = LightType::LIGHT_DIRECTIONAL;
    lSettings.shadowMapIndex = 0;
    lSettings.intensity = 1.7f;
    lSettings.color = { 1.f, 0.8f, 0.3f };
    lSettings.directionalLight.direction = glm::vec3(1.f, 0.f, 0.f);
    m_Sun = std::reinterpret_pointer_cast<DirectionalLight>(m_Engine.GetResourceManager().CreateLight(lSettings));

    //Create shadow map generation passes.
    //Generate shadowmaps before doing the forward rendering.
    m_ShadowGenerationPass = m_Pipeline->AppendRenderPass<RenderPass_ShadowMap>(RenderPassType::RP_SHADOWMAP);
    m_ShadowGenerationPass->SetCamera(m_Camera);

    //Positional shadow array.
    TextureSettings shadowPosSettings;
    shadowPosSettings.dimensions = glm::vec3(SHADOW_MAP_DIMENSION, SHADOW_MAP_DIMENSION, m_Lights.size() * 6);
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
    posShadowClear.size = glm::vec3(SHADOW_MAP_DIMENSION, SHADOW_MAP_DIMENSION, m_Lights.size() * 6);
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
    m_TransformBuffer = m_Engine.GetResourceManager().CreateGpuBuffer(gpuBufferSettings);

    //Load GLTF mesh.
    //m_Scene = LoadMesh(MeshLoaderSettings{"MetalRoughSpheres.gltf", "meshes/spheres/", 0, nullptr}, m_Engine.GetResourceManager(), true);
    m_Scene = LoadMesh(MeshLoaderSettings{"scene.gltf", "meshes/town/", 0, nullptr}, m_Engine.GetResourceManager(), true);
    //m_Scene = LoadMesh(MeshLoaderSettings{ "scene.gltf", "meshes/forest/", 0, nullptr }, m_Engine.GetResourceManager(), true);
    //m_Scene = LoadMesh(MeshLoaderSettings{ "DamagedHelmet.gltf", "meshes/Helmet/", 0, nullptr }, m_Engine.GetResourceManager(), true);

    //Upload matrices for each object, appending to the end of the buffer.
    //Remember the end of the buffer as a global variable so that I don't accidentally overwrite it.
    m_TransformEnd = 0;
    for(auto& mesh : m_Scene.meshes)
    {
        auto view = m_TransformBuffer->WriteData<glm::mat4>(m_TransformEnd, mesh.transforms.size(), 16, &mesh.transforms[0]);
        m_TransformEnd = view.end;
        for(auto drawableId : mesh.drawableIds)
        {
            auto& drawable = m_Scene.drawDatas[drawableId];
            drawable.transformData.dataBuffer = m_TransformBuffer;
            drawable.transformData.dataRange = view;
        }

        //Transparency.
        for (auto drawableId : mesh.transparentDrawableIds)
        {
            auto& drawable = m_Scene.transparentDrawDatas[drawableId];
            drawable.transformData.dataBuffer = m_TransformBuffer;
            drawable.transformData.dataRange = view;
        }
    }

    //Append all draw data (first solid then transparent). Remember where transparency starts for eventual sorting.
    drawDatas.reserve(m_Scene.drawDatas.size() + m_Scene.transparentDrawDatas.size());
    drawDatas.insert(drawDatas.end(), m_Scene.drawDatas.begin(), m_Scene.drawDatas.end());
    m_TransparentStart = drawDatas.size();
    drawDatas.insert(drawDatas.end(), m_Scene.transparentDrawDatas.begin(), m_Scene.transparentDrawDatas.end());

    //Set up shadow map generation for the render passes using the now existing data buffers.
    //Some of these datatypes are in shared_ptr format so that they can be modified during pipeline execution (offsets into buffers).
    //This is needed because shadow map generation generates data that is required on the GPU.
    m_DirLightMatView = GpuBufferView::MakeShared();
    m_DirLightDataOffsetView = GpuBufferView::MakeShared();
    ShadowData shadowData;
    shadowData.directional.shadowMaps = m_DirShadowArray;
    shadowData.directional.numCascades = NUM_CASCADES;
    shadowData.directional.dataBuffer = m_TransformBuffer;
    shadowData.directional.dataRange = m_DirLightMatView;
    shadowData.positional.shadowMaps = m_PosShadowArray;

    //Set information required for shadow map generation specifically.
    shadowData.directional.cascadeDistances = cascadeDistances;
    shadowData.directional.startOffset = m_DirLightDataOffsetView;

    //Pass the shadow data to the forward and shadow generation passes.
    m_ShadowGenerationPass->SetOutput(shadowData);
    m_ForwardPass->SetShadowData(shadowData);
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


        if (input.getKeyState(KEY_1) != ButtonState::NOT_PRESSED)
        {
            m_Lights[0]->SetPosition(m_Camera->GetTransform().GetTranslation());
        }
        if (input.getKeyState(KEY_2) != ButtonState::NOT_PRESSED)
        {
            m_Lights[1]->SetPosition(m_Camera->GetTransform().GetTranslation());
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

void Game::UpdateGame()
{
}

void Game::Render()
{
    //Reset the passes.
    m_ForwardPass->Reset();
    m_ShadowGenerationPass->Reset();

    //Tell the shadow pass to use all geometry as shadow casters.
    //This looks complex, but all it does is specify an array of lights indexes for every DrawData object.
    //When drawing that geometry, all lights specified will be considered for shadow generation.
    
    //Every light affects every geometry, so make a collection of each index.
    std::vector<int> dirLightShadowIndices;
    std::vector<int> posLightShadowIndices;

    //Setup the shadow mapping for this frame. Exclude the last Drawdata which is the light mesh.
    for (int i = 0; i < m_Lights.size(); ++i)
    {
        m_ShadowGenerationPass->AddLight(m_Lights[i], i);
        posLightShadowIndices.push_back(i);
    }

    m_ShadowGenerationPass->AddLight(m_Sun, 0);
    dirLightShadowIndices.push_back(0);

    //Only take solid geometry for shadows for now.
    std::vector<blurp::LightIndexData> lIndexData;
    lIndexData.reserve(m_TransparentStart);

    for (int i = 0; i < static_cast<int>(m_TransparentStart); ++i)
    {
        //0 and 0 indicates that each piece of geometry is affected by the dir light at index 0, and the pos light at index 0.
        lIndexData.emplace_back(blurp::LightIndexData{ dirLightShadowIndices, posLightShadowIndices });
    }

    m_ShadowGenerationPass->SetGeometry(&drawDatas[0], &lIndexData[0], lIndexData.size());


    //Upload light data
    blurp::LightData lData;
    lData.ambient = { 0.03f, 0.03f, 0.03f };
    blurp::LightUploadData lud;
    lud.lightData = &lData;
    lud.point.count = m_Lights.size();
    lud.point.lights = &m_Lights[0];
    lud.directional.lights = &m_Sun;
    lud.directional.count = 1;
    auto lightView = m_TransformBuffer->WriteData(m_TransformEnd, lud);

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
