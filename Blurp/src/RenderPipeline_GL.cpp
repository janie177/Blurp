#include "opengl/RenderPipeline_GL.h"

namespace blurp
{
    bool RenderPipeline_GL::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        return true;
    }

    bool RenderPipeline_GL::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        return true;
    }

    bool RenderPipeline_GL::HasFinishedExecuting()
    {
        return true;
    }

    void RenderPipeline_GL::PreExecute()
    {
    }

    void RenderPipeline_GL::PostExecute()
    {
    }
}
