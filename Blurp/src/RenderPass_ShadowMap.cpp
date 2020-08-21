#include "RenderPass_ShadowMap.h"
#include "Texture.h"

namespace blurp
{
    void RenderPass_ShadowMap::AddLight(const ShadowMapGenerationData& a_Data)
    {
        assert(a_Data.light && "Light cannot be nullptr!");
        assert(a_Data.drawData != nullptr && a_Data.drawDataCount > 0 && "Generating a shadowmap without any geometry doesn't make sense!");

        switch (a_Data.light->GetType())
        {
        case LightType::LIGHT_POINT:
        case LightType::LIGHT_SPOT:
            m_PositionalLights.emplace_back(a_Data);
            break;
        case LightType::LIGHT_DIRECTIONAL:
            m_DirectionalLights.emplace_back(a_Data);
            break;
        default:
            throw std::exception("Light type cannot generate a shadow!");
            break;
        }
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

        return true;
    }
}
