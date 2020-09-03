#include "RenderPass_Forward.h"

#include <algorithm>

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

    void RenderPass_Forward::SetDrawData(DrawData* a_DrawDataPtr, std::uint32_t a_DrawDataCount)
    {
        m_DrawDataPtr = a_DrawDataPtr;
        m_DrawDataCount = a_DrawDataCount;
    }

    void RenderPass_Forward::AddLight(const std::shared_ptr<Light>& a_Light, const std::int32_t a_ShadowMapIndex)
    {
        assert(a_Light != nullptr && "Cannot add nullptr light to forward renderer!");

        switch (a_Light->GetType())
        {
        case LightType::LIGHT_POINT:
        {
            if (a_ShadowMapIndex > -1)
            {
                m_ShadowCounts.x += 1;
            }
            else
            {
                m_LightCounts.x += 1;
            }
        }
        break;
        case LightType::LIGHT_SPOT:
        {
            if (a_ShadowMapIndex > -1)
            {
                m_ShadowCounts.y += 1;
            }
            else
            {
                m_LightCounts.y += 1;
            }
        }
        break;
        case LightType::LIGHT_DIRECTIONAL:
        {
            if (a_ShadowMapIndex > -1)
            {
                m_ShadowCounts.z += 1;
            }
            else
            {
                m_LightCounts.z += 1;
            }
        }
        break;
        //Append ambient light.
        case LightType::LIGHT_AMBIENT:
        {
            m_AmbientLight += (a_Light->GetColor() * a_Light->GetIntensity());
        }
        break;
        default:
        {
            throw std::exception("Light type not implemented!");
        }
        break;
        }

        m_LightData.emplace_back(LightData{ a_Light, a_ShadowMapIndex});
        m_ReuploadLights = true;
    }

    void RenderPass_Forward::AddLight(const std::shared_ptr<Light>& a_Light)
    {
       AddLight(a_Light, -1);
    }

    void RenderPass_Forward::SetPointSpotShadowMaps(const std::shared_ptr<Texture>& a_ShadowMaps)
    {
        assert(a_ShadowMaps->GetTextureType() == TextureType::TEXTURE_CUBEMAP_ARRAY && "Shadowmaps for pointlights need to be cubemap arrays!");
        m_PointSpotShadowMaps = a_ShadowMaps;
    }

    void RenderPass_Forward::SetDirectionalShadowMaps(const std::shared_ptr<Texture>& a_Texture,
        const std::uint32_t a_NumCascades, const float a_CascadeDistance,
        const std::shared_ptr<GpuBuffer>& a_TransformBuffer, GpuBufferView& a_TransformViewPtr)
    {
        assert(a_Texture != nullptr && a_TransformBuffer != nullptr);
        assert(a_NumCascades > 0u && "For directional lights, 1 cascade is the minimum!");
        assert(a_CascadeDistance > 0.f && "Shadow cascade distance has to be a positive number!");
        assert(a_Texture->GetTextureType() == TextureType::TEXTURE_2D_ARRAY && "Shadowmaps for directional lights need to be 2D texture arrays!");
        m_DirectionalShadowMaps = a_Texture;
        m_DirShadowBuffer = a_TransformBuffer;
        m_DirShadowView = &a_TransformViewPtr;
        m_NumDirCascades = a_NumCascades;
        m_DirCascadeDistance = a_CascadeDistance;
    }

    void RenderPass_Forward::Reset()
    {
        ResetDrawData();
        ResetLights();
    }

    void RenderPass_Forward::ResetLights()
    {
        m_LightCounts = glm::vec3(0.f);
        m_ShadowCounts = glm::vec3(0.f);
        m_AmbientLight = glm::vec3(0.f);
        m_LightData.clear();
        m_ReuploadLights = true;
    }

    void RenderPass_Forward::ResetDrawData()
    {
        m_DrawDataPtr = nullptr;
        m_DrawDataCount = 0;
    }

    bool RenderPass_Forward::IsStateValid()
    {
        return m_Output != nullptr && m_Camera != nullptr;
    }
}
