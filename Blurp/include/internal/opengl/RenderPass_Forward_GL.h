#pragma once

#include "RenderPass_Forward.h"
#include "opengl/Shader_GL.h"

namespace blurp
{
    struct MeshInstanceData
    {
        glm::mat4 transform;
    };

    class RenderPass_Forward_GL : public RenderPass_Forward
    {
    public:
        RenderPass_Forward_GL(RenderPipeline& a_Pipeline)
            : RenderPass_Forward(a_Pipeline), m_Ssbo(0)
        {
        }

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;
        void Execute() override;

    private:
        std::shared_ptr<Shader_GL> m_Shader;
        GLuint m_Ssbo;
    };
}