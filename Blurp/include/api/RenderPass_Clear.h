#pragma once
#include "RenderTarget.h"
#include "RenderPass.h"

namespace blurp
{
    /*
     * This renderpass allows the clearing of render targets and other writable data.
     */
    class RenderPass_Clear : public RenderPass
    {
    public:
        explicit RenderPass_Clear(RenderPipeline& a_Pipeline)
            : RenderPass(a_Pipeline)
        {
        }

        /*
         * Add a render target to be cleared.
         */
        void AddRenderTarget(const std::shared_ptr<RenderTarget>& a_Target);

        /*
         * Add a texture to be cleared with the given clear color.
         */
        void AddTexture(const std::shared_ptr<Texture>& a_Texture, const ClearData& a_ClearData);

        RenderPassType GetType() override;
        void Reset() override;

    protected:
        bool IsStateValid() override;

    protected:
        std::vector<std::shared_ptr<RenderTarget>> m_RenderTargets;
        std::vector<std::pair<std::shared_ptr<Texture>, ClearData>> m_Textures;
    };
}
