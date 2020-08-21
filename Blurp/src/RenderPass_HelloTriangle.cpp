#include "RenderPass_HelloTriangle.h"

#include <utility>
#include "RenderTarget.h"
#include "Texture.h"

namespace blurp
{
    void RenderPass_HelloTriangle::SetColor(const glm::vec4& a_Color)
    {
        m_Color = a_Color;
    }

    RenderPassType RenderPass_HelloTriangle::GetType()
    {
        return RenderPassType::RP_HELLOTRIANGLE;
    }

    void RenderPass_HelloTriangle::SetTarget(std::shared_ptr<RenderTarget> a_Target)
    {
        m_Target = std::move(a_Target);
    }

    void RenderPass_HelloTriangle::Reset()
    {
        //Nothing to do here.
    }

    bool RenderPass_HelloTriangle::IsStateValid()
    {
        return m_Target != nullptr && m_Target->HasColorAttachment();
    }
}
