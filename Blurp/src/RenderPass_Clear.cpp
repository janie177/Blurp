#include "RenderPass_Clear.h"
#include "Texture.h"

namespace blurp
{
    void RenderPass_Clear::AddRenderTarget(const std::shared_ptr<RenderTarget>& a_Target)
    {
        m_RenderTargets.emplace_back(a_Target);
    }

    void RenderPass_Clear::AddTexture(const std::shared_ptr<Texture>& a_Texture, const glm::vec4& a_ClearColor)
    {
        assert(a_Texture->GetAccessMode() == AccessMode::READ_WRITE && "Textures can only be cleared if they are mutable!");
        m_Textures.emplace_back(std::make_pair(a_Texture, a_ClearColor));
    }

    RenderPassType RenderPass_Clear::GetType()
    {
        return RenderPassType::RP_CLEAR;
    }

    void RenderPass_Clear::Reset()
    {
        m_RenderTargets.clear();
        m_Textures.clear();
    }

    bool RenderPass_Clear::IsStateValid()
    {
        return true;
    }
}
