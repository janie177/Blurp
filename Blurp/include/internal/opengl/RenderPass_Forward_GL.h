#pragma once

#include <GL/glew.h>

#include "RenderPass_Forward.h"
#include "ShaderCache.h"
#include "Settings.h"
#include <stdint.h>

namespace blurp
{
    struct CameraData
    {
        glm::mat4 pv;
        glm::vec4 camPos;
    };

    struct SpotLightData
    {
        glm::vec3 color;
        glm::vec3 position;
        glm::vec3 direction;
        float angle;
        float intensity;
    };

    struct PointLightData
    {
        glm::vec3 color;
        glm::vec3 position;
        float intensity;
    };

    struct DirectionalLightData
    {
        glm::vec3 color;
        glm::vec3 direction;
        float intensity;
    };

    class RenderPass_Forward_GL : public RenderPass_Forward
    {
    public:
        RenderPass_Forward_GL(RenderPipeline& a_Pipeline)
            : RenderPass_Forward(a_Pipeline), m_CameraUbo(0)
        {
        }

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;
        void Execute() override;

    private:
        //Shader cache that compiles shaders dynamically based on required attributes.
        ShaderCache<std::uint32_t, std::uint32_t> m_ShaderCache;

        //Camera buffer.
        GLuint m_CameraUbo;
    };
}