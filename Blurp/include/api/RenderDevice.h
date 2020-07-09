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
    struct MaterialSettings;

    //Resources forward declarations.
    class Window;
    class Light;
    class Camera;
    class Texture;
    class Mesh;
    class RenderTarget;
    class SwapChain;
    class Material;

    //Forward declared enums.
    enum class RenderPassType;

    class RenderDevice
    {
    public:
        virtual ~RenderDevice() = default;

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
         * Create a render pass from the given type.
         */
        //TODO
        virtual void CreateRenderPass(RenderPassType& a_Type) = 0;

        /*
         * Create a pipeline.
         */
        //TODO
        virtual void CreatePipeline() = 0;

        /*
         * Bind a Window and SwapChain together.
         * This loads the swapchain and inits it with the window.
         * This gives the window shared ownership of the swapchain.
         */
        void BindWindowAndSwapChain(BlurpEngine& a_BlurpEngine, Window* a_Window, std::shared_ptr<SwapChain> a_SwapChain) const;
    };
}
