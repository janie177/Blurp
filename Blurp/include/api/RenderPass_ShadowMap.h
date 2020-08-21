#pragma once
#include "Light.h"
#include "RenderPass.h"

namespace blurp
{
    /*
     * Data required 
     */
    struct ShadowMapGenerationData
    {
        ShadowMapGenerationData()
        {
            index = 0;
            drawData = nullptr;
            drawDataCount = 0;
        }

        ShadowMapGenerationData(std::uint32_t a_Index, std::shared_ptr<Light> a_Light, DrawData* a_DataPtr, std::uint32_t a_DataCount) : index(a_Index), light(a_Light), drawData(a_DataPtr), drawDataCount(a_DataCount)
        {
            
        }

        //The index into the array texture to store the shadow map.
        std::uint32_t index;

        //The light to render the shadow map for.
        std::shared_ptr<Light> light;

        //A pointer to the start of the draw data affecting this light.
        DrawData* drawData;

        //The amount of draw data in the data pointer provided above.
        std::uint32_t drawDataCount;
    };

    class RenderPass_ShadowMap : public RenderPass
    {
    public:
        explicit RenderPass_ShadowMap(RenderPipeline& a_Pipeline)
            : RenderPass(a_Pipeline), m_DirectionalCascades(0), m_DirectionalCascadeDistance(0)
        {
        }

    public:
        /*
         * Add a light to generate a shadowmap.
         * The index provided is the index into the array texture where the generated shadowmap will be stored.
         */
        void AddLight(const ShadowMapGenerationData& a_Data);

        /*
         * Set the output array texture to store the shadow maps in.
         * This texture is used for all point and spotlights.
         * Has to be of texture type TextureCubeArray.
         * PixelFormat has to be Depth.
         */
        void SetOutputPositional(const std::shared_ptr<Texture>& a_Texture);

        /*
         * Set the output array texture to store the shadow maps in.
         * This texture is used for all directional lights.
         * Has to be of texture type Texture2DArray.
         * PixelFormat has to be Depth.
         */
        void SetOutputDirectional(const std::shared_ptr<Texture>& a_Texture);

        /*
         * Set directional light cascading mode.
         * NumCascades is the number of cascades generated per directional light map. 0 For no cascading.
         * CascadeDistance represents the distance each cascade will span along the light direction.
         * A minimum distance of 1 is required.
         */
        void SetDirectionalCascading(std::uint32_t a_NumCascades, std::uint32_t a_CascadeDistance);

        RenderPassType GetType() override;

        void Reset() override;

    protected:

        bool IsStateValid() override;

    protected:

        //Collection of lights with a position. and the texture to store the shadowmaps in.
        std::vector<ShadowMapGenerationData> m_PositionalLights;
        std::shared_ptr<Texture> m_ShadowMapsPositional;

        //Collection of lights with a direction and the texture to store the shadowmaps in.
        std::vector<ShadowMapGenerationData> m_DirectionalLights;
        std::shared_ptr<Texture> m_ShadowMapsDirectional;

        //How many directional light cascades are used.
        std::uint32_t m_DirectionalCascades;
        std::uint32_t m_DirectionalCascadeDistance;

    };
}
