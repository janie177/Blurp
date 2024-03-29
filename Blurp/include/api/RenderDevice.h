#pragma once
#include <memory>


#include "RenderResource.h"

namespace blurp
{
    //Structs to construct resources forward declarations.
    struct BlurpSettings;
    struct WindowSettings;
    struct CameraSettings;
    struct TextureSettings;
    struct MeshSettings;
    struct LightSettings;
    struct RenderTargetSettings;
    struct SwapChainSettings;
    struct MaterialBatchSettings;
    struct MaterialSettings;
    struct ShaderSettings;
    struct GpuBufferSettings;

    //Resources forward declarations.
    class Window;
    class Light;
    class Camera;
    class Texture;
    class Mesh;
    class RenderTarget;
    class SwapChain;
    class Material;
    class MaterialBatch;
    class Shader;
    class GpuBuffer;

    class RenderPass;
    class RenderPipeline;

    //Forward declared enums.
    enum class RenderPassType;

    class RenderDevice
    {
        friend class BlurpEngine;
        friend class RenderResourceManager;
    public:
        virtual ~RenderDevice() = default;

        RenderDevice(BlurpEngine& a_Engine) : m_Engine(a_Engine) {}

        //These functions are protected so that only BlurpEngine and the resource manager can call them.
        //These create resources but don't properly load them. Available functions for loading resources
        //can be found in the RenderResourceManager class.
    protected:
        /*
         * Initialize the rendering context and setup any required systems.
         * This includes setting up a swap chain for the provided window if not null.
         */
        virtual bool Init(BlurpEngine& a_BlurpEngine, const WindowSettings& a_WindowSettings) = 0;

        /*
         * Create a resource of this type specific to the active graphics API.
         */
        virtual std::shared_ptr<Light> CreateLight(const LightSettings& a_Settings) = 0;

        /*
         * Create a resource of this type specific to the active graphics API.
         */
        virtual std::shared_ptr<Camera> CreateCamera(const CameraSettings& a_Settings) = 0;

        /*
         * Create a resource of this type specific to the active graphics API.
         */
        virtual std::shared_ptr<Mesh> CreateMesh(const MeshSettings& a_Settings) = 0;

        /*
         * Create a resource of this type specific to the active graphics API.
         */
        virtual std::shared_ptr<Texture> CreateTexture(const TextureSettings& a_Settings) = 0;

        /*
         * Create a resource of this type specific to the active graphics API.
         */
        virtual std::shared_ptr<RenderTarget> CreateRenderTarget(const RenderTargetSettings& a_Settings) = 0;

        /*
         * Create a resource of this type specific to the active graphics API.
         */
        virtual std::shared_ptr<SwapChain> CreateSwapChain(const WindowSettings& a_Settings) = 0;

        /*
         * Create a resource of this type specific to the active graphics API.
         */
        virtual std::shared_ptr<Material> CreateMaterial(const MaterialSettings& a_Settings) = 0;

        /*
         * Create a resource of this type specific to the active graphics API.
         */
        virtual std::shared_ptr<MaterialBatch> CreateMaterialBatch(const MaterialBatchSettings& a_Settings) = 0;

        /*
         * Create a render pass from the given type.
         */
        virtual std::shared_ptr<RenderPass> CreateRenderPass(RenderPassType& a_Type, RenderPipeline& a_Pipeline) = 0;

        /*
         * Create a render pipeline.
         */
        virtual std::shared_ptr<RenderPipeline> CreatePipeline(const PipelineSettings& a_Settings) = 0;

        /*
         * Create a shader from the given settings.
         */
        virtual std::shared_ptr<Shader> CreateShader(const ShaderSettings& a_Settings) = 0;

        /*
         * Create a GpuBuffer from the given settings.
         */
        virtual std::shared_ptr<GpuBuffer> CreateGpuBuffer(const GpuBufferSettings& a_Settings) = 0;

    protected:
        /*
         * Bind a Window and SwapChain together.
         * This loads the SwapChain and initializes it with the window.
         * This gives the window shared ownership of the SwapChain.
         */
        void BindWindowAndSwapChain(BlurpEngine& a_BlurpEngine, Window* a_Window, std::shared_ptr<SwapChain> a_SwapChain) const;

    protected:
        BlurpEngine& m_Engine;
    };
}
