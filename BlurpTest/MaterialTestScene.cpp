#include "MaterialTestScene.h"
#include <BlurpEngine.h>
#include <RenderPipeline.h>
#include <Camera.h>
#include <KeyCodes.h>
#include <Window.h>
#include <RenderPass_Forward.h>
#include <RenderResourceManager.h>
#include <iostream>

#include "MaterialLoader.h"


using namespace blurp;

const static std::float_t CUBE_DATA[]
{
//      X    Y     Z            NORMAL              UV              Color             
    	-0.5f, -0.5f, 0.5f,     0.f, 0.f, 1.f,      0.f, 1.f,       1.f, 1.f, 1.f,
        -0.5f, 0.5f, 0.5f,      0.f, 0.f, 1.f,      0.f, 0.f,       1.f, 1.f, 1.f,
        0.5f, -0.5f, 0.5f,      0.f, 0.f, 1.f,      1.f, 1.f,       1.f, 1.f, 1.f,
        0.5f, 0.5f, 0.5f,       0.f, 0.f, 1.f,      1.f, 0.f,       1.f, 1.f, 1.f,

    	0.5f, -0.5f, -0.5f,     1.f, 0.f, 0.f,      1.f, 1.f,       1.f, 1.f, 1.f,
        0.5f, -0.5f, 0.5f,      1.f, 0.f, 0.f,      0.f, 1.f,       1.f, 1.f, 1.f,
        0.5f, 0.5f, -0.5f,      1.f, 0.f, 0.f,      1.f, 0.f,       1.f, 1.f, 1.f,
        0.5f, 0.5f, 0.5f,       1.f, 0.f, 0.f,      0.f, 0.f,       1.f, 1.f, 1.f,

    	-0.5f, -0.5f, -0.5f,    0.f, 0.f, -1.f,     1.f, 1.f,       1.f, 1.f, 1.f,
        -0.5f, 0.5f, -0.5f,     0.f, 0.f, -1.f,     1.f, 0.f,       1.f, 1.f, 1.f,
        0.5f, -0.5f, -0.5f,     0.f, 0.f, -1.f,     0.f, 1.f,       1.f, 1.f, 1.f,
        0.5f, 0.5f, -0.5f,      0.f, 0.f, -1.f,     0.f, 0.f,       1.f, 1.f, 1.f,

    	-0.5f, -0.5f, -0.5f,    -1.f, 0.f, 0.f,     0.f, 1.f,       1.f, 1.f, 1.f,
        -0.5f, -0.5f, 0.5f,     -1.f, 0.f, 0.f,     1.f, 1.f,       1.f, 1.f, 1.f,
        -0.5f, 0.5f, -0.5f,     -1.f, 0.f, 0.f,     0.f, 0.f,       1.f, 1.f, 1.f,
        -0.5f, 0.5f, 0.5f,      -1.f, 0.f, 0.f,     1.f, 0.f,       1.f, 1.f, 1.f,

    	-0.5f, 0.5f, -0.5f,     0.f, 1.f, 0.f,      1.f, 1.f,       1.f, 1.f, 1.f,
        -0.5f, 0.5f, 0.5f,      0.f, 1.f, 0.f,      1.f, 0.f,       1.f, 1.f, 1.f,
        0.5f, 0.5f, -0.5f,      0.f, 1.f, 0.f,      0.f, 1.f,       1.f, 1.f, 1.f,
        0.5f, 0.5f, 0.5f,       0.f, 1.f, 0.f,      0.f, 0.f,       1.f, 1.f, 1.f,

    	-0.5f, -0.5f, -0.5f,    0.f, -1.f, 0.f,     1.f, 1.f,       1.f, 1.f, 1.f,
        -0.5f, -0.5f, 0.5f,     0.f, -1.f, 0.f,     1.f, 0.f,       1.f, 1.f, 1.f,
        0.5f, -0.5f, -0.5f,     0.f, -1.f, 0.f,     0.f, 1.f,       1.f, 1.f, 1.f,
        0.5f, -0.5f, 0.5f,      0.f, -1.f, 0.f,     0.f, 0.f,       1.f, 1.f, 1.f,
};

const static std::uint16_t CUBE_INDICES[]
{
    0, 3, 1, 0, 2, 3,	    //SOUTH
    4, 7, 5, 4, 6, 7,	    //EAST
    8, 11, 10, 8, 9, 11,	//NORTH
    12, 15, 14, 12, 13, 15,	//WEST
    16, 19, 18, 16, 17, 19,	//UP
    20, 23, 21, 20, 22, 23,	//DOWN
};

void MaterialTestScene::Init()
{
    //Create the pipeline object.
    PipelineSettings pSettings;
    pSettings.waitForGpu = true;
    m_Pipeline = m_Engine.GetResourceManager().CreatePipeline(pSettings);

    //Set the clear color.
    m_Window->GetRenderTarget()->SetClearColor({ 0.1f, 0.1f, 0.45f, 1.f });

    //Create a camera to use.
    CameraSettings camSettings;
    camSettings.width = m_Window->GetDimensions().x;
    camSettings.height = m_Window->GetDimensions().y;
    camSettings.farPlane = 100000000000000.f;
    m_Camera = m_Engine.GetResourceManager().CreateCamera(camSettings);

    //Create a forward renderpass that draws directly to the screen.
    m_ForwardPass = m_Pipeline->AppendRenderPass<RenderPass_Forward>(RenderPassType::RP_FORWARD);
    m_ForwardPass->SetCamera(m_Camera);
    m_ForwardPass->SetTarget(m_Window->GetRenderTarget());

    //Create the mesh that the material is used with.
    MeshSettings meshSettings;
    meshSettings.indexData = &CUBE_INDICES;
    meshSettings.vertexData = &CUBE_DATA;
    meshSettings.indexDataType = DataType::USHORT;
    meshSettings.usage = MemoryUsage::GPU;
    meshSettings.access = AccessMode::READ_ONLY;
    meshSettings.vertexDataSizeBytes = sizeof(CUBE_DATA);
    meshSettings.numIndices = sizeof(CUBE_INDICES) / sizeof(CUBE_INDICES[0]);

    //Enabled attributes for the mesh.
    meshSettings.vertexSettings.EnableAttribute(VertexAttribute::POSITION_3D, 0, 44, 0);
    meshSettings.vertexSettings.EnableAttribute(VertexAttribute::NORMAL, 12, 44, 0);
    meshSettings.vertexSettings.EnableAttribute(VertexAttribute::UV_COORDS, 24, 44, 0);
    meshSettings.vertexSettings.EnableAttribute(VertexAttribute::COLOR, 32, 44, 0);

    //Load the mesh.
    m_Mesh = m_Engine.GetResourceManager().CreateMesh(meshSettings);

    //Create the GPU buffer containing the transform for the mesh.
    GpuBufferSettings gpuBufferSettings;
    gpuBufferSettings.size = std::pow(2, 15);
    gpuBufferSettings.resizeWhenFull = true;
    gpuBufferSettings.memoryUsage = MemoryUsage::CPU_W;
    m_GpuBuffer = m_Engine.GetResourceManager().CreateGpuBuffer(gpuBufferSettings);

    //Let the forward pass read data from this GPU buffer.
    m_ForwardPass->SetGpuBuffer(m_GpuBuffer);


    /*
     * Set up the material from the given paths.
     */
    MaterialData materialData;
    materialData.path = "materials/eggs/";
    materialData.diffuseTextureName = "diffuse.jpg";
    m_Material = LoadMaterial(m_Engine.GetResourceManager(), materialData);

    //Set up the object containing info about how to draw the mesh.
    m_QueueData.mesh = m_Mesh;
    m_QueueData.count = 1;
    m_QueueData.material = m_Material;

    //Set the camera away from the mesh and looking at it.
    m_Camera->GetTransform().SetTranslation(m_MeshTransform.GetTranslation() - (m_Camera->GetTransform().GetBack() * 20.f));
}

void MaterialTestScene::Update()
{
    //Rotate the mesh a bit each frame.
    const static float ROTATION_SPEED = 0.0005;
    m_MeshTransform.Rotate(m_MeshTransform.GetUp(), ROTATION_SPEED);

    //Read the input from the window.
    auto input = m_Window->PollInput();

    KeyboardEvent kEvent;
    MouseEvent mEvent;

    //If escape was pressed, exit.
    while (input.getNextEvent(kEvent))
    {
        if (kEvent.keyCode == KEY_ESCAPE)
        {
            m_Window->Close();
        }
    }


    //TODO: Listen for keyboard input to toggle different parts of the material on or off.


    const static float MOVE_SENSITIVITY = 0.001f;
    const static float SCROLL_SENSITIVITY = 1.5f;
    float mouseMoveX = 0;
    float mouseMoveY = 0;
    float mouseScroll = 0;

    //Read mouse input to control the camera.
    while (input.getNextEvent(mEvent))
    {
        if(mEvent.action == MouseAction::SCROLL)
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

    //Update the camera position based on the mouse input.
    if(mouseMoveX != 0)
    {
        m_Camera->GetTransform().RotateAround(m_MeshTransform.GetTranslation(), m_Camera->GetTransform().GetUp(), MOVE_SENSITIVITY * mouseMoveX);
    }
    if(mouseMoveY != 0)
    {
        m_Camera->GetTransform().RotateAround(m_MeshTransform.GetTranslation(), m_Camera->GetTransform().GetRight(), MOVE_SENSITIVITY * mouseMoveY);
    }
    //Handle zooming in and out.
    if(mouseScroll != 0)
    {
        const static float MIN_DISTANCE = 1.0f;
        auto moved = m_Camera->GetTransform().GetForward() * SCROLL_SENSITIVITY * mouseScroll;

        //Zooming out is always possible.
        if(mouseScroll > 0)
        {
            m_Camera->GetTransform().Translate(moved);
        }
        //Make sure not zooming in too close.
        else
        {
            const auto oldDistance = glm::distance(m_Camera->GetTransform().GetTranslation(), m_MeshTransform.GetTranslation());
            const auto moveLength = glm::length(moved);

            //Zoomed in too close. Set to correct distance.
            if(oldDistance - moveLength < MIN_DISTANCE)
            {
                m_Camera->GetTransform().LookAt(m_MeshTransform.GetTranslation() + (m_Camera->GetTransform().GetForward() * MIN_DISTANCE), m_MeshTransform.GetTranslation() + (m_Camera->GetTransform().GetForward() * MIN_DISTANCE * 2.f), m_Camera->GetTransform().GetUp());
            }
            //Zoom allowed.
            else
            {
                m_Camera->GetTransform().Translate(moved);
            }
        }
    }

    //Handle alt enter to go fullscreen.
    if (input.getKeyState(KEY_ALT) != ButtonState::NOT_PRESSED && input.getKeyState(KEY_ENTER) == ButtonState::FIRST_PRESSED)
    {
        m_Window->SetFullScreen(!m_Window->IsFullScreen());
    }

    /*
     * Upload the updated matrix data to the GPU.
     */

     //Reset old data.
    m_ForwardPass->Reset();

    //Calculate the mesh's MVP and upload it to the GPU buffer.
    const auto pv = m_Camera->GetProjectionMatrix() * m_Camera->GetViewMatrix();
    const glm::mat4 meshMVP = pv * m_MeshTransform.GetTransformation();
    m_QueueData.dataRange = m_GpuBuffer->WriteData<glm::mat4>(static_cast<void*>(0), 1, 16, &meshMVP);

    //Queue for draw.
    m_ForwardPass->QueueForDraw(m_QueueData);

    //Update the rendering pipeline.
    m_Pipeline->Execute();

    while (true)
    {
        if (m_Pipeline->HasFinishedExecuting())
        {
            break;
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(3));
}
