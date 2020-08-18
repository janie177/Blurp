#pragma once
#include "RenderPass_Skybox.h"
#include "Shader_GL.h"

namespace blurp
{
    class RenderPass_Skybox_GL : public RenderPass_Skybox
    {
    public:
        explicit RenderPass_Skybox_GL(RenderPipeline& a_Pipeline)
            : RenderPass_Skybox(a_Pipeline)
        {
        }

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;
        void Execute() override;

    private:
        std::shared_ptr<Shader_GL> m_Shader;
    };
}
