#pragma once
#include "RenderDevice.h"

namespace blurp
{
    class BlurpEngine;

    /*
     * Sets up OpenGL context and 
     */
    class RenderDevice_GL : public RenderDevice
    {
    public:
        RenderDevice_GL(BlurpEngine& a_Engine) : RenderDevice(a_Engine) {}

    protected:
        bool Init(BlurpEngine& a_BlurpEngine, const WindowSettings& a_WindowSettings) override;

        std::shared_ptr<Light> CreateLight(const LightSettings& a_Settings) override;
        std::shared_ptr<Camera> CreateCamera(const CameraSettings& a_Settings) override;
        std::shared_ptr<Mesh> CreateMesh(const MeshSettings& a_Settings) override;
        std::shared_ptr<Texture> CreateTexture(const TextureSettings& a_Settings) override;
        std::shared_ptr<RenderTarget> CreateRenderTarget(const RenderTargetSettings& a_Settings) override;
        std::shared_ptr<SwapChain> CreateSwapChain(const WindowSettings& a_Settings) override;
        std::shared_ptr<Material> CreateMaterial(const MaterialSettings& a_Settings) override;
        std::shared_ptr<RenderPass> CreateRenderPass(RenderPassType& a_Type, RenderPipeline& a_Pipeline) override;
        std::shared_ptr<RenderPipeline> CreatePipeline() override;
    };
}
