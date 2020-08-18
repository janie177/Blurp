#include "RenderPass_Skybox.h"
#include "Texture.h"


namespace blurp
{
    void RenderPass_Skybox::SetTexture(const std::shared_ptr<Texture>& a_Texture)
    {
        assert(a_Texture->GetTextureType() == TextureType::TEXTURE_CUBEMAP && "Cubemaps have to be used to render a skybox!");
        m_Texture = a_Texture;
    }

    void RenderPass_Skybox::SetCamera(const std::shared_ptr<Camera>& a_Camera)
    {
        m_Camera = a_Camera;
    }

    void RenderPass_Skybox::SetOpacity(float a_Opacity)
    {
        assert(a_Opacity >= 0.f && a_Opacity <= 1.f);
        m_Opacity= a_Opacity;
    }

    void RenderPass_Skybox::SetMixColor(const glm::vec3& a_MixColor)
    {
        m_MixColor = a_MixColor;
    }

    void RenderPass_Skybox::SetColorMultiplier(const glm::vec3& a_ColorMultiplier)
    {
        m_ColorMultiplier = a_ColorMultiplier;
    }

    void RenderPass_Skybox::SetTarget(const std::shared_ptr<RenderTarget>& a_Target)
    {
        m_Target = a_Target;
    }

    RenderPassType RenderPass_Skybox::GetType()
    {
        return RenderPassType::RP_SKYBOX;
    }

    void RenderPass_Skybox::Reset()
    {
        //No reset needed.
    }

    std::vector<std::pair<Lockable*, LockType>> RenderPass_Skybox::GetLockableResources() const
    {
        std::vector<std::pair<Lockable*, LockType>> lockVector;
        if(m_Texture->GetAccessMode() != AccessMode::READ_ONLY)
        {
            lockVector.emplace_back(std::make_pair(m_Texture.get(), LockType::READ));
        }

        lockVector.emplace_back(std::make_pair(m_Target.get(), LockType::WRITE));
        return lockVector;
    }

    bool RenderPass_Skybox::IsStateValid()
    {
        return m_Camera != nullptr && m_Target != nullptr && m_Texture != nullptr && m_Texture->GetTextureType() == TextureType::TEXTURE_CUBEMAP;
    }
}
