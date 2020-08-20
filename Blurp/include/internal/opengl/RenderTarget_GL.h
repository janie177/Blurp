#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "RenderTarget.h"

namespace blurp
{
    class RenderTarget_GL : public RenderTarget
    {
        friend class SwapChain_GL_Win32;
    public:
        /*
         * Create a OpenGL Render Target.
         * When a_DefaultFrameBuffer is set to true, this acts as a dummy object that binds to default frame buffer created
         * by OpenGL itself.
         */
        RenderTarget_GL(const RenderTargetSettings& a_Settings, bool a_DefaultFrameBuffer, bool a_HasDepth, bool a_HasStencil) : RenderTarget(a_Settings),
                                                                                            m_Fbo(0),
                                                                                            m_IsDefault(a_DefaultFrameBuffer), m_HasDefaultDepth(a_HasDepth), m_HasDefaultStencil(a_HasStencil)
        {

        }

        RenderTarget_GL(const RenderTargetSettings& a_Settings) : RenderTarget(a_Settings), m_Fbo(0), m_IsDefault(false), m_HasDefaultDepth(false), m_HasDefaultStencil(false) {}

        /*
         * Get the FBO id of this FrameBuffer.
         */
        GLint GetFrameBufferId() const;

        /*
         * Returns true if this FrameBuffer is the default OpenGL created FrameBuffer.
         * This means that it is owned and maintained by the window and OS.
         * Textures and attachments cannot be used in those cases.
         */
        bool IsDefaultGlTarget() const;

        /*
         * Bind the framebuffer as render target to the OpenGL state.
         */
        void Bind();

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;

    public:
        glm::vec4 GetClearColor() override;
        void SetClearColor(const glm::vec4& a_ClearColor) override;
        glm::vec4 GetViewPort() override;
        void SetViewPort(const glm::vec<4, std::uint32_t>& a_ViewPort) override;
        glm::vec4 GetScissorRect() override;
        void SetScissorRect(const glm::vec<4, std::uint32_t>& a_ScissorRect) override;
        void OnColorAttachmentBound(std::uint16_t a_Slot, const std::shared_ptr<Texture>& a_Added) override;
        void OnDepthStencilAttachmentBound(const std::shared_ptr<Texture>& a_Added) override;

        //Overridden because OpenGL has the default framebuffer.

        bool HasDepthAttachment() const override;
        bool HasStencilAttachment() const override;
    private:
        GLuint m_Fbo;

        //The following only applies to the default OpenGL framebuffer.
        bool m_IsDefault;
        bool m_HasDefaultDepth;
        bool m_HasDefaultStencil;

        glm::vec4 m_ClearColor;
        glm::vec4 m_ViewPort;
        glm::vec4 m_ScissorRect;
    };
}
