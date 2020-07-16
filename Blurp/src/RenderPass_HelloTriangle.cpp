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

    void RenderPass_HelloTriangle::SetTexture(std::shared_ptr<Texture> a_Texture)
    {
        m_Texture = std::move(a_Texture);
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

    std::vector<Lockable*> RenderPass_HelloTriangle::GetLockableResources() const
    {
        //TODO get all lockable resources.
        return std::vector<Lockable*>();
    }
}
