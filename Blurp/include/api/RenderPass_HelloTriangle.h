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
         * Set the texture for the triangle.
         */
        void SetTexture(std::shared_ptr<Texture> a_Texture);

        /*
         * Get the type of this RenderPass.
         */
        RenderPassType GetType() override;

        /*
         * Set the target to render the triangle into.
         */
        void SetTarget(std::shared_ptr<RenderTarget> a_Target);

    protected:
        void Reset() override;

        bool IsStateValid() override;

    protected:
        glm::vec4 m_Color;
        std::shared_ptr<RenderTarget> m_Target;
        std::shared_ptr<Texture> m_Texture;
    };
}
