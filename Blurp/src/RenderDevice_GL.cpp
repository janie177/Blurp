#include "opengl/RenderDevice_GL.h"

#include "opengl/RenderTarget_GL.h"
#include "BlurpEngine.h"
#include "opengl/Mesh_GL.h"
#include "opengl/RenderPass_HelloTriangle_GL.h"
#include "opengl/RenderPipeline_GL.h"
#include "opengl/Shader_GL.h"
#include "opengl/SwapChain_GL_Win32.h"
#include "opengl/Texture_GL.h"
#include "Camera.h"
#include "Light.h"
#include "opengl/GpuBuffer_GL.h"
#include "opengl/MaterialBatch_GL.h"
#include "opengl/RenderPass_Forward_GL.h"
#include "Material.h"

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

        //Enable debugging
#ifdef _DEBUG
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, 0);
#endif

        return true;
    }

    std::shared_ptr<Light> RenderDevice_GL::CreateLight(const LightSettings& a_Settings)
    {
        switch (a_Settings.type)
        {
        case LightType::LIGHT_AMBIENT:
            return std::make_shared<Light>(a_Settings);
        case LightType::LIGHT_DIRECTIONAL:
            return std::make_shared<DirectionalLight>(a_Settings);
        case LightType::LIGHT_POINT:
            return std::make_shared<PointLight>(a_Settings);
        case LightType::LIGHT_SPOT:
            return std::make_shared<SpotLight>(a_Settings);
        };

        throw std::exception("Unsupported light type!");
    }

    std::shared_ptr<Camera> RenderDevice_GL::CreateCamera(const CameraSettings& a_Settings)
    {
        return std::make_shared<Camera>(a_Settings);
    }

    std::shared_ptr<Mesh> RenderDevice_GL::CreateMesh(const MeshSettings& a_Settings)
    {
        return std::make_shared<Mesh_GL>(a_Settings);
    }

    std::shared_ptr<Texture> RenderDevice_GL::CreateTexture(const TextureSettings& a_Settings)
    {
        //Return the texture class that internally sets up according to type.
        return std::make_shared<Texture_GL>(a_Settings);
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
        {
            throw std::exception("Swap chain for given window not implemented for OpenGL!");
            return nullptr;
        }
        }
        
    }

    std::shared_ptr<Material> RenderDevice_GL::CreateMaterial(const MaterialSettings& a_Settings)
    {
        return std::make_shared<Material>(a_Settings);
    }

    std::shared_ptr<RenderPass> RenderDevice_GL::CreateRenderPass(RenderPassType& a_Type, RenderPipeline& a_Pipeline)
    {
        switch(a_Type)
        {
            case RenderPassType::RP_HELLOTRIANGLE:
                return std::make_shared<RenderPass_HelloTriangle_GL>(a_Pipeline);
            case RenderPassType::RP_FORWARD:
                return std::make_shared<RenderPass_Forward_GL>(a_Pipeline);
        default:
            throw std::exception("RenderPassType not implemented for OpenGL!");
            break;
        }
    }

    std::shared_ptr<RenderPipeline> RenderDevice_GL::CreatePipeline(const PipelineSettings& a_Settings)
    {
        return std::make_shared<RenderPipeline_GL>(a_Settings, m_Engine, *this);
    }

    std::shared_ptr<Shader> RenderDevice_GL::CreateShader(const ShaderSettings& a_Settings)
    {
        return std::make_shared<Shader_GL>(a_Settings);
    }

    std::shared_ptr<GpuBuffer> RenderDevice_GL::CreateGpuBuffer(const GpuBufferSettings& a_Settings)
    {
        return std::make_shared<GpuBuffer_GL>(a_Settings);
    }

    std::shared_ptr<MaterialBatch> RenderDevice_GL::CreateMaterialBatch(const MaterialBatchSettings& a_Settings)
    {
        return std::make_shared<MaterialBatch_GL>(a_Settings);
    }

    void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
    {
        fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
    }
}
