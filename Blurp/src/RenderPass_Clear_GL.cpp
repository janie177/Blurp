#pragma once

#include "opengl/RenderPass_Clear_GL.h"
#include "opengl/GLUtils.h"
#include "opengl/RenderTarget_GL.h"
#include "opengl/Texture_GL.h"

namespace blurp
{
    bool RenderPass_Clear_GL::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        //Nothing here because glClear is a thing.
        return true;
    }

    bool RenderPass_Clear_GL::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        //Nothing needed.
        return true;
    }

    void RenderPass_Clear_GL::Execute()
    {
        for(auto& tex : m_Textures)
        {
            auto glTex = static_cast<Texture_GL*>(tex.first.get());
            glClearTexImage(glTex->GetTextureId(), 0, ToGL(glTex->GetPixelFormat()), ToGL(glTex->GetDataType()), &tex.second);
        }

        for(auto& rt : m_RenderTargets)
        {
            auto glTarget = static_cast<RenderTarget_GL*>(rt.get());
            glTarget->Bind();

            GLenum clearBit = 0;
            if(glTarget->GetNumColorAttachments() != 0)
            {
                clearBit = GL_COLOR_BUFFER_BIT;
            }
            if(glTarget->HasStencilAttachment())
            {
                clearBit |= GL_STENCIL_BUFFER_BIT;
            }
            if (glTarget->HasDepthAttachment())
            {
                clearBit |= GL_DEPTH_BUFFER_BIT;
            }

            //Only clear if there is attachments.
            if(clearBit != 0)
            {
                glClear(clearBit);
            }
        }
    }
}
