#include "opengl/RenderTarget_GL.h"

namespace blurp
{
    GLint RenderTarget_GL::GetFrameBufferID()
    {
        return m_FBO;
    }

    bool RenderTarget_GL::Load(BlurpEngine& a_BlurpEngine)
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

    bool RenderTarget_GL::Destroy(BlurpEngine& a_BlurpEngine)
    {
        if(!m_IsDefault)
        {
            //TODO destroy here because not default.
        }
        return true;
    }
}
