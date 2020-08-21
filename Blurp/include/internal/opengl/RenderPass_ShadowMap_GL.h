#pragma once
#include <GL/glew.h>

#include "RenderPass_ShadowMap.h"
#include "ShaderCache.h"

namespace blurp
{
    class RenderPass_ShadowMap_GL : public RenderPass_ShadowMap
    {
    public:
        explicit RenderPass_ShadowMap_GL(RenderPipeline& a_Pipeline)
            : RenderPass_ShadowMap(a_Pipeline), m_Fbo(0)
        {
        }

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;
        void Execute() override;

    private:
        GLuint m_Fbo;
        ShaderCache<std::uint32_t, std::uint32_t> m_ShaderCache;
    };
}
