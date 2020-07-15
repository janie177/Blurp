#pragma once

#include "RenderPass_Forward.h"

namespace blurp
{
    class RenderPass_Forward_GL : public RenderPass_Forward
    {
    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;
        void Execute() override;
    };
}