#pragma once
#include <memory>
#include <vector>


#include "RenderResource.h"

namespace blurp
{
    class BlurpEngine;
    class RenderDevice;

    //Structs to construct resources forward declarations.
    struct BlurpSettings;
    struct WindowSettings;
    struct CameraSettings;
    struct TextureSettings;
    struct MeshSettings;
    struct LightSettings;
    struct RenderTargetSettings;
    struct SwapChainSettings;
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
    class Shader;
    class GpuBuffer;

    class RenderPass;
    class RenderPipeline;

    //Forward declared enums.
    enum class RenderPassType;

    /*
     * RenderResourceManager is the only class that can construct instances of RenderResource.
     * All instances of RenderResource are tracked as shared_ptr.
     *
     * When no longer used, CleanUpUnused will unload them properly.
     */
    class RenderResourceManager
    {
    public:

        RenderResourceManager(BlurpEngine& a_Engine, RenderDevice& a_Device);

        ~RenderResourceManager();

        /*
         * Clean up any unused resources.
         */
        void CleanUpUnused();

        //Creation methods.
    public:

        /*
         * Create a resource of this type specific to the active graphics API.
         */
        std::shared_ptr<Light> CreateLight(const LightSettings& a_Settings);

        /*
         * Create a resource of this type specific to the active graphics API.
         */
        std::shared_ptr<Camera> CreateCamera(const CameraSettings& a_Settings);

        /*
         * Create a resource of this type specific to the active graphics API.
         */
        std::shared_ptr<Mesh> CreateMesh(const MeshSettings& a_Settings);

        /*
         * Create a resource of this type specific to the active graphics API.
         */
        std::shared_ptr<Texture> CreateTexture(const TextureSettings& a_Settings);

        /*
         * Create a resource of this type specific to the active graphics API.
         */
        std::shared_ptr<RenderTarget> CreateRenderTarget(const RenderTargetSettings& a_Settings);

        /*
         * Create a resource of this type specific to the active graphics API.
         */
        std::shared_ptr<SwapChain> CreateSwapChain(const WindowSettings& a_Settings);

        /*
         * Create a resource of this type specific to the active graphics API.
         */
        std::shared_ptr<Material> CreateMaterial(const MaterialSettings& a_Settings);

        /*
         * Create a render pass from the given type.
         */
        std::shared_ptr<RenderPass> CreateRenderPass(RenderPassType& a_Type, RenderPipeline& a_Pipeline);

        /*
         * Create a render pipeline.
         */
        std::shared_ptr<RenderPipeline> CreatePipeline(const PipelineSettings& a_Settings);

        /*
         * Create a shader from the given settings.
         */
        std::shared_ptr<Shader> CreateShader(const ShaderSettings& a_Settings);

        /*
         * Create a GpuBuffer from the given settings.
         */
        std::shared_ptr<GpuBuffer> CreateGpuBuffer(const GpuBufferSettings& a_Settings);

    private:
        std::vector<std::shared_ptr<RenderResource>> m_Resources;
        RenderDevice& m_RenderDevice;
        BlurpEngine& m_Engine;
    };
}
