#include "UniverseScene.h"
#include <BlurpEngine.h>
#include <iostream>
#include <Settings.h>
#include <glm/glm.hpp>
#include <Window.h>
#include <GpuBuffer.h>

#include <RenderPass_Forward.h>
#include <KeyCodes.h>
#include <RenderResourceManager.h>
#include <RenderPipeline.h>
#include <RenderPass_HelloTriangle.h>
#include <RenderTarget.h>
#include <glm/gtc/type_ptr.hpp>
#include <MaterialFile.h>
//#include "MaterialLoader.h"

#include "MeshFile.h"
#include "Sphere.h"

using namespace blurp;

void UniverseScene::Init()
{
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

    //Create a test render target.
    TextureSettings colorAttachment;
    colorAttachment.dimensions = { m_Window->GetDimensions(), 1 };
    colorAttachment.memoryAccess = AccessMode::READ_WRITE;
    colorAttachment.pixelFormat = PixelFormat::RGBA;
    colorAttachment.dataType = DataType::UBYTE;


    //Set up a pipeline that draws a triangle onto a texture. Then it draws a triangle with the first texture on it on the 2nd triangle on the screen.
    PipelineSettings pSettings;
    pSettings.waitForGpu = true;
    pipeline = m_Engine.GetResourceManager().CreatePipeline(pSettings);

    //Add a clear pass first.
    m_ClearPass = pipeline->AppendRenderPass<RenderPass_Clear>(RenderPassType::RP_CLEAR);

    //Set the clear color.
    auto renderTarget = m_Window->GetRenderTarget();
    renderTarget->SetClearColor({ 0.f, 0.1f, 0.1f, 1.f });

    //Mark the render target for clearing at the start of each pass.
    m_ClearPass->AddRenderTarget(renderTarget);

    CameraSettings camSettings;
    camSettings.width = m_Window->GetDimensions().x;
    camSettings.height = m_Window->GetDimensions().y;
    camSettings.farPlane = 9000.f;
    camera = m_Engine.GetResourceManager().CreateCamera(camSettings);
    forwardPass = pipeline->AppendRenderPass<RenderPass_Forward>(RenderPassType::RP_FORWARD);
    forwardPass->SetCamera(camera);
    forwardPass->SetTarget(m_Window->GetRenderTarget());

    //Resize callback.
    m_Window->SetResizeCallback([&](int w, int h)
    {
        //Update camera.
        CameraSettings camS;
        camS.width = w;
        camS.height = h;
        camS.fov = 90.f;
        camS.nearPlane = 0.1f;
        camS.farPlane = 9000.f;
        camera->UpdateSettings(camS);
    });

    //Scene graph with a mesh and transforms.
    MeshSettings meshSettings;
    meshSettings.indexData = &CUBE_INDICES;
    meshSettings.vertexData = &CUBE_DATA;
    meshSettings.indexDataType = DataType::USHORT;
    meshSettings.usage = MemoryUsage::GPU;
    meshSettings.access = AccessMode::READ_ONLY;
    meshSettings.vertexDataSizeBytes = sizeof(CUBE_DATA);
    meshSettings.numIndices = sizeof(CUBE_INDICES) / sizeof(CUBE_INDICES[0]);
    meshSettings.vertexSettings.EnableAttribute(VertexAttribute::POSITION_3D, 0, 44, 0);
    meshSettings.vertexSettings.EnableAttribute(VertexAttribute::NORMAL, 12, 44, 0);
    meshSettings.vertexSettings.EnableAttribute(VertexAttribute::UV_COORDS, 24, 44, 0);
    meshSettings.vertexSettings.EnableAttribute(VertexAttribute::COLOR, 32, 44, 0);

    /*
     * Mesh file testing here
     */
    std::cout << "Creating mesh file Cube1." << std::endl;
    bool created = CreateMeshFile(meshSettings, "meshes/", "Cube1");
    std::cout << (created ? "Success!" : "Failed.") << std::endl;

    std::cout << "Loading mesh file Cube1." << std::endl;
    std::shared_ptr<Mesh> mesh = LoadMeshFile(m_Engine.GetResourceManager(), "meshes/Cube1");
    //------------------------

    //std::shared_ptr<Mesh> mesh = m_Engine.GetResourceManager().CreateMesh(meshSettings);
    

    Transform transform;
    transform.SetTranslation({ 0, 0, 0.f });

    numSpasmCubes = 100;

    //Init the scene graph.
    for (int i = 0; i < numSpasmCubes; ++i)
    {
        transforms.emplace_back(transform.GetTransformation());
    }

    /*
     * Generate a big instance enabled mesh for testing.
     */
    const int numinstances = 1000000;
    const float maxDistance = 10000.f;
    const float maxRotation = 6.28f;
    const float maxScale = 20.f;
    const float minScale = 0.5f;

    std::vector<float> floats;
    floats.reserve((16 * numinstances) + sizeof(CUBE_DATA) + (numinstances * sizeof(float)));

    Transform t;

    //Fill the floats array with matrices. numInstances is the number of instances.
    for (int i = 0; i < numinstances; ++i)
    {
        //Generate an orientation.
        float x = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f) * maxDistance;
        float y = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f) * maxDistance;
        float z = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f) * maxDistance;
        float rotx = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * maxRotation;
        float roty = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * maxRotation;
        float rotz = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * maxRotation;

        float scalex = minScale + ((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * (maxScale - minScale));
        float scaley = minScale + ((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * (maxScale - minScale));
        float scalez = minScale + ((static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * (maxScale - minScale));

        t.SetTranslation({ x, y, z });
        t.SetRotation(Transform::GetWorldUp(), roty);
        t.SetRotation(Transform::GetWorldRight(), rotx);
        t.SetRotation(Transform::GetWorldForward(), rotz);
        t.SetScale({ scalex, scaley, scalez });

        glm::mat4 mat = t.GetTransformation();

        float* ptr = reinterpret_cast<float*>(&mat);

        for (int i = 0; i < 16; ++i)
        {
            floats.push_back(ptr[i]);
        }
    }

    //Emplace the material ID between 0 and 2 in the dataset.
    int matID = 0;
    for(auto i = 0; i < numinstances; ++i)
    {
        floats.push_back(matID);
        ++matID;
        if(matID > 2)
        {
            matID = 0;
        }
    }

    //Emplace the actual mesh data in the floats array.
    for (auto& f : CUBE_DATA)
    {
        floats.push_back(f);
    }

    //Data for the mesh with all vertex attribs and pointers to the data.
    MeshSettings instanceMeshSettings;
    instanceMeshSettings.indexData = &CUBE_INDICES;
    instanceMeshSettings.vertexData = &floats[0];
    instanceMeshSettings.indexDataType = DataType::USHORT;
    instanceMeshSettings.usage = MemoryUsage::GPU;
    instanceMeshSettings.access = AccessMode::READ_ONLY;
    instanceMeshSettings.vertexDataSizeBytes = floats.size() * sizeof(float);
    instanceMeshSettings.numIndices = sizeof(CUBE_INDICES) / sizeof(CUBE_INDICES[0]);
    instanceMeshSettings.vertexSettings.EnableAttribute(VertexAttribute::POSITION_3D, (17 * numinstances * sizeof(float)) + 0, 44, 0);
    instanceMeshSettings.vertexSettings.EnableAttribute(VertexAttribute::NORMAL, 17 * numinstances * sizeof(float) + 12, 44, 0);
    instanceMeshSettings.vertexSettings.EnableAttribute(VertexAttribute::UV_COORDS, 17 * numinstances * sizeof(float) + 24, 44, 0);
    instanceMeshSettings.vertexSettings.EnableAttribute(VertexAttribute::COLOR, 17 * numinstances * sizeof(float) + 32, 44, 0);
    instanceMeshSettings.vertexSettings.EnableAttribute(VertexAttribute::MATRIX, 0, 16 * sizeof(float), 1);
    instanceMeshSettings.vertexSettings.EnableAttribute(VertexAttribute::MATERIAL_ID, 16 * numinstances * sizeof(float), 0, 1);
    instanceMeshSettings.instanceCount = numinstances;

    //Create the mesh from the generated data.
    std::shared_ptr<Mesh> instanced = m_Engine.GetResourceManager().CreateMesh(instanceMeshSettings);

    //Generate the queue data and mark the mesh for drawing.
    iMTransform.SetTranslation({ 0.f, 0.f, -40.f });
    m = iMTransform.GetTransformation();

    /*
     * GPU BUFFER TESTING.
     */

    GpuBufferSettings gpuBufferSettings;
    gpuBufferSettings.size = std::pow(2, 15);
    gpuBufferSettings.resizeWhenFull = true;
    gpuBufferSettings.memoryUsage = MemoryUsage::CPU_W;
    gpuBuffer = m_Engine.GetResourceManager().CreateGpuBuffer(gpuBufferSettings);

    //Draw one instance of the massively instanced mesh.
    iData.mesh = instanced;
    iData.instanceCount = 1;
    iData.attributes.EnableAttribute(DrawAttribute::TRANSFORMATION_MATRIX).EnableAttribute(DrawAttribute::MATERIAL_BATCH);
    iData.transformData.dataBuffer = gpuBuffer;
    iData.materialData.materialBatch = LoadMaterialBatch(m_Engine.GetResourceManager(), "materials/Batch/MaterialBatch");
    //iData.materialData.materialBatch = LoadMaterialBatch(m_Engine.GetResourceManager());

    //Draw many instances of the single cubes that spasm around.
    data.mesh = mesh;
    data.instanceCount = transforms.size();
    data.attributes.EnableAttribute(DrawAttribute::TRANSFORMATION_MATRIX);
    data.transformData.dataBuffer = gpuBuffer;


    //THE SUN!
    sunData.mesh = Sphere::Load(m_Engine, 50.f, 128, 128);
    sunData.instanceCount = 1;


    {
        MaterialSettings sunSettings;
        sunSettings.SetEmissiveConstant({ 1.f, 1.f, 0.05f });
        sunSettings.EnableAttribute(MaterialAttribute::EMISSIVE_CONSTANT_VALUE);
        sunData.materialData.material = m_Engine.GetResourceManager().CreateMaterial(sunSettings);
        sunData.transformData.dataBuffer = gpuBuffer;
        sunData.attributes.EnableAttribute(DrawAttribute::TRANSFORMATION_MATRIX).EnableAttribute(DrawAttribute::MATERIAL_SINGLE);
    }

    {
        LightSettings sunSettings;
        sunSettings.type = LightType::LIGHT_POINT;
        sunSettings.color = glm::vec3(0.9f, 0.9f, 0.4f);
        sunSettings.intensity = 100000.f;
        m_Sun = std::reinterpret_pointer_cast<PointLight>(m_Engine.GetResourceManager().CreateLight(sunSettings));
    }
}

void UniverseScene::Update()
{
    auto input = m_Window->PollInput();

    KeyboardEvent kEvent;
    MouseEvent mEvent;

    while (input.getNextEvent(kEvent))
    {
        if (kEvent.keyCode >= 48 && kEvent.keyCode <= 90)
        {
            std::cout << "Key input: " << std::string(1, static_cast<char>(kEvent.keyCode)) << " was " << (kEvent.action == KeyboardAction::KEY_PRESSED ? "pressed" : "released") << "." << std::endl;
        }
        else
        {
            std::cout << "System Key input: " << kEvent.keyCode << " was " << (kEvent.action == KeyboardAction::KEY_PRESSED ? "pressed" : "released") << "." << std::endl;
        }

        if (kEvent.keyCode == KEY_ESCAPE)
        {
            std::cout << "Closing window";
            m_Window->Close();
        }
    }

    while (input.getNextEvent(mEvent))
    {
        if (mEvent.action == MouseAction::RELEASE || mEvent.action == MouseAction::CLICK)
        {
            std::string mButton;
            switch (mEvent.button)
            {
            case MouseButton::LMB:
                mButton = "LMB";
                break;
            case MouseButton::RMB:
                mButton = "RMB";
                break;
            case MouseButton::MMB:
                mButton = "MMB";
                break;
            }

            std::string pressrelease = (mEvent.action == MouseAction::RELEASE ? "released." : "pressed.");

            std::cout << "Mouse input: " << mButton << " was " << pressrelease << std::endl;
        }

        //else if(mEvent.action == MouseAction::SCROLL)
        //{
        //    std::cout << "Mouse input: Scrolled distance " << mEvent.value << "." << std::endl;
        //}
        //else if (mEvent.action == MouseAction::MOVE_X)
        //{
        //    std::cout << "Mouse moved x: " << mEvent.value << "." << std::endl;
        //}
        //else if (mEvent.action == MouseAction::MOVE_Y)
        //{
        //    std::cout << "Mouse moved y: " << mEvent.value << "." << std::endl;
        //}
    }

    //Handle alt enter:
    if (input.getKeyState(KEY_ALT) != ButtonState::NOT_PRESSED && input.getKeyState(KEY_ENTER) == ButtonState::FIRST_PRESSED)
    {
        std::cout << "Toggling fullscreen mode" << std::endl;
        m_Window->SetFullScreen(!m_Window->IsFullScreen());
    }

    //Toggle the render pass on or off at runtime.
    //if(input.getKeyState(KEY_T) == ButtonState::FIRST_PRESSED)
    //{
    //    triangleRenderPass->SetEnabled(!triangleRenderPass->IsEnabled());
    //}
    //

    //Update the positions of the cubes.
    for (int i = 0; i < numSpasmCubes; ++i)
    {
        auto& mat = transforms[i];

        float x = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f;
        float y = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f;
        float z = static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f;

        mat = glm::translate(mat, { x, y, z });
    }

    //Update the camera based on input.
    {
        auto& transform = camera->GetTransform();
        bool shift = input.getKeyState(KEY_SHIFT) != ButtonState::NOT_PRESSED;

        const float movespeed = 2.f * (shift ? 20.f : 2.f);

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
            transform.Rotate(transform.GetRight(), 0.05f);
        }
        if (input.getKeyState(KEY_LEFT) != ButtonState::NOT_PRESSED)
        {
            transform.Rotate(transform.GetUp(), 0.05f);
        }
        if (input.getKeyState(KEY_DOWN) != ButtonState::NOT_PRESSED)
        {
            transform.Rotate(transform.GetRight(), -0.05f);
        }
        if (input.getKeyState(KEY_RIGHT) != ButtonState::NOT_PRESSED)
        {
            transform.Rotate(transform.GetUp(), -0.05f);
        }
    }

    /*
     * Upload the updated matrix data to the GPU.
     */

     //Reset old data.
    forwardPass->Reset();

    //Update the single transform for the 20 million cubes.
    iMTransform.Rotate(Transform::GetWorldUp(), 0.005f);
    m = iMTransform.GetTransformation();

    //Update the sun
    Transform sunTransform;
    sunTransform.Translate({ -5, 10, -5 });
    auto sunMat = sunTransform.GetTransformation();


    data.transformData.dataRange = gpuBuffer->WriteData<glm::mat4>(0, transforms.size(), 16, &transforms[0]);
    iData.transformData.dataRange = gpuBuffer->WriteData<glm::mat4>(data.transformData.dataRange.end, 1, 16, &m);
    sunData.transformData.dataRange = gpuBuffer->WriteData<glm::mat4>(iData.transformData.dataRange.end, 1, 16, &sunMat);

    //Queue for draw.
    std::vector<DrawData> drawDatas = { data, iData, sunData};
    forwardPass->SetDrawData({ &drawDatas[0], static_cast<std::uint32_t>(drawDatas.size()) });

    LightData lData;
    LightUploadData lud;
    lud.lightData = &lData;

    lud.point.count = 1;
    lud.point.lights = &m_Sun;

    gpuBuffer->WriteData(sunData.transformData.dataRange.end, lud);

    //Add the light to the scene.
    forwardPass->SetLights(lData);

    //Update the rendering pipeline.
    pipeline->Execute();

    while (true)
    {
        if (pipeline->HasFinishedExecuting())
        {
            break;
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(6));
}
