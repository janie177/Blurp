#pragma once

#include "opengl/RenderPass_Clear_GL.h"

#include <iostream>

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
            auto& clearData = tex.second;

            const auto dataType = ToGL(glTex->GetDataType());
            const auto format = ToGL(glTex->GetPixelFormat());

            //Clear the texture. This works for any dimension of texture because the parameters are left at 0 when they don't apply.
            glClearTexSubImage(glTex->GetTextureId(), 0, static_cast<GLsizei>(clearData.offset.x), static_cast<GLsizei>(clearData.offset.y), static_cast<GLsizei>(clearData.offset.z), static_cast<GLsizei>(clearData.size.x), static_cast<GLsizei>(clearData.size.y), static_cast<GLsizei>(clearData.size.z), format, dataType, &clearData.clearValue);
        }

        for(auto& rt : m_RenderTargets)
        {
            auto glTarget = static_cast<RenderTarget_GL*>(rt.get());
            glTarget->Bind();

            GLenum clearBit = 0;
            if(glTarget->HasColorAttachment() != 0)
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
