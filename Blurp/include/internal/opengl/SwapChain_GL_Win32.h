#pragma once
#include "Window.h"
#include "SwapChain.h"
#include <windows.h>

#include "RenderTarget_GL.h"

namespace blurp
{
    class SwapChain_GL_Win32 : public SwapChain
    {
    public:
        SwapChain_GL_Win32(const SwapChainSettings& a_Settings) : SwapChain(a_Settings), m_GlContext(nullptr),
                                                                  m_Hdc(nullptr)
        {
        }

        void Resize(const glm::vec2& a_Dimensions, bool a_FullScreen) override;

        std::uint16_t GetNumBuffers() override;

        std::shared_ptr<RenderTarget> GetRenderTarget() override;

        void Present() override;
    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;

    private:
        std::shared_ptr<RenderTarget_GL> m_RenderTarget;
        HGLRC m_GlContext;
        HDC m_Hdc;
    };
}
