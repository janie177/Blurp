#include "opengl/RenderPass_HelloTriangle_GL.h"

namespace blurp
{
    bool RenderPass_HelloTriangle_GL::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        //TODO setup a rendering pipeline for triangle in OpenGL.
        return true;
    }

    bool RenderPass_HelloTriangle_GL::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        //TODO Destroy the triangle pipeline.
        return true;
    }

    void RenderPass_HelloTriangle_GL::Execute()
    {
        //TODO run the triangle pipeline.
        //This sets ALL state required to do this drawing.
    }
}
