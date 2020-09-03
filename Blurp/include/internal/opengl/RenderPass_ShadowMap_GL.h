#pragma once
#include <GL/glew.h>

#include "RenderPass_ShadowMap.h"
#include "ShaderCache.h"

#define MAX_NUM_LIGHTS 64

namespace blurp
{
    struct PosLightData
    {
        glm::vec4 lightPosition;
        glm::mat4 matrices[6];
        glm::vec<4, std::int32_t> shadowMapIndex;   //3 * 4 Bytes padding.
    };

    struct DirLightData
    {
        glm::ivec4 numCascades;                     //The amount of cascades stored in X.
        glm::vec4 camPosCascadeDistance;            //Cam pos and the distance per cascade.
        glm::ivec4 shadowIndices[MAX_NUM_LIGHTS];   //Only X is used.
    };

    class RenderPass_ShadowMap_GL : public RenderPass_ShadowMap
    {
    public:
        explicit RenderPass_ShadowMap_GL(RenderPipeline& a_Pipeline)
            : RenderPass_ShadowMap(a_Pipeline), m_Fbo(0), m_LightUbo(0)
        {
        }

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;
        void Execute() override;

    private:
        GLuint m_Fbo;
        GLuint m_LightUbo;          //Used for both pos and dir lights. Data overwritten and interpreted differently in the shader.
        GLuint m_LightIndicesUbo;   //Used for both lights, data overwritten between outputs.
        ShaderCache<std::uint32_t, std::uint32_t> m_ShaderCache;
    };
}
