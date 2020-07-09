#pragma once
#include <glm/glm.hpp>
#include <memory>

#include "RenderResource.h"

namespace blurp
{
    struct SwapChainSettings;
    class RenderTarget;

    class SwapChain : public RenderResource
    {
    public:
        virtual ~SwapChain() = default;
        SwapChain(const SwapChainSettings& a_Settings) : m_Settings(a_Settings) {}

        /*
         * Called when the swapchain needs to resize.
         */
        virtual void Resize(const glm::vec2& a_Dimensions, bool a_FullScreen) = 0;

        /*
         * Get the amount of buffers in this swapchain.
         */
        virtual std::uint16_t GetNumBuffers() = 0;

        /*
         * Get the render target that can currently be written to.
         */
        virtual std::shared_ptr<RenderTarget> GetRenderTarget() = 0;

        /*
         * Swap the buffers to present the next frame.
         */
        virtual void Present() = 0;

    protected:
        SwapChainSettings m_Settings;
    };
}