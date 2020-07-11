#include <BlurpEngine.h>
#include <iostream>
#include <Settings.h>
#include <glm/glm.hpp>
#include <Window.h>

#include "KeyCodes.h"
#include "RenderResourceManager.h"
#include "RenderPipeline.h"
#include "RenderPass_HelloTriangle.h"
#include "SwapChain.h"

int main()
{
    //SETUP

    blurp::BlurpEngine engine;
    blurp::BlurpSettings blurpSettings;
    blurpSettings.graphicsAPI = blurp::GraphicsAPI::OPENGL;

    blurp::WindowSettings windowSettings;
    windowSettings.dimensions = glm::vec2{ 800, 800 };
    windowSettings.type = blurp::WindowType::WINDOW_WIN32;
    windowSettings.name = "My lovely little window";
    windowSettings.flags = blurp::WindowFlags::CAPTURE_CURSOR | blurp::WindowFlags::HIDE_CURSOR;
    windowSettings.swapChainSettings.vsync = false ;

    blurpSettings.windowSettings = windowSettings;
    engine.Init(blurpSettings);
    auto window = engine.GetWindow();


    //RENDERING

    auto pipeline = engine.GetResourceManager().CreatePipeline();
    auto triangleRenderPass = pipeline->AppendRenderPass<blurp::RenderPass_HelloTriangle>(blurp::RenderPassType::RP_HELLOTRIANGLE);

    triangleRenderPass->SetTarget(window->GetRenderTarget());
    triangleRenderPass->SetColor({0.f, 0.f, 1.f, 1.f});

    /*
     * Main loop. Render as long as the window remains open.
     */
    while(!window->IsClosed())
    {
        auto input = window->PollInput();

        blurp::KeyboardEvent kEvent;
        blurp::MouseEvent mEvent;

        while(input.getNextEvent(kEvent))
        {
            if(kEvent.keyCode >= 48 && kEvent.keyCode <= 90)
            {
                std::cout << "Key input: " << std::string(1, static_cast<char>(kEvent.keyCode)) << " was " << (kEvent.action == blurp::KeyboardAction::KEY_PRESSED ? "pressed" : "released") << "." << std::endl;
            }
            else
            {
                std::cout << "System Key input: " << kEvent.keyCode << " was " << (kEvent.action == blurp::KeyboardAction::KEY_PRESSED ? "pressed" : "released") << "." << std::endl;
            }

            if(kEvent.keyCode == KEY_ESCAPE)
            {
                std::cout << "Closing window";
                window->Close();
            }
        }

        while (input.getNextEvent(mEvent))
        {
            if(mEvent.action == blurp::MouseAction::RELEASE || mEvent.action == blurp::MouseAction::CLICK)
            {
                std::string mButton;
                switch (mEvent.button)
                {
                case blurp::MouseButton::LMB:
                    mButton = "LMB";
                    break;
                case blurp::MouseButton::RMB:
                    mButton = "RMB";
                    break;
                case blurp::MouseButton::MMB:
                    mButton = "MMB";
                    break;
                }

                std::string pressrelease = (mEvent.action == blurp::MouseAction::RELEASE ? "released." : "pressed.");

                std::cout << "Mouse input: " << mButton << " was " << pressrelease << std::endl;
            }

            //else if(mEvent.action == blurp::MouseAction::SCROLL)
            //{
            //    std::cout << "Mouse input: Scrolled distance " << mEvent.value << "." << std::endl;
            //}
            //else if (mEvent.action == blurp::MouseAction::MOVE_X)
            //{
            //    std::cout << "Mouse moved x: " << mEvent.value << "." << std::endl;
            //}
            //else if (mEvent.action == blurp::MouseAction::MOVE_Y)
            //{
            //    std::cout << "Mouse moved y: " << mEvent.value << "." << std::endl;
            //}
        }

        //Handle alt enter:
        if(input.getKeyState(KEY_ALT) != blurp::ButtonState::NOT_PRESSED && input.getKeyState(KEY_ENTER) == blurp::ButtonState::FIRST_PRESSED)
        {
            std::cout << "Toggling fullscreen mode" << std::endl;
            window->SetFullScreen(!window->IsFullScreen());
        }

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
