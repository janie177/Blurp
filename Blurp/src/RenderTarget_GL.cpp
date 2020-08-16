#include "opengl/RenderTarget_GL.h"



#include <glm/common.hpp>
#include <glm/common.hpp>
#include <glm/common.hpp>
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <iostream>

#include "BlurpEngine.h"
#include "opengl/Texture_GL.h"
#include "opengl/GLUtils.h"


namespace blurp
{
    GLint RenderTarget_GL::GetFrameBufferId() const
    {
        return m_Fbo;
    }

    bool RenderTarget_GL::IsDefaultGlTarget() const
    {
        return m_IsDefault;
    }

    bool RenderTarget_GL::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        //Set the settings objects.

        assert(m_Settings.scissorRect.z > 0 && m_Settings.scissorRect.w > 0 && "Scissorrect needs positive dimensions!");
        assert(m_Settings.viewPort.z > 0 && m_Settings.viewPort.w > 0 && "Viewport needs positive dimensions!");

        m_ClearColor = m_Settings.clearColor;
        m_ScissorRect = m_Settings.scissorRect;
        m_ViewPort = m_Settings.viewPort;

        //Default framebuffer doesn't need special setup. Just ID 0 works.
        if (m_IsDefault)
        {
            m_Fbo = 0;
        }
        //Non default. Set it all up.
        else
        {
            glGenFramebuffers(1, &m_Fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo);

            //Attach defaults.

            if (m_Settings.defaultColorAttachment != nullptr)
            {
                SetColorAttachment(0, m_Settings.defaultColorAttachment);
            }

            if (m_Settings.defaultDepthStencilAttachment != nullptr)
            {
                SetDepthStencilAttachment(m_Settings.defaultDepthStencilAttachment);
            }

            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                std::cout << "Warning: FrameBuffer created but not yet complete. This is not a problem if attachments are later added." << std::endl;
            }

            //If read only, now set the variable that ensures no attachments can be added.
            //This has to be done after adding the defaults for obvious reasons.
            m_AllowAttachments = m_Settings.allowAttachments;

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        return true;
    }

    bool RenderTarget_GL::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        if(!m_IsDefault)
        {
            glDeleteFramebuffers(1, &m_Fbo);
        }
        return true;
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

    void RenderTarget_GL::SetViewPort(const glm::vec<4, std::uint32_t>& a_ViewPort)
    {
        m_ViewPort = a_ViewPort;
    }

    glm::vec4 RenderTarget_GL::GetScissorRect()
    {
        return m_ScissorRect;
    }

    void RenderTarget_GL::SetScissorRect(const glm::vec<4, std::uint32_t>& a_ScissorRect)
    {
        m_ScissorRect = a_ScissorRect;
    }

    void RenderTarget_GL::OnColorAttachmentBound(std::uint16_t a_Slot, const std::shared_ptr<Texture>& a_Added)
    {
        const auto textureType = ToGL(a_Added->GetTextureType());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + a_Slot, textureType, std::reinterpret_pointer_cast<Texture_GL>(a_Added)->GetTextureId(), 0);
    }

    void RenderTarget_GL::OnDepthStencilAttachmentBound(const std::shared_ptr<Texture>& a_Added)
    {
        const auto textureType = ToGL(a_Added->GetTextureType());

        GLenum attachmentPoint;

        if(a_Added->GetPixelFormat() == PixelFormat::DEPTH_STENCIL)
        {
            attachmentPoint = GL_DEPTH_STENCIL_ATTACHMENT;
        }
        else
        {
            attachmentPoint = GL_DEPTH_ATTACHMENT;
        }
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentPoint, textureType, std::reinterpret_pointer_cast<Texture_GL>(a_Added)->GetTextureId(), 0);
    }
}
