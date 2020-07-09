#pragma once
#include "RenderResource.h"
#include "Settings.h"

namespace blurp
{

    /*
     * RenderTarget is something that can be rendered into by the shader pipeline.
     * In OpenGL, this is implemented through the use of FrameBufferObjects.
     * In Direct3D12, RenderTargetViews are used and bound to the command lists.
     */
    class RenderTarget : public RenderResource
    {
    public:
        RenderTarget(const RenderTargetSettings& a_Settings) : m_Settings(a_Settings){}

        //TODO make it so that a render target can only be bound to a single pipeline at the same time.
        //Add some sort of locking mechanism.
        //
        ////TODO Make render target control the viewport and scissor rects.
        
    protected:
        RenderTargetSettings m_Settings;
    };
}
