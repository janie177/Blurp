#pragma once
#include <GL/glew.h>
#include "RenderTarget.h"

namespace blurp
{
    class RenderTarget_GL : public RenderTarget
    {
    public:
        /*
         * Create a OpenGL Render Target.
         * When a_DefaultFrameBuffer is set to true, this acts as a dummy object that binds to default frame buffer created
         * by OpenGL itself.
         */
        RenderTarget_GL(const RenderTargetSettings& a_Settings, bool a_DefaultFrameBuffer) : RenderTarget(a_Settings),
                                                                                             m_FBO(0),
                                                                                             m_IsDefault(
                                                                                                 a_DefaultFrameBuffer)
        {
        }

        GLint GetFrameBufferID();

    protected:
        bool Load(BlurpEngine& a_BlurpEngine) override;
        bool Destroy(BlurpEngine& a_BlurpEngine) override;

    private:
        GLuint m_FBO;
        bool m_IsDefault;
    };
}
