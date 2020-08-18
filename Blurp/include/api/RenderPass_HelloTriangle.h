#pragma once
#include <memory>

#include "RenderPass.h"

namespace blurp
{
    class RenderTarget;

    /*
     * A renderpass that draws a triangle on the screen for testing purposes.
     */
    class RenderPass_HelloTriangle : public RenderPass
    {
    public:
        RenderPass_HelloTriangle(RenderPipeline& a_Pipeline) : RenderPass(a_Pipeline), m_Color({1.f, 1.f, 1.f, 1.f }){}

        /*
         * Set the triangle color.
         */
        void SetColor(const glm::vec4& a_Color);

        /*
         * Get the type of this RenderPass.
         */
        RenderPassType GetType() override;

        /*
         * Set the target to render the triangle into.
         */
        void SetTarget(std::shared_ptr<RenderTarget> a_Target);

        /*
         * For triangle drawing this doesn't do anything.
         */
        void Reset() override;

    protected:

        bool IsStateValid() override;

        std::vector<std::pair<Lockable*, LockType>> GetLockableResources() const override;

    protected:
        glm::vec4 m_Color;
        std::shared_ptr<RenderTarget> m_Target;
    };
}
