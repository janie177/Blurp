#pragma once
#include "RenderPass_HelloTriangle.h"

namespace blurp
{
    class RenderPass_HelloTriangle_GL : public RenderPass_HelloTriangle
    {
    public:
        RenderPass_HelloTriangle_GL(RenderPipeline& a_Pipeline) : RenderPass_HelloTriangle(a_Pipeline) {}

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;
        void Execute() override;
    };
}
