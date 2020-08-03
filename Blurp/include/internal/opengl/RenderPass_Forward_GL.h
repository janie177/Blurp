#pragma once

#include "RenderPass_Forward.h"
#include "ShaderCache.h"
#include "Settings.h"

namespace blurp
{
    class RenderPass_Forward_GL : public RenderPass_Forward
    {
    public:
        RenderPass_Forward_GL(RenderPipeline& a_Pipeline)
            : RenderPass_Forward(a_Pipeline)
        {
        }

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;
        void Execute() override;

    private:
        //Shader cache that compiles shaders dynamically based on required attributes.
        ShaderCache<VertexAttribute, std::uint16_t> m_ShaderCache;
    };
}