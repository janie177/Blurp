#pragma once
#include "RenderPass_Clear.h"

namespace blurp
{
    class RenderPass_Clear_GL : public RenderPass_Clear
    {
    public:
        explicit RenderPass_Clear_GL(RenderPipeline& a_Pipeline)
            : RenderPass_Clear(a_Pipeline)
        {
        }

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;
        void Execute() override;

    };
}