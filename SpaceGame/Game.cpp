#include "Game.h"
#include <KeyCodes.h>
#include <BlurpEngine.h>
#include <RenderResourceManager.h>
#include <Window.h>
#include <RenderPipeline.h>
#include <GpuBuffer.h>

#include "CubeMapLoader.h"
#include "MeshLoader.h"

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
    camSettings.fov = 120.f;
    camSettings.nearPlane = 0.1f;
    camSettings.farPlane = 1000.f;
    m_Camera = m_Engine.GetResourceManager().CreateCamera(camSettings);

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
        camS.fov = 120.f;
        camS.nearPlane = 0.1f;
        camS.farPlane = 1000.f;
        m_Camera->UpdateSettings(camS);

    });

    //Create the GPU buffer used to put dynamic data in.
    GpuBufferSettings gpuBufferSettings;
    gpuBufferSettings.size = std::pow(2, 15);
    gpuBufferSettings.resizeWhenFull = true;
    gpuBufferSettings.memoryUsage = MemoryUsage::CPU_W;
    m_TransformBuffer = m_Engine.GetResourceManager().CreateGpuBuffer(gpuBufferSettings);

    //Light
    LightSettings lSettings;
    lSettings.type = LightType::LIGHT_POINT;
    lSettings.intensity = 2000.f;
    lSettings.color = { 1.f, 1.f, 1.f };
    lSettings.pointLight.position = { 0.f, 40.f, 10.f };
    m_Light = std::reinterpret_pointer_cast<PointLight>(m_Engine.GetResourceManager().CreateLight(lSettings));

    //Load GLTF mesh.
    m_Drawables = LoadMesh(MeshLoaderSettings{"meshes/Duck/Duck.gltf", 0, nullptr}, m_Engine.GetResourceManager());
    for(auto& drawable : m_Drawables)
    {
        drawable.instanceCount = 1;
        drawable.attributes.EnableAttribute(DrawAttribute::TRANSFORMATION_MATRIX);
    }

    m_ObjectTransform.Translate({ 0.f, 1.f, 1.f });
    m_ObjectTransform.Scale(0.1f);
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

        const float movespeed = (shift ? 0.1f : 0.02f);
        const float rotationSpeed = (shift ? 0.005f : 0.001f);

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
    //Upload transforms to the GPU and set every mesh to use the transform.
    glm::mat4 transform = m_ObjectTransform.GetTransformation();
    auto view = m_TransformBuffer->WriteData<glm::mat4>(0, 1, 16, &transform);
    for(auto& drawable : m_Drawables)
    {
        drawable.transformData.dataBuffer = m_TransformBuffer;
        drawable.transformData.dataRange = view;
    }

    //Update the forward pass data.
    m_ForwardPass->Reset();
    blurp::LightData lData;
    blurp::LightUploadData lud;
    lud.lightData = &lData;
    lud.point.count = 1;
    lud.point.lights = &m_Light;
    m_TransformBuffer->WriteData(view.end, lud);
    m_ForwardPass->SetLights(lData);

    //Pass a reference to all the meshes to the forward render pass.
    blurp::DrawDataSet drawableSet;
    drawableSet.drawDataCount = m_Drawables.size();
    drawableSet.drawDataPtr = &m_Drawables[0];

    m_ForwardPass->SetDrawData(drawableSet);

    //Update the rendering pipeline.
    m_Pipeline->Execute();

    while (true)
    {
        if (m_Pipeline->HasFinishedExecuting())
        {
            break;
        }
    }
}
