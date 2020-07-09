#pragma once

namespace blurp
{
    class Window;
    enum class RenderPassType;
    struct SwapChainSettings;
    struct RenderTargetSettings;
    struct MeshSettings;
    struct TextureSettings;

    class RenderDevice
    {
    public:
        /*
         * Initialize the rendering context and setup any required systems.
         */
        virtual void Init() = 0;

        //TODO make types and replace void

        /*
         * Create a Texture from the given settings.
         */
        virtual void CreateTexture(TextureSettings& a_Settings) = 0;

        /*
         * Create a mesh from the given settings.
         */
        virtual void CreateMesh(MeshSettings& a_Settings) = 0;

        /*
         * Create a render target from the given settings.
         */
        virtual void CreateRenderTarget(RenderTargetSettings& a_Settings) = 0;

        /*
         * Create a swap chain from the given settings for the specified window.
         */
        virtual void CreateSwapChain(SwapChainSettings& a_Settings, Window* a_Window) = 0;

        /*
         * Create a render pass from the given type.
         */
        virtual void CreateRenderPass(RenderPassType& a_Type) = 0;

        /*
         * Create a pipeline.
         */
        virtual void CreatePipeline() = 0;
    };
}