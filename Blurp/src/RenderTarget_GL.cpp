#include "opengl/RenderTarget_GL.h"



#include <glm/common.hpp>
#include <glm/common.hpp>
#include <glm/common.hpp>
#include <glm/common.hpp>
#include <glm/common.hpp>
#include <glm/common.hpp>
#include <glm/common.hpp>
#include <glm/common.hpp>
#include <glm/common.hpp>
#include <glm/common.hpp>

namespace blurp
{
    GLint RenderTarget_GL::GetFrameBufferID()
    {
        return m_FBO;
    }

    bool RenderTarget_GL::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        //Default framebuffer doesn't need special setup. Just ID 0 works.
        if(m_IsDefault)
        {
            m_FBO = 0;
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

    bool RenderTarget_GL::GetNumColorAttachments()
    {
        //TODO
        return true;
    }

    bool RenderTarget_GL::HasColorAttachment()
    {
        //TODO
        return true;
    }

    bool RenderTarget_GL::HasDepthAttachment()
    {
        //TODO
        return true;
    }

    bool RenderTarget_GL::HasStencilAttachment()
    {
        //TODO
        return true;
    }

    glm::vec4 RenderTarget_GL::GetClearColor()
    {
        //TODO
        return glm::vec4();
    }

    void RenderTarget_GL::SetClearColor(const glm::vec4& a_ClearColor)
    {
        //TODO
    }

    glm::vec4 RenderTarget_GL::GetViewPort()
    {
        //TODO
        return glm::vec4();
    }

    void RenderTarget_GL::SetViewPort(const glm::vec4& a_ViewPort)
    {
        //TODO
    }

    glm::vec4 RenderTarget_GL::GetScissorRect()
    {
        //TODO
        return glm::vec4();
    }

    void RenderTarget_GL::SetScissorRect(const glm::vec4& a_ScissorRect)
    {
        //TODO
    }

    std::shared_ptr<Texture> RenderTarget_GL::GetColorAttachment()
    {
        //TODO
        return nullptr;
    }

    std::shared_ptr<Texture> RenderTarget_GL::GetDepthStencilAttachment()
    {
        //TODO
        return nullptr;
    }
}
