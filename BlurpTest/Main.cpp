#include <BlurpEngine.h>
#include <iostream>
#include <Settings.h>
#include <glm/glm.hpp>
#include <Window.h>

#include "KeyCodes.h"
#include "RenderResourceManager.h"
#include "RenderPipeline.h"
#include "RenderPass_HelloTriangle.h"
#include "RenderTarget.h"

int main()
{
    using namespace blurp;

    //SETUP

    BlurpEngine engine;
    BlurpSettings blurpSettings;
    blurpSettings.graphicsAPI = GraphicsAPI::OPENGL;

    WindowSettings windowSettings;
    windowSettings.dimensions = glm::vec2{ 800, 800 };
    windowSettings.type = WindowType::WINDOW_WIN32;
    windowSettings.name = "My lovely little window";
    windowSettings.flags = WindowFlags::CAPTURE_CURSOR | WindowFlags::HIDE_CURSOR;
    windowSettings.swapChainSettings.vsync = false ;

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
    auto pipeline = engine.GetResourceManager().CreatePipeline();

    //Draw a triangle in the render target.
    auto triangleRenderPass = pipeline->AppendRenderPass<RenderPass_HelloTriangle>(RenderPassType::RP_HELLOTRIANGLE);
    triangleRenderPass->SetColor({ 1.f, 1.f, 1.f, 1.f });
    renderTarget->SetClearColor({ 0.f, 0.f, 1.f, 1.f });
    triangleRenderPass->SetTarget(renderTarget);

    //Draw a triangle on the screen. Texture the triangle with the previously rendered texture.
    auto triangleRenderPass2 = pipeline->AppendRenderPass<RenderPass_HelloTriangle>(RenderPassType::RP_HELLOTRIANGLE);
    window->GetRenderTarget()->SetClearColor({ 1.f, 0.1f, 0.1f,1.f });
    triangleRenderPass2->SetTarget(window->GetRenderTarget());
    triangleRenderPass2->SetColor({ 1.f, 0.6f, 0.2f, 1.f });
    triangleRenderPass2->SetTexture(renderTarget->GetColorAttachment(0));

    //The result should be a red screen with a triangle in it. The triangle should be blue with a second triangle inside it.

    /*
     * Main loop. Render as long as the window remains open.
     */
    while(!window->IsClosed())
    {
        auto input = window->PollInput();

        KeyboardEvent kEvent;
        MouseEvent mEvent;

        while(input.getNextEvent(kEvent))
        {
            if(kEvent.keyCode >= 48 && kEvent.keyCode <= 90)
            {
                std::cout << "Key input: " << std::string(1, static_cast<char>(kEvent.keyCode)) << " was " << (kEvent.action == KeyboardAction::KEY_PRESSED ? "pressed" : "released") << "." << std::endl;
            }
            else
            {
                std::cout << "System Key input: " << kEvent.keyCode << " was " << (kEvent.action == KeyboardAction::KEY_PRESSED ? "pressed" : "released") << "." << std::endl;
            }

            if(kEvent.keyCode == KEY_ESCAPE)
            {
                std::cout << "Closing window";
                window->Close();
            }
        }

        while (input.getNextEvent(mEvent))
        {
            if(mEvent.action == MouseAction::RELEASE || mEvent.action == MouseAction::CLICK)
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
        if(input.getKeyState(KEY_ALT) != ButtonState::NOT_PRESSED && input.getKeyState(KEY_ENTER) == ButtonState::FIRST_PRESSED)
        {
            std::cout << "Toggling fullscreen mode" << std::endl;
            window->SetFullScreen(!window->IsFullScreen());
        }

        //Toggle the render pass on or off at runtime.
        //if(input.getKeyState(KEY_T) == ButtonState::FIRST_PRESSED)
        //{
        //    triangleRenderPass->SetEnabled(!triangleRenderPass->IsEnabled());
        //}

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
