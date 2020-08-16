#include "RenderPass_Forward.h"
#include "Texture.h"

namespace blurp
{
    RenderPassType RenderPass_Forward::GetType()
    {
        return RenderPassType::RP_FORWARD;
    }

    void RenderPass_Forward::SetCamera(const std::shared_ptr<Camera>& a_Camera)
    {
        m_Camera = a_Camera;
    }

    void RenderPass_Forward::SetTarget(const std::shared_ptr<RenderTarget>& a_RenderTarget)
    {
        m_Output = a_RenderTarget;
    }

    void RenderPass_Forward::SetTransformBuffer(const std::shared_ptr<GpuBuffer>& a_Buffer)
    {
        m_TransformBuffer = a_Buffer;
    }

    void RenderPass_Forward::SetUvModifierBuffer(const std::shared_ptr<GpuBuffer>& a_Buffer)
    {
        m_UvModifierBuffer = a_Buffer;
    }

    void RenderPass_Forward::QueueForDraw(ForwardDrawData a_Data)
    {
        m_DrawQueue.emplace_back(std::move(a_Data));
    }

    void RenderPass_Forward::AddLight(const std::shared_ptr<Light>& a_Light, const std::int32_t a_ShadowMapIndex, const glm::mat4& a_ShadowMatrix)
    {
        m_LightData.emplace_back(LightData{ a_Light, a_ShadowMapIndex, a_ShadowMatrix });
    }

    void RenderPass_Forward::AddLight(const std::shared_ptr<Light>& a_Light)
    {
        m_LightData.emplace_back(LightData{a_Light, -1, glm::mat4()});
    }

    void RenderPass_Forward::SetPointShadowMaps(const std::shared_ptr<Texture>& a_ShadowMaps)
    {
        assert(a_ShadowMaps->GetTextureType() == TextureType::TEXTURE_CUBEMAP_ARRAY && "Shadowmaps for pointlights need to be cubemap arrays!");
        m_PointShadowMaps = a_ShadowMaps;
    }

    void RenderPass_Forward::SetDirectionalShadowMaps(const std::shared_ptr<Texture>& a_ShadowMaps)
    {
        assert(a_ShadowMaps->GetTextureType() == TextureType::TEXTURE_2D_ARRAY && "Shadowmaps for directional lights need to be 2D texture arrays!");
        m_DirectionalShadowMaps = a_ShadowMaps;
    }

    void RenderPass_Forward::Reset()
    {
        m_DrawQueue.clear();
        m_LightData.clear();
    }

    bool RenderPass_Forward::IsStateValid()
    {
        return m_Output != nullptr && m_Camera != nullptr;
    }

    std::vector<std::pair<Lockable*, LockType>> RenderPass_Forward::GetLockableResources() const
    {
        std::vector<std::pair<Lockable*, LockType>> lockables;

        if (m_Output != nullptr)
        {
            lockables.emplace_back(std::make_pair(m_Output.get(), LockType::WRITE));
        }

        if (m_TransformBuffer != nullptr)
        {
            lockables.emplace_back(std::make_pair(m_TransformBuffer.get(), LockType::READ));
        }

        if(m_UvModifierBuffer != nullptr)
        {
            lockables.emplace_back(std::make_pair(m_UvModifierBuffer.get(), LockType::READ));
        }

        if(m_PointShadowMaps != nullptr)
        {
            lockables.emplace_back(std::make_pair(m_PointShadowMaps.get(), LockType::READ));
        }

        if(m_DirectionalShadowMaps != nullptr)
        {
            lockables.emplace_back(std::make_pair(m_DirectionalShadowMaps.get(), LockType::READ));
        }

        return lockables;
    }
}
