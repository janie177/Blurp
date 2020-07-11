#include "opengl/RenderTarget_GL.h"
#include <glm/glm.hpp>


namespace blurp
{
    GLint RenderTarget_GL::GetFrameBufferID()
    {
        return m_Fbo;
    }

    bool RenderTarget_GL::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        //Default framebuffer doesn't need special setup. Just ID 0 works.
        if(m_IsDefault)
        {
            m_Fbo = 0;
        }
        //Non default. Set it all up.
        else
        {
            //TODO set up an FBO object.
        }
        return true;
    }

    bool RenderTarget_GL::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        if(!m_IsDefault)
        {
            //TODO destroy here because not default.
        }
        return true;
    }

    std::uint16_t RenderTarget_GL::GetNumColorAttachments()
    {
        //TODO always one for now unless not present. In the future this may be more than one.
        return HasColorAttachment() ? 1 : 0;
    }

    bool RenderTarget_GL::HasColorAttachment()
    {
        return m_ColorAttachment != nullptr || m_IsDefault;
    }

    bool RenderTarget_GL::HasDepthAttachment()
    {
        return m_DepthStencilAttachment != nullptr;
    }

    bool RenderTarget_GL::HasStencilAttachment()
    {
        return m_Settings.depthStencilSettings.format == PixelFormat::DEPTH_STENCIL;
    }

    glm::vec4 RenderTarget_GL::GetClearColor()
    {
        return m_ClearColor;
    }

    void RenderTarget_GL::SetClearColor(const glm::vec4& a_ClearColor)
    {
        m_ClearColor = a_ClearColor;
    }

    glm::vec4 RenderTarget_GL::GetViewPort()
    {
        return m_ViewPort;
    }

    void RenderTarget_GL::SetViewPort(const glm::vec4& a_ViewPort)
    {
        m_ViewPort = a_ViewPort;
    }

    glm::vec4 RenderTarget_GL::GetScissorRect()
    {
        return m_ScissorRect;
    }

    void RenderTarget_GL::SetScissorRect(const glm::vec4& a_ScissorRect)
    {
        m_ScissorRect = a_ScissorRect;
    }

    std::shared_ptr<Texture> RenderTarget_GL::GetColorAttachment()
    {
        return m_ColorAttachment;
    }

    std::shared_ptr<Texture> RenderTarget_GL::GetDepthStencilAttachment()
    {
        return m_DepthStencilAttachment;
    }
}
