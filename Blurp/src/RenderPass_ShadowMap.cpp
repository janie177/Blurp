#include "RenderPass_ShadowMap.h"
#include "Texture.h"

namespace blurp
{
    void RenderPass_ShadowMap::SetCamera(const std::shared_ptr<Camera>& a_Camera)
    {
        m_Camera = a_Camera;
    }

    void RenderPass_ShadowMap::AddLight(const std::shared_ptr<Light>& a_Light, std::uint32_t a_Index, float a_Near, float a_Far)
    {
        assert(a_Light && "Light cannot be nullptr!");

        //Cast to the right light type and then extract the required data.
        switch (a_Light->GetType())
        {
        case LightType::LIGHT_POINT:
            m_PositionalLights.emplace_back(LightShadowData(a_Index, std::static_pointer_cast<PointLight>(a_Light)->GetPosition(), a_Near, a_Far));
            break;
        case LightType::LIGHT_SPOT:
            m_PositionalLights.emplace_back(LightShadowData(a_Index, std::static_pointer_cast<SpotLight>(a_Light)->GetPosition(), a_Near, a_Far));
            break;
        case LightType::LIGHT_DIRECTIONAL:
            m_DirectionalLights.emplace_back(LightShadowData(a_Index, std::static_pointer_cast<DirectionalLight>(a_Light)->GetDirection(), a_Near, a_Far));
            break;
        default:
            throw std::exception("Light type cannot generate a shadow!");
            break;
        }
    }

    void RenderPass_ShadowMap::SetGeometry(const DrawData* a_DrawData, const LightIndexData* a_LightIndexData, const std::uint32_t a_Count)
    {
        m_DrawDataPtr = a_DrawData;
        m_DrawDataCount = a_Count;
        m_LightIndices = a_LightIndexData;
    }

    void RenderPass_ShadowMap::SetOutputPositional(const std::shared_ptr<Texture>& a_Texture)
    {
        assert(a_Texture->GetTextureType() == TextureType::TEXTURE_CUBEMAP_ARRAY && "Positional light shadows require a Cubemap Texture Array.");
        assert(a_Texture->GetPixelFormat() == PixelFormat::DEPTH && "Shadowmaps only need a depth channel.");
        m_ShadowMapsPositional = a_Texture;
    }

    void RenderPass_ShadowMap::SetOutputDirectional(const std::shared_ptr<Texture>& a_Texture)
    {
        assert(a_Texture->GetTextureType() == TextureType::TEXTURE_2D_ARRAY && "Directional light shadows require a 2D Texture Array.");
        assert(a_Texture->GetPixelFormat() == PixelFormat::DEPTH && "Shadowmaps only need a depth channel.");
        m_ShadowMapsDirectional = a_Texture;
    }

    void RenderPass_ShadowMap::SetDirectionalCascading(std::uint32_t a_NumCascades, std::uint32_t a_CascadeDistance)
    {
        assert((a_CascadeDistance >= 1.f || a_NumCascades <= 1.f) && "When shadow cascading is enabled, the length of each cascade has to be at least 1.");
        m_DirectionalCascades = a_NumCascades;
        m_DirectionalCascadeDistance = a_CascadeDistance;
    }

    RenderPassType RenderPass_ShadowMap::GetType()
    {
        return RenderPassType::RP_SHADOWMAP;
    }

    void RenderPass_ShadowMap::Reset()
    {
        m_DirectionalLights.clear();
        m_PositionalLights.clear();
        m_DrawDataPtr = nullptr;
        m_DrawDataCount = 0;
    }

    bool RenderPass_ShadowMap::IsStateValid()
    {
        if(!m_DirectionalLights.empty() && !m_ShadowMapsDirectional)
        {
            return false;
        }

        if (!m_PositionalLights.empty() && !m_ShadowMapsPositional)
        {
            return false;
        }

        if(m_Camera == nullptr)
        {
            return false;
        }

        return true;
    }
}
