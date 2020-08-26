#pragma once
#include <GL/glew.h>

#include "RenderPass_ShadowMap.h"
#include "ShaderCache.h"

#define MAX_NUM_LIGHTS 64

namespace blurp
{
    struct PosLightData
    {
        glm::mat4 matrices[6];
        glm::vec<4, std::int32_t> shadowMapIndex;   //3 Bytes padding.
    };

    class RenderPass_ShadowMap_GL : public RenderPass_ShadowMap
    {
    public:
        explicit RenderPass_ShadowMap_GL(RenderPipeline& a_Pipeline)
            : RenderPass_ShadowMap(a_Pipeline), m_Fbo(0), m_PosLightUbo(0)
        {
        }

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;
        void Execute() override;

    private:
        GLuint m_Fbo;
        GLuint m_PosLightUbo;
        GLuint m_LightIndicesUbo;
        ShaderCache<std::uint32_t, std::uint32_t> m_ShaderCache;
    };
}
