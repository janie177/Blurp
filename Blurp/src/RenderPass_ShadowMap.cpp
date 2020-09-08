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

    void RenderPass_ShadowMap::SetOutputPositional(const std::shared_ptr<Texture>& a_Texture)
    {
        assert(a_Texture->GetTextureType() == TextureType::TEXTURE_CUBEMAP_ARRAY && "Positional light shadows require a Cubemap Texture Array.");
        assert(a_Texture->GetPixelFormat() == PixelFormat::DEPTH && "Shadowmaps only need a depth channel.");
        m_ShadowMapsPositional = a_Texture;
    }

    void RenderPass_ShadowMap::SetOutputDirectional(const std::shared_ptr<Texture>& a_Texture,
        const std::uint32_t a_NumCascades, const std::vector<float>& a_CascadeDistances,
        const std::shared_ptr<GpuBuffer>& a_TransformBuffer, const std::shared_ptr<GpuBufferView>& a_Offset,
        const std::shared_ptr<GpuBufferView>& a_TransformView)
    {
        //Ensure that cascading is set up correctly.
        assert(a_Offset != nullptr && "Offset buffer cannot be empty!");
        assert(a_TransformView != nullptr && "The GpuBuffer to store information in cannot be empty!");
        assert(a_CascadeDistances.size() == a_NumCascades && "The amount of shadow cascade distances has to be equal to the amount of cascades!");
        assert(a_NumCascades >= 1u && "A minimum of 1 shadow cascade is required.");

#ifndef NDEBUG
        for (auto& f : a_CascadeDistances)
        {
            assert(f >= 0.f && "Cascade distances have to be positive!");
        }
#endif
        //Ensure that all distances are positive.


        m_NumDirectionalCascades = a_NumCascades;
        m_DirectionalCascadeDistances = a_CascadeDistances;

        //Ensure the texture provided is the right format.
        assert(a_Texture->GetTextureType() == TextureType::TEXTURE_2D_ARRAY && "Directional light shadows require a 2D Texture Array.");
        assert(a_Texture->GetPixelFormat() == PixelFormat::DEPTH && "Shadowmaps only need a depth channel.");
        m_ShadowMapsDirectional = a_Texture;

        //Store the buffer and view with required offset to store transforms in for each light cascade.
        assert(a_TransformBuffer != nullptr && "Directional shadow transform buffer cannot be nullptr!");
        m_DirShadowTransformBuffer = a_TransformBuffer;
        m_DirShadowTransformView = a_TransformView;
        m_DirShadowTransformOffset = a_Offset;
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
        if(!m_DirectionalLights.empty() && (!m_ShadowMapsDirectional || !m_DirShadowTransformBuffer || m_DirShadowTransformView == nullptr))
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
