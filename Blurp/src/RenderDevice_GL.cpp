#include "opengl/RenderDevice_GL.h"

#include "opengl/RenderTarget_GL.h"
#include "BlurpEngine.h"
#include "opengl/RenderPass_HelloTriangle_GL.h"
#include "opengl/RenderPipeline_GL.h"
#include "opengl/SwapChain_GL_Win32.h"


namespace blurp
{
    bool RenderDevice_GL::Init(BlurpEngine& a_BlurpEngine, const WindowSettings& a_WindowSettings)
    {
        const auto window = a_BlurpEngine.GetWindow();
        if(window == nullptr)
        {
            throw std::exception("For OpenGL contexts, a window is required!");
            return false;
        }

        //Create a swapchain, which will internally access the window and bind to it.
        const auto swapChain = CreateSwapChain(a_WindowSettings);

        //Bind the swapchain to the window.This loads the swapchain.
        BindWindowAndSwapChain(a_BlurpEngine, window.get(), swapChain);

        //Inside the swapchain, the OpenGL rendering context was created (OpenGL requires a window, it's dumb).
        //Now init glew and make sure it works correctly.
        if(glewInit() != GLEW_OK)
        {
            throw std::exception("Glew could not be initialized! How horrible!");
            return false;
        }

        return true;
    }

    std::shared_ptr<Light> RenderDevice_GL::CreateLight(const LightSettings& a_Settings)
    {
        return nullptr;
    }

    std::shared_ptr<Camera> RenderDevice_GL::CreateCamera(const CameraSettings& a_Settings)
    {
        return nullptr;
    }

    std::shared_ptr<Mesh> RenderDevice_GL::CreateMesh(const MeshSettings& a_Settings)
    {
        return nullptr;
    }

    std::shared_ptr<Texture> RenderDevice_GL::CreateTexture(const TextureSettings& a_Settings)
    {
        return nullptr;
    }

    std::shared_ptr<RenderTarget> RenderDevice_GL::CreateRenderTarget(const RenderTargetSettings& a_Settings)
    {
        return std::make_shared<RenderTarget_GL>(a_Settings, false);
    }

    std::shared_ptr<SwapChain> RenderDevice_GL::CreateSwapChain(const WindowSettings& a_Settings)
    {
        switch(a_Settings.type)
        {
        case WindowType::WINDOW_WIN32:
            return std::make_shared<SwapChain_GL_Win32>(a_Settings.swapChainSettings);
        default:
            throw std::exception("Swap chain for given window not implemented for OpenGL!");
            return nullptr;
        }
        
    }

    std::shared_ptr<Material> RenderDevice_GL::CreateMaterial(const MaterialSettings& a_Settings)
    {
        return nullptr;
    }

    std::shared_ptr<RenderPass> RenderDevice_GL::CreateRenderPass(RenderPassType& a_Type, RenderPipeline& a_Pipeline)
    {
        switch(a_Type)
        {
            case RenderPassType::RP_HELLOTRIANGLE:
                return std::make_shared<RenderPass_HelloTriangle_GL>(a_Pipeline);
            break;
        default:
            throw std::exception("RenderPassType not implemented for OpenGL!");
            break;
        }
    }

    std::shared_ptr<RenderPipeline> RenderDevice_GL::CreatePipeline()
    {
        return std::make_shared<RenderPipeline_GL>(m_Engine, *this);
    }
}
