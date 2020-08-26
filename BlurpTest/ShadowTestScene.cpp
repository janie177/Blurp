#include "ShadowTestScene.h"

#include <BlurpEngine.h>
#include <KeyCodes.h>
#include <RenderResourceManager.h>
#include <Texture.h>

#include <RenderPass_Clear.h>
#include "GpuBuffer.h"
#include "MaterialLoader.h"
#include "Sphere.h"

#include "ImageUtil.h"

#define MESH_SCALE 500.f

#define NUM_SHADOW_POS_LIGHTS 1

#define NEAR_PLANE 1.f
#define FAR_PLANE 100.f

const static std::float_t CUBE_DATA[]
{
    //      X    Y     Z            NORMAL              UV                
            -0.5f, -0.5f, 0.5f,     0.f, 0.f, 1.f,      0.f, MESH_SCALE,
            -0.5f, 0.5f, 0.5f,      0.f, 0.f, 1.f,      0.f, 0.f,
            0.5f, -0.5f, 0.5f,      0.f, 0.f, 1.f,      MESH_SCALE, MESH_SCALE,
            0.5f, 0.5f, 0.5f,       0.f, 0.f, 1.f,      MESH_SCALE, 0.f,

            0.5f, -0.5f, -0.5f,     1.f, 0.f, 0.f,      MESH_SCALE, MESH_SCALE,
            0.5f, -0.5f, 0.5f,      1.f, 0.f, 0.f,      0.f, MESH_SCALE,
            0.5f, 0.5f, -0.5f,      1.f, 0.f, 0.f,      MESH_SCALE, 0.f,
            0.5f, 0.5f, 0.5f,       1.f, 0.f, 0.f,      0.f, 0.f,

            -0.5f, -0.5f, -0.5f,    0.f, 0.f, -1.f,     MESH_SCALE, MESH_SCALE,
            -0.5f, 0.5f, -0.5f,     0.f, 0.f, -1.f,     MESH_SCALE, 0.f,
            0.5f, -0.5f, -0.5f,     0.f, 0.f, -1.f,     0.f, MESH_SCALE,
            0.5f, 0.5f, -0.5f,      0.f, 0.f, -1.f,     0.f, 0.f,

            -0.5f, -0.5f, -0.5f,    -1.f, 0.f, 0.f,     0.f, MESH_SCALE,
            -0.5f, -0.5f, 0.5f,     -1.f, 0.f, 0.f,     MESH_SCALE, MESH_SCALE,
            -0.5f, 0.5f, -0.5f,     -1.f, 0.f, 0.f,     0.f, 0.f,
            -0.5f, 0.5f, 0.5f,      -1.f, 0.f, 0.f,     MESH_SCALE, 0.f,

            -0.5f, 0.5f, -0.5f,     0.f, 1.f, 0.f,      MESH_SCALE, MESH_SCALE,
            -0.5f, 0.5f, 0.5f,      0.f, 1.f, 0.f,      MESH_SCALE, 0.f,
            0.5f, 0.5f, -0.5f,      0.f, 1.f, 0.f,      0.f, MESH_SCALE,
            0.5f, 0.5f, 0.5f,       0.f, 1.f, 0.f,      0.f, 0.f,

            -0.5f, -0.5f, -0.5f,    0.f, -1.f, 0.f,     MESH_SCALE, MESH_SCALE,
            -0.5f, -0.5f, 0.5f,     0.f, -1.f, 0.f,     MESH_SCALE, 0.f,
            0.5f, -0.5f, -0.5f,     0.f, -1.f, 0.f,     0.f, MESH_SCALE,
            0.5f, -0.5f, 0.5f,      0.f, -1.f, 0.f,     0.f, 0.f,
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

void ShadowTestScene::Init()
{
    using namespace blurp;

    //Create the pipeline object.
    PipelineSettings pSettings;
    pSettings.waitForGpu = true;
    m_Pipeline = m_Engine.GetResourceManager().CreatePipeline(pSettings);

    //Add a clear pass first.
    m_ClearPass = m_Pipeline->AppendRenderPass<RenderPass_Clear>(RenderPassType::RP_CLEAR);

    //Set the clear color.
    auto renderTarget = m_Window->GetRenderTarget();
    renderTarget->SetClearColor({ 0.f, 0.1f, 0.1f, 1.f });

    //Mark the render target for clearing at the start of each pass.
    m_ClearPass->AddRenderTarget(renderTarget);

    //Create a camera to use.
    CameraSettings camSettings;
    camSettings.width = m_Window->GetDimensions().x;
    camSettings.height = m_Window->GetDimensions().y;
    camSettings.fov = 120.f;
    camSettings.nearPlane = NEAR_PLANE;
    camSettings.farPlane = FAR_PLANE;
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
    m_SkyboxPass->SetTarget(m_Window->GetRenderTarget());
    m_SkyboxPass->SetTexture(m_SkyBoxTexture);

    //Generate shadowmaps before doing the forward rendering.
    m_ShadowGenerationPass = m_Pipeline->AppendRenderPass<RenderPass_ShadowMap>(RenderPassType::RP_SHADOWMAP);
    m_ShadowGenerationPass->SetCamera(m_Camera);


    TextureSettings shadowPosSettings;
    shadowPosSettings.dimensions = glm::vec3(1024.f, 1024.f, NUM_SHADOW_POS_LIGHTS * 6);
    shadowPosSettings.generateMipMaps = false;
    shadowPosSettings.dataType = DataType::FLOAT;
    shadowPosSettings.pixelFormat = PixelFormat::DEPTH;
    shadowPosSettings.memoryAccess = AccessMode::READ_WRITE;
    shadowPosSettings.memoryUsage = MemoryUsage::GPU;
    shadowPosSettings.textureType = TextureType::TEXTURE_CUBEMAP_ARRAY;
    m_PosShadowArray = m_Engine.GetResourceManager().CreateTexture(shadowPosSettings);

    //Clear the shadow textures every frame.
    ClearData posShadowClear;
    posShadowClear.size = glm::vec3(1024, 1024, NUM_SHADOW_POS_LIGHTS * 6);
    posShadowClear.clearValue.floats[0] = 1.f;
    m_ClearPass->AddTexture(m_PosShadowArray, posShadowClear);

    //Add the shadow data to the shadow pass.
    m_ShadowGenerationPass->SetOutputPositional(m_PosShadowArray);

    //Create a forward renderpass that draws directly to the screen.
    m_ForwardPass = m_Pipeline->AppendRenderPass<RenderPass_Forward>(RenderPassType::RP_FORWARD);
    m_ForwardPass->SetCamera(m_Camera);
    m_ForwardPass->SetTarget(m_Window->GetRenderTarget());

    //Use shadow mapping
    m_ForwardPass->SetPointSpotShadowMaps(m_PosShadowArray);

    //Resize callback.
    m_Window->SetResizeCallback([&](int w, int h)
    {
        //Update camera.
        CameraSettings camS;
        camS.width = w;
        camS.height = h;
        camS.fov = 120.f;
        camS.nearPlane = 0.1f;
        camS.farPlane = FAR_PLANE;
        m_Camera->SetProjection(camS);
    });

    //Create the GPU buffer containing the transform for the mesh.
    GpuBufferSettings gpuBufferSettings;
    gpuBufferSettings.size = std::pow(2, 15);
    gpuBufferSettings.resizeWhenFull = true;
    gpuBufferSettings.memoryUsage = MemoryUsage::CPU_W;
    m_TransformBuffer = m_Engine.GetResourceManager().CreateGpuBuffer(gpuBufferSettings);

    //Load the material for the plane.
    MaterialData materialData;
    materialData.path = "materials/stone/";
    materialData.diffuseTextureName = "diffuse.jpg";
    materialData.normalTextureName = "normal.jpg";
    materialData.metallicTextureName = "metallic.jpg";
    materialData.roughnessTextureName = "roughness.jpg";
    materialData.aoTextureName = "ao.jpg";

    //materialData.heightTextureName = "height.jpg";
    m_PlaneMaterial = LoadMaterial(m_Engine.GetResourceManager(), materialData);

    //Transform the plane and set up the drawing for it.
    m_PlaneTransform.Scale({ MESH_SCALE / 2.f, 1.f, MESH_SCALE / 2.f });

    //Cube mesh
    MeshSettings meshSettings;
    meshSettings.indexData = &CUBE_INDICES;
    meshSettings.vertexData = &CUBE_DATA;
    meshSettings.indexDataType = DataType::USHORT;
    meshSettings.usage = MemoryUsage::GPU;
    meshSettings.access = AccessMode::READ_ONLY;
    meshSettings.vertexDataSizeBytes = sizeof(CUBE_DATA);
    meshSettings.numIndices = sizeof(CUBE_INDICES) / sizeof(CUBE_INDICES[0]);
    meshSettings.vertexSettings.EnableAttribute(VertexAttribute::POSITION_3D, 0, 32, 0);
    meshSettings.vertexSettings.EnableAttribute(VertexAttribute::NORMAL, 12, 32, 0);
    meshSettings.vertexSettings.EnableAttribute(VertexAttribute::UV_COORDS, 24, 32, 0);
    m_Cube = m_Engine.GetResourceManager().CreateMesh(meshSettings);

    //Set static plane draw data.
    m_PlaneDrawData.instanceCount = 1;
    m_PlaneDrawData.materialData.material = m_PlaneMaterial;
    m_PlaneDrawData.mesh = m_Cube;
    m_PlaneDrawData.attributes.EnableAttribute(DrawAttribute::TRANSFORMATION_MATRIX).EnableAttribute(DrawAttribute::MATERIAL_SINGLE);
    m_PlaneDrawData.transformData.dataBuffer = m_TransformBuffer;

    //Load the light visualizer.
    m_LightMesh = Sphere::Load(m_Engine, 0.2f, 16, 16);
    MaterialSettings lightMat;
    lightMat.EnableAttribute(MaterialAttribute::EMISSIVE_CONSTANT_VALUE);
    lightMat.SetEmissiveConstant({ 1, 1, 1 });
    m_LightMaterial = m_Engine.GetResourceManager().CreateMaterial(lightMat);

    //Set static light draw data.
    m_LightMeshDrawData.instanceCount = 1;
    m_LightMeshDrawData.attributes.EnableAttribute(DrawAttribute::TRANSFORMATION_MATRIX).EnableAttribute(DrawAttribute::MATERIAL_SINGLE);
    m_LightMeshDrawData.transformData.dataBuffer = m_TransformBuffer;
    m_LightMeshDrawData.materialData.material = m_LightMaterial;
    m_LightMeshDrawData.mesh = m_LightMesh;

    //Set up the light itself.
    LightSettings pointSettings;
    pointSettings.color = { 1.f, 1.f, 1.f };
    pointSettings.intensity = 250.f;
    pointSettings.pointLight.position = { 0.f, 8.f, 0.f };
    pointSettings.type = LightType::LIGHT_POINT;
    m_Light = std::reinterpret_pointer_cast<PointLight>(m_Engine.GetResourceManager().CreateLight(pointSettings));

    //Ambient light
    LightSettings ambientSettings;
    ambientSettings.color = glm::vec3(1.f);
    ambientSettings.intensity = 0.5f;
    ambientSettings.type = LightType::LIGHT_AMBIENT;
    m_AmbientLight = std::reinterpret_pointer_cast<PointLight>(m_Engine.GetResourceManager().CreateLight(ambientSettings));

    //Set up the objects draw data.
    m_DrawData.mesh = m_Cube;
    m_DrawData.attributes.EnableAttribute(DrawAttribute::TRANSFORMATION_MATRIX);
    m_DrawData.transformData.dataBuffer = m_TransformBuffer;

    //Add the actual transforms.
    {
        Transform t;
        t.SetTranslation({0.f, 5.f, -10.f});
        t.SetRotation({ 0.f, 1.f, 0.f }, 1.f);
        t.SetScale({4.f, 4.f, 4.f});
        m_Transforms.emplace_back(t);
    }
}

void ShadowTestScene::Update()
{
    using namespace blurp;

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

    //Move the light.
    const static float MOVE_SPEED = 0.1f;
    if (input.getKeyState(KEY_A) != ButtonState::NOT_PRESSED)
    {
        m_Light->SetPosition(m_Light->GetPosition() + glm::vec3{ -MOVE_SPEED, 0.f, 0.f });
    }
    if (input.getKeyState(KEY_D) != ButtonState::NOT_PRESSED)
    {
        m_Light->SetPosition(m_Light->GetPosition() + glm::vec3{ MOVE_SPEED, 0.f, 0.f });
    }
    if (input.getKeyState(KEY_E) != ButtonState::NOT_PRESSED)
    {
        m_Light->SetPosition(m_Light->GetPosition() + glm::vec3{ 0.f, MOVE_SPEED, 0.f });
    }
    if (input.getKeyState(KEY_Q) != ButtonState::NOT_PRESSED)
    {
        m_Light->SetPosition(m_Light->GetPosition() + glm::vec3{ 0.f, -MOVE_SPEED, 0.f });
    }
    if (input.getKeyState(KEY_W) != ButtonState::NOT_PRESSED)
    {
        m_Light->SetPosition(m_Light->GetPosition() + glm::vec3{ 0.f, 0.f, -MOVE_SPEED});
    }
    if (input.getKeyState(KEY_S) != ButtonState::NOT_PRESSED)
    {
        m_Light->SetPosition(m_Light->GetPosition() + glm::vec3{ 0.f, 0.f, MOVE_SPEED });
    }

    const static float MOVE_SENSITIVITY = 0.001f;
    const static float SCROLL_SENSITIVITY = 1.5f;
    const static float MIN_DISTANCE = 1.0f;
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
        m_Camera->GetTransform().RotateAround(m_PlaneTransform.GetTranslation(), m_Camera->GetTransform().GetUp(), MOVE_SENSITIVITY * mouseMoveX);
    }
    if (mouseMoveY != 0)
    {
        m_Camera->GetTransform().RotateAround(m_PlaneTransform.GetTranslation(), m_Camera->GetTransform().GetRight(), MOVE_SENSITIVITY * mouseMoveY);
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
            const auto oldDistance = glm::distance(m_Camera->GetTransform().GetTranslation(), m_PlaneTransform.GetTranslation());
            const auto moveLength = glm::length(moved);

            //Zoomed in too close. Set to correct distance.
            if (oldDistance - moveLength < MIN_DISTANCE)
            {
                m_Camera->GetTransform().LookAt(m_PlaneTransform.GetTranslation() + (m_Camera->GetTransform().GetForward() * MIN_DISTANCE), m_PlaneTransform.GetTranslation() + (m_Camera->GetTransform().GetForward() * MIN_DISTANCE * 2.f), m_Camera->GetTransform().GetUp());
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

     //Reset old data.
    m_ForwardPass->Reset();
    m_ShadowGenerationPass->Reset();

    //Calculate the mesh's MVP and upload it to the GPU buffer.
    auto matrix = m_PlaneTransform.GetTransformation();
    m_PlaneDrawData.transformData.dataRange = m_TransformBuffer->WriteData<glm::mat4>(0, 1, 16, &matrix);

    //Upload the mesh representing the light transform.
    m_LightMeshTransform.SetTranslation(m_Light->GetPosition());
    auto lightMat = m_LightMeshTransform.GetTransformation();
    m_LightMeshDrawData.transformData.dataRange = m_TransformBuffer->WriteData<glm::mat4>(m_PlaneDrawData.transformData.dataRange.end, 1, 16, &lightMat);

    //Add the light to the scene.
    m_ForwardPass->AddLight(m_Light);//TODO add shadow map index
    m_ForwardPass->AddLight(m_AmbientLight);

    std::vector<DrawData> drawDatas = {m_PlaneDrawData};

    //Add the other data for drawing.
    if(!m_Transforms.empty())
    {
        std::vector<glm::mat4> mats;
        mats.reserve(m_Transforms.size());
        for (auto& transform : m_Transforms)
        {
            mats.emplace_back(transform.GetTransformation());
        }

        //Update draw data.
        m_DrawData.instanceCount = mats.size();

        //Upload.
        m_DrawData.transformData.dataRange = m_TransformBuffer->WriteData<glm::mat4>(m_LightMeshDrawData.transformData.dataRange.end, mats.size(), 16, &mats[0]);

        drawDatas.push_back(m_DrawData);
    }

    //Don't count the light for the shadow.
    drawDatas.push_back(m_LightMeshDrawData);

    //Setup the shadow mapping for this frame.
    m_ShadowGenerationPass->AddLight(m_Light, 0, NEAR_PLANE, FAR_PLANE);
    std::vector<LightIndexData> lIndexData;
    lIndexData.reserve(drawDatas.size());
    for(int i = 0; i < static_cast<int>(drawDatas.size()); ++i)
    {
        lIndexData.emplace_back(LightIndexData{std::vector<int>(), std::vector<int>{0}});
    }
    m_ShadowGenerationPass->SetGeometry(&drawDatas[0], &lIndexData[0], lIndexData.size());



    //Queue for draw.
    m_ForwardPass->SetDrawData(&drawDatas[0], drawDatas.size());

    //Update the rendering pipeline.
    m_Pipeline->Execute();

    while (true)
    {
        if (m_Pipeline->HasFinishedExecuting())
        {
            break;
        }
    }

    //printscreen.
    if(input.getKeyState(KEY_P) == ButtonState::FIRST_PRESSED)
    {
        std::string path = "shadowmap";
        std::string extension = ".jpg";
        for(int i = 0; i < 6; ++i)
        {
            std::string file = path + std::to_string(i) + extension;

            unsigned char* data = m_PosShadowArray->GetPixels(glm::vec3(0.f, 0.f, i), glm::vec3(1024, 1024, 1), 1);
            unsigned char* converted = new unsigned char[1024 * 1024 * 3];

            float* asFloat = reinterpret_cast<float*>(data);
            for(int i = 0; i < 1024; ++i)
            {
                for(int j = 0; j < 1024; ++j)
                {
                    int coordX = (3 * 1024 * i);
                    int coordY =  (3 * j);

                    float f = asFloat[i * 1024 + j];
                    assert(f >= 0.f && f <= 1.f);

                    float linear = (2.0 * NEAR_PLANE) / (FAR_PLANE + NEAR_PLANE - f * (FAR_PLANE - NEAR_PLANE));

                    unsigned char value =  static_cast<unsigned char>(linear * 255.f);
                    for (int k = 0; k < 3; ++k)
                    {
                        converted[coordX + coordY + 0] = value;
                        converted[coordX + coordY + 1] = value;
                        converted[coordX + coordY + 2] = value;
                    }
                }
            }

            SaveToImage(file, 1024, 1024, 3, converted);

            delete[] data;
        }
    }
}
