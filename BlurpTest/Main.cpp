#include <BlurpEngine.h>
#include <iostream>
#include <Settings.h>
#include <glm/glm.hpp>
#include <Window.h>


#include "RenderPass_Forward.h"
#include "KeyCodes.h"
#include "RenderResourceManager.h"
#include "RenderPipeline.h"
#include "RenderPass_HelloTriangle.h"
#include "RenderTarget.h"
#include <glm/gtc/type_ptr.hpp>

//Color and vertices for a cube.
float cubeVertices[] = {
    // front
    -1.0, -1.0,  1.0, 0.5f, 1.0, 1.0,
     1.0, -1.0,  1.0, 1.f, 0.5, 1.0,
     1.0,  1.0,  1.0, 0.f, 1.0, 0.7,
    -1.0,  1.0,  1.0, 0.f, 0.0, 1.0,
    // back
    -1.0, -1.0, -1.0, 0.f, 1.0, 1.0,
     1.0, -1.0, -1.0, 1.f, 1.0, 1.0,
     1.0,  1.0, -1.0, 1.f, 0.0, 1.0,
    -1.0,  1.0, -1.0, 1.f, 1.0, 0.0,
};

std::uint16_t cubeIndices[]
{
        0, 1, 2,
        2, 3, 0,
        1, 5, 6,
        6, 2, 1,
        7, 6, 5,
        5, 4, 7,
        4, 0, 3,
        3, 7, 4,
        4, 5, 1,
        1, 0, 4,
        3, 2, 6,
        6, 7, 3
};

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

    //Create a test render target.
    TextureSettings colorAttachment;
    colorAttachment.dimensions = { window->GetDimensions(), 1};
    colorAttachment.memoryAccess = AccessMode::READ_WRITE;
    colorAttachment.pixelFormat = PixelFormat::RGBA;
    colorAttachment.dataType = DataType::UBYTE;

    RenderTargetSettings renderTargetSettings;
    renderTargetSettings.viewPort = { 0, 0, window->GetDimensions() };
    auto colorAttachmentTex = engine.GetResourceManager().CreateTexture(colorAttachment);

    renderTargetSettings.defaultColorAttachment = colorAttachmentTex;

    auto renderTarget = engine.GetResourceManager().CreateRenderTarget(renderTargetSettings);

    //Set up a pipeline that draws a triangle onto a texture. Then it draws a triangle with the first texture on it on the 2nd triangle on the screen.
    PipelineSettings pSettings;
    pSettings.waitForGpu = true;
    auto pipeline = engine.GetResourceManager().CreatePipeline(pSettings);

    //Draw a triangle in the render target.
    auto triangleRenderPass = pipeline->AppendRenderPass<RenderPass_HelloTriangle>(RenderPassType::RP_HELLOTRIANGLE);
    triangleRenderPass->SetColor({ 1.f, 1.f, 1.f, 1.f });
    renderTarget->SetClearColor({ 0.f, 0.f, 1.f, 1.f });
    triangleRenderPass->SetTarget(renderTarget);

    //Draw a triangle on the screen. Texture the triangle with the previously rendered texture.
    auto triangleRenderPass2 = pipeline->AppendRenderPass<RenderPass_HelloTriangle>(RenderPassType::RP_HELLOTRIANGLE);
    window->GetRenderTarget()->SetClearColor({ 0.f, 0.1f, 0.1f,1.f });
    triangleRenderPass2->SetTarget(window->GetRenderTarget());
    triangleRenderPass2->SetColor({ 1.f, 0.6f, 0.2f, 1.f });
    triangleRenderPass2->SetTexture(renderTarget->GetColorAttachment(0));


    triangleRenderPass->SetEnabled(false);
    triangleRenderPass2->SetEnabled(false);

    CameraSettings camSettings;
    camSettings.width = window->GetDimensions().x;
    camSettings.height = window->GetDimensions().y;
    camSettings.farPlane = 100000000000000.f;
    auto camera = engine.GetResourceManager().CreateCamera(camSettings);
    auto forwardPass = pipeline->AppendRenderPass<RenderPass_Forward>(RenderPassType::RP_FORWARD);
    forwardPass->SetCamera(camera);
    forwardPass->SetTarget(window->GetRenderTarget());

    //Scene graph with a mesh and transforms.
    MeshSettings meshSettings;
    meshSettings.indexData = &cubeIndices;
    meshSettings.vertexData = &cubeVertices;
    meshSettings.indexDataType = DataType::USHORT;
    meshSettings.usage = AccessMode::READ;
    meshSettings.vertexDataSizeBytes = sizeof(cubeVertices);
    meshSettings.numIndices = sizeof(cubeIndices) / sizeof(cubeIndices[0]);
    meshSettings.vertexSettings.EnableAttribute(VertexAttribute::POSITION_3D, 0, 24, 0);
    meshSettings.vertexSettings.EnableAttribute(VertexAttribute::COLOR, 12, 24, 0);

    std::shared_ptr<Mesh> mesh = engine.GetResourceManager().CreateMesh(meshSettings);
    std::vector<glm::mat4> transforms;

    Transform transform;
    transform.SetTranslation({ 0, 0, 0.f });

    const int numSpasmCubes = 1000;

    //Init the scene graph.
    for(int i = 0; i < numSpasmCubes; ++i)
    {
        transforms.emplace_back(transform.GetTransformation());
    }





    /*
     * Generate a big instance enabled mesh for testing.
     */
    const int numinstances = 100000;
    const float maxDistance = 200000.f;
    const float maxRotation = 6.28f;
    const float maxScale = 20.f;
    const float minScale = 0.5f;

    std::vector<float> floats;
    floats.reserve(16 * numinstances + sizeof(cubeVertices));

    Transform t;

    //Fill the floats array with matrices. numInstances is the number of instances.
    for(int i = 0; i < numinstances; ++i)
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

        for(int i = 0; i < 16; ++i)
        {
            floats.push_back(ptr[i]);
        }
    }

    //Emplace the actual mesh data in the floats array.
    for(auto& f : cubeVertices)
    {
        floats.push_back(f);
    }

    //Data for the mesh with all vertex attribs and pointers to the data.
    MeshSettings instanceMeshSettings;
    instanceMeshSettings.indexData = &cubeIndices;
    instanceMeshSettings.vertexData = &floats[0];
    instanceMeshSettings.indexDataType = DataType::USHORT;
    instanceMeshSettings.usage = AccessMode::READ;
    instanceMeshSettings.vertexDataSizeBytes = floats.size() * sizeof(float);
    instanceMeshSettings.numIndices = sizeof(cubeIndices) / sizeof(cubeIndices[0]);
    instanceMeshSettings.vertexSettings.EnableAttribute(VertexAttribute::POSITION_3D, 16 * numinstances * sizeof(float), 24, 0);
    instanceMeshSettings.vertexSettings.EnableAttribute(VertexAttribute::COLOR, (16 * numinstances * sizeof(float)) + 12, 24, 0);
    instanceMeshSettings.vertexSettings.EnableAttribute(VertexAttribute::MATRIX, 0, 16 * sizeof(float), 1);
    instanceMeshSettings.instanceCount = numinstances;

    //Creat the mesh from the generated data.
    std::shared_ptr<Mesh> instanced = engine.GetResourceManager().CreateMesh(instanceMeshSettings);

    //Generate the queue data and mark the mesh for drawing.
    Transform iMTransform;
    iMTransform.SetTranslation({ 0.f, 0.f, -40.f });
    glm::mat4 m = iMTransform.GetTransformation();
















    /*
     * GPU BUFFER TESTING.
     */

    GpuBufferSettings gpuBufferSettings;
    gpuBufferSettings.size = std::pow(2, 15);
    gpuBufferSettings.resizeWhenFull = true;
    gpuBufferSettings.memoryUsage = AccessMode::READ_WRITE;
    auto gpuBuffer = engine.GetResourceManager().CreateGpuBuffer(gpuBufferSettings);

    forwardPass->SetGpuBuffer(gpuBuffer);


    //Draw one instance of the massively instanced mesh.
    InstanceDrawQueueData iData;
    iData.mesh = instanced.get();
    iData.count = 1;

    //Draw many instances of the single cubes that spasm around.
    InstanceDrawQueueData data;
    data.mesh = mesh.get();
    data.count = transforms.size();


    //Vector containing all PV multiplied transforms. Will be updated each frame.
    std::vector<glm::mat4> transformed;
    transformed.resize(transforms.size());












    /*
     * Main loop. Render as long as the window remains open.
     */
    while (!window->IsClosed())
    {
        auto input = window->PollInput();

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
                window->Close();
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
            window->SetFullScreen(!window->IsFullScreen());
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

        //Update the rotation of the instanced mesh.
        iMTransform.Rotate(Transform::GetWorldUp(), 0.005f);
        m = iMTransform.GetTransformation();


        /*
         * Upload the updated matrix data to the GPU.
         */

        //Reset old data.
        forwardPass->Reset();

        //Get camera.
        auto pv = camera->GetProjectionMatrix() * camera->GetViewMatrix();

        //Transforms all matrices with the camera and store them.
        for (size_t i = 0; i < transforms.size(); ++i)
        {
            transformed[i] = pv * transforms[i];
        }

        //Update the single transform for the 20 million cubes.
        m = pv * iMTransform.GetTransformation();

        data.dataRange = gpuBuffer->WriteData<glm::mat4>(static_cast<void*>(0), transformed.size(), 16, &transformed[0]);
        iData.dataRange = gpuBuffer->WriteData<glm::mat4>(static_cast<void*>(0), 1, 16, &m);

        //Queue for draw.
        forwardPass->QueueForDraw(data);
        forwardPass->QueueForDraw(iData);

        //Update the rendering pipeline.
        pipeline->Execute();

        while(true)
        {
            if (pipeline->HasFinishedExecuting())
            {
                break;
            }
        }

        //Finally display on the screen.
        window->Present();
    }


    std::cout << "Window closed." << std::endl;
    getchar();

    return 0;
}
