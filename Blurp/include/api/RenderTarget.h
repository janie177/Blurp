#pragma once
#include "RenderResource.h"

namespace blurp
{
    /*
     * RenderTarget is something that can be rendered into by the shader pipeline.
     * In OpenGL, this is implemented through the use of FrameBufferObjects.
     * In Direct3D12, RenderTargetViews are used and bound to the command lists.
     */
    class RenderTarget : public RenderResource
    {

    };
}
