#include "RenderPass_ShadowMap.h"
#include "Texture.h"

namespace blurp
{
    void RenderPass_ShadowMap::SetCamera(const std::shared_ptr<Camera>& a_Camera)
    {
        m_Camera = a_Camera;
    }

    void RenderPass_ShadowMap::AddLight(const std::shared_ptr<Light>& a_Light, std::uint32_t a_Index)
    {
        assert(a_Light && "Light cannot be nullptr!");

        //Cast to the right light type and then extract the required data.
        switch (a_Light->GetType())
        {
        case LightType::LIGHT_POINT:
            m_PositionalLights.emplace_back(LightShadowData(a_Index, std::static_pointer_cast<PointLight>(a_Light)->GetPosition()));
            break;
        case LightType::LIGHT_SPOT:
            m_PositionalLights.emplace_back(LightShadowData(a_Index, std::static_pointer_cast<SpotLight>(a_Light)->GetPosition()));
            break;
        case LightType::LIGHT_DIRECTIONAL:
            m_DirectionalLights.emplace_back(LightShadowData(a_Index, std::static_pointer_cast<DirectionalLight>(a_Light)->GetDirection()));
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

    void RenderPass_ShadowMap::SetOutput(const ShadowData& a_Data)
    {
        //Ensure positional shadows are correctly set up. No shadow maps are generated if the textures provided are null.
        assert((a_Data.positional.shadowMaps == nullptr || a_Data.positional.shadowMaps->GetTextureType() == TextureType::TEXTURE_CUBEMAP_ARRAY) && "Positional light shadows require a Cubemap Texture Array.");
        assert((a_Data.positional.shadowMaps == nullptr || a_Data.positional.shadowMaps->GetPixelFormat() == PixelFormat::DEPTH) && "Shadowmaps only need a depth channel.");

        //Ensure that cascading is set up correctly.
        assert((a_Data.directional.shadowMaps == nullptr || a_Data.directional.startOffset != nullptr) && "Offset buffer cannot be empty!");
        assert((a_Data.directional.shadowMaps == nullptr || a_Data.directional.dataBuffer != nullptr) && "The GpuBuffer to store information in cannot be empty!");
        assert((a_Data.directional.shadowMaps == nullptr || a_Data.directional.cascadeDistances.size() == a_Data.directional.numCascades) && "The amount of shadow cascade distances has to be equal to the amount of cascades!");
        assert((a_Data.directional.shadowMaps == nullptr || a_Data.directional.numCascades >= 1u) && "A minimum of 1 shadow cascade is required.");
        assert((a_Data.directional.shadowMaps == nullptr || a_Data.directional.shadowMaps->GetTextureType() == TextureType::TEXTURE_2D_ARRAY) && "Directional light shadows require a 2D Texture Array.");
        assert((a_Data.directional.shadowMaps == nullptr || a_Data.directional.shadowMaps->GetPixelFormat() == PixelFormat::DEPTH) && "Shadowmaps only need a depth channel.");
        assert((a_Data.directional.shadowMaps == nullptr || a_Data.directional.dataBuffer != nullptr) && "Directional shadow transform buffer cannot be nullptr!");

#ifndef NDEBUG
        for (auto& f : a_Data.directional.cascadeDistances)
        {
            assert(f >= 0.f && "Cascade distances have to be positive!");
        }
#endif

        //Finally store the data struct if everything is in order.
        m_ShadowData = a_Data;
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
        //A directional light is queued for shadow generation, but there's no directional shadow map.
        if(!m_DirectionalLights.empty() && !m_ShadowData.directional.shadowMaps)
        {
            return false;
        }

        //A positional light is queued, but there is no shadow texture specified.
        if (!m_PositionalLights.empty() && !m_ShadowData.positional.shadowMaps)
        {
            return false;
        }

        //There is no camera specified for the scene.
        if(m_Camera == nullptr)
        {
            return false;
        }

        return true;
    }
}
