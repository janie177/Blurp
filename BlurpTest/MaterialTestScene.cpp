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
#include "Sphere.h"


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


    glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
    glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
    glm::vec3 pos3(1.0f, -1.0f, 0.0f);
    glm::vec3 pos4(1.0f, 1.0f, 0.0f);
    // texture coordinates
    glm::vec2 uv1(0.0f, 1.0f);
    glm::vec2 uv2(0.0f, 0.0f);
    glm::vec2 uv3(1.0f, 0.0f);
    glm::vec2 uv4(1.0f, 1.0f);
    // normal vector
    glm::vec3 nm(0.0f, 0.0f, 1.0f);

    // calculate tangent/bitangent vectors of both triangles
    glm::vec3 tangent1, bitangent1;
    glm::vec3 tangent2, bitangent2;
    // triangle 1
    // ----------
    glm::vec3 edge1 = pos2 - pos1;
    glm::vec3 edge2 = pos3 - pos1;
    glm::vec2 deltaUV1 = uv2 - uv1;
    glm::vec2 deltaUV2 = uv3 - uv1;

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
    tangent1 = glm::normalize(tangent1);

    bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
    bitangent1 = glm::normalize(bitangent1);

    // triangle 2
    // ----------
    edge1 = pos3 - pos1;
    edge2 = pos4 - pos1;
    deltaUV1 = uv3 - uv1;
    deltaUV2 = uv4 - uv1;

    f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
    tangent2 = glm::normalize(tangent2);


    bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
    bitangent2 = glm::normalize(bitangent2);


    float quadVertices[] = {
        // positions            // normal         // texcoords  // tangent                          // bitangent
        pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
        pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
        pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

        pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
        pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
        pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
    };

    std::uint16_t quadIndices[]
    {
        0,1,2,3,4,5
    };

    //Create the mesh that the material is used with.
    MeshSettings meshSettings;
    meshSettings.indexData = &quadIndices;
    meshSettings.vertexData = &quadVertices;
    meshSettings.indexDataType = DataType::USHORT;
    meshSettings.usage = MemoryUsage::GPU;
    meshSettings.access = AccessMode::READ_ONLY;
    meshSettings.vertexDataSizeBytes = sizeof(quadVertices);
    meshSettings.numIndices = 6;

    //Enabled attributes for the mesh.
    meshSettings.vertexSettings.EnableAttribute(VertexAttribute::POSITION_3D, 0, 56, 0);
    meshSettings.vertexSettings.EnableAttribute(VertexAttribute::NORMAL, 12, 56, 0);
    meshSettings.vertexSettings.EnableAttribute(VertexAttribute::UV_COORDS, 24, 56, 0);
    meshSettings.vertexSettings.EnableAttribute(VertexAttribute::TANGENT, 32, 56, 0);
    meshSettings.vertexSettings.EnableAttribute(VertexAttribute::BI_TANGENT, 44, 56, 0);


    m_Mesh = m_Engine.GetResourceManager().CreateMesh(meshSettings);

    //Load the mesh.
    //m_Mesh = Sphere::Load(m_Engine, 2.f, 64, 64);

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
    materialData.normalTextureName = "normal.jpg";
    materialData.metallicTextureName = "metallic.jpg";
    materialData.roughnessTextureName = "roughness.jpg";
    materialData.aoTextureName = "ao.jpg";
    //materialData.emissiveTextureName = "emissive.jpg";
    materialData.heightTextureName = "height.jpg";
    m_Material = LoadMaterial(m_Engine.GetResourceManager(), materialData);

    //Set up the object containing info about how to draw the mesh.
    m_QueueData.mesh = m_Mesh;
    m_QueueData.count = 1;
    m_QueueData.materialData.material = m_Material;

    //Enable transform uploading. Now a single matrix is expected in the GpuBufferView.
    m_QueueData.data.transform = true;


    m_MeshTransform.Scale({ 3.0, 3.0, 1.0 });
    m_MeshTransform.Translate({0, 10, 0});

    //Set the camera away from the mesh and looking at it.
    m_Camera->GetTransform().SetTranslation(m_MeshTransform.GetTranslation() - (m_Camera->GetTransform().GetBack() * 20.f));

    //Visualize the light as a white ball.
    auto lightMesh = Sphere::Load(m_Engine, 1.f, 64, 64);
    MaterialSettings lightMat;
    lightMat.EnableAttribute(MaterialAttribute::EMISSIVE_CONSTANT_VALUE);
    lightMat.SetEmissiveConstant({ 1, 1, 1 });
    auto lightMaterial = m_Engine.GetResourceManager().CreateMaterial(lightMat);


    m_LightQueueData.mesh = lightMesh;
    m_LightQueueData.count = 1;
    m_LightQueueData.materialData.material = lightMaterial;
    m_LightQueueData.data.transform = true;
}

void MaterialTestScene::Update()
{
    //Rotate the mesh a bit each frame.
    const static float ROTATION_SPEED = 0.005;
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

    MaterialAttribute attribute = static_cast<MaterialAttribute>(0);

    //Buttons pressed:
    if (input.getKeyState(KEY_0) == ButtonState::FIRST_PRESSED)
    {
        attribute = MaterialAttribute::DIFFUSE_TEXTURE;
    }
    if (input.getKeyState(KEY_1) == ButtonState::FIRST_PRESSED)
    {
        attribute = MaterialAttribute::NORMAL_TEXTURE;
    }
    if (input.getKeyState(KEY_2) == ButtonState::FIRST_PRESSED)
    {
        attribute = MaterialAttribute::EMISSIVE_TEXTURE;
    }
    if (input.getKeyState(KEY_3) == ButtonState::FIRST_PRESSED)
    {
        attribute = MaterialAttribute::METALLIC_TEXTURE;
    }
    if (input.getKeyState(KEY_4) == ButtonState::FIRST_PRESSED)
    {
        attribute = MaterialAttribute::ROUGHNESS_TEXTURE;
    }
    if (input.getKeyState(KEY_5) == ButtonState::FIRST_PRESSED)
    {
        attribute = MaterialAttribute::ALPHA_TEXTURE;
    }
    if (input.getKeyState(KEY_6) == ButtonState::FIRST_PRESSED)
    {
        attribute = MaterialAttribute::OCCLUSION_TEXTURE;
    }
    if (input.getKeyState(KEY_7) == ButtonState::FIRST_PRESSED)
    {
        attribute = MaterialAttribute::HEIGHT_TEXTURE;
    }
    if (input.getKeyState(KEY_8) == ButtonState::FIRST_PRESSED)
    {
        attribute = MaterialAttribute::DIFFUSE_CONSTANT_VALUE;
    }
    if (input.getKeyState(KEY_9) == ButtonState::FIRST_PRESSED)
    {
        attribute = MaterialAttribute::EMISSIVE_CONSTANT_VALUE;
    }
    if (input.getKeyState(KEY_Q) == ButtonState::FIRST_PRESSED)
    {
        attribute = MaterialAttribute::METALLIC_CONSTANT_VALUE;
    }
    if (input.getKeyState(KEY_W) == ButtonState::FIRST_PRESSED)
    {
        attribute = MaterialAttribute::ROUGHNESS_CONSTANT_VALUE;
    }
    if (input.getKeyState(KEY_E) == ButtonState::FIRST_PRESSED)
    {
        attribute = MaterialAttribute::ALPHA_CONSTANT_VALUE;
    }

    //If a switch was made, toggle it.
    if (static_cast<std::uint32_t>(attribute) != 0)
    {
        if (m_Material->GetSettings().IsAttributeEnabled(attribute))
        {
            m_Material->GetSettings().DisableAttribute(attribute);
        }
        else
        {
            m_Material->GetSettings().EnableAttribute(attribute);
        }
    }


    const static float MOVE_SENSITIVITY = 0.001f;
    const static float SCROLL_SENSITIVITY = 1.5f;
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

    //Update the camera position based on the mouse input.
    if (mouseMoveX != 0)
    {
        m_Camera->GetTransform().RotateAround(m_MeshTransform.GetTranslation(), m_Camera->GetTransform().GetUp(), MOVE_SENSITIVITY * mouseMoveX);
    }
    if (mouseMoveY != 0)
    {
        m_Camera->GetTransform().RotateAround(m_MeshTransform.GetTranslation(), m_Camera->GetTransform().GetRight(), MOVE_SENSITIVITY * mouseMoveY);
    }
    //Handle zooming in and out.
    if (mouseScroll != 0)
    {
        const static float MIN_DISTANCE = 1.0f;
        auto moved = m_Camera->GetTransform().GetForward() * SCROLL_SENSITIVITY * mouseScroll;

        //Zooming out is always possible.
        if (mouseScroll > 0)
        {
            m_Camera->GetTransform().Translate(moved);
        }
        //Make sure not zooming in too close.
        else
        {
            const auto oldDistance = glm::distance(m_Camera->GetTransform().GetTranslation(), m_MeshTransform.GetTranslation());
            const auto moveLength = glm::length(moved);

            //Zoomed in too close. Set to correct distance.
            if (oldDistance - moveLength < MIN_DISTANCE)
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
    auto matrix = m_MeshTransform.GetTransformation();;
    m_QueueData.data.dataRange = m_GpuBuffer->WriteData<glm::mat4>(static_cast<void*>(0), 1, 16, &matrix);

    //Upload light transform.
    Transform lightTransform;
    lightTransform.Translate({ -5, 10, -5 });
    auto lightMat = lightTransform.GetTransformation();
    m_LightQueueData.data.dataRange = m_GpuBuffer->WriteData<glm::mat4>(reinterpret_cast<void*>(m_QueueData.data.dataRange.end), 1, 16, &lightMat);

    //Queue for draw.
    m_ForwardPass->QueueForDraw(m_QueueData);
    m_ForwardPass->QueueForDraw(m_LightQueueData);

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