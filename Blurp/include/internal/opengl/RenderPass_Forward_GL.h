#pragma once

#include <GL/glew.h>

#include "RenderPass_Forward.h"
#include "ShaderCache.h"
#include <stdint.h>

//Maximum amount of lights that will fit in the GPU buffer. 65KB for UBO, which will fit about 580 tightly packed structs of LightDataPacked.
#define MAX_LIGHTS 580

namespace blurp
{
    struct StaticData
    {
        glm::mat4 pv;
        glm::vec4 camPosFarPlane;
        glm::vec4 numLights;        //X = numPointLight. Y = numSpotLights. Z = numDirectionalLights.
        glm::vec4 numShadows;       //X = numPointShadows. Y = numSpotShadows. Z = numDirectionalShadows.
        glm::vec4 ambientLight;     //The ambient light RGB.
    };

    /*
     * Struct that is laid out to comply with UBO rules with correct padding and alignment.
     * Enough data is present to contain any light type.
     *
     * Depending on the light type, the GPU will interpret these values differently.
     */
    struct LightDataPacked
    {
        glm::vec4 vec1;
        glm::vec4 vec2;
        glm::vec4 vec3;
        glm::mat4 shadowMatrix;
    };

    class RenderPass_Forward_GL : public RenderPass_Forward
    {
    public:
        RenderPass_Forward_GL(RenderPipeline& a_Pipeline)
            : RenderPass_Forward(a_Pipeline), m_StaticDataUbo(0), m_LightUbo(0), m_ShadowSampler(0)
        {
        }

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;
        void Execute() override;

    private:
        //Shader cache that compiles shaders dynamically based on required attributes.
        ShaderCache<std::uint64_t, std::uint64_t> m_ShaderCache;

        //UBO used to upload data that is persistent for the entire frame. For example: camera matrices and light amount.
        GLuint m_StaticDataUbo;

        //UBOs used for light uploading and light transforming.
        GLuint m_LightUbo;

        //Shadow sampler objects with texture comparisons enabled.
        GLuint m_ShadowSampler;
    };
}