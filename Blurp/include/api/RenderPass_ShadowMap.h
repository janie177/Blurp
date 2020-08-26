#pragma once
#include "Camera.h"
#include "Light.h"
#include "RenderPass.h"

namespace blurp
{
    /*
     * Struct containing the data required to render a shadow map for a positional light.
     */
    struct LightShadowData
    {
        LightShadowData(std::int32_t a_Index, const glm::vec3& a_Data, float a_Near, float a_Far) : index(a_Index), data(a_Data), farPlane(a_Far), nearPlane(a_Near)
        {
            
        }

        //The index into the array texture to store the shadow map.
        std::int32_t index;

        //The lights position or direction.
        glm::vec3 data;

        //The far plane. This is the maximum shadow distance for this light.
        float farPlane;

        //The near plane. This is the minimum shadow distance for this light.
        float nearPlane;
    };

    struct LightIndexData
    {
        //Indices of all directional lights that this geometry affects.
        std::vector<std::int32_t> dirIndices;

        //Indices of all positional lights that this geometry affects.
        std::vector<std::int32_t> posIndices;
    };

    class RenderPass_ShadowMap : public RenderPass
    {
    public:
        RenderPass_ShadowMap(RenderPipeline& a_Pipeline)
            : RenderPass(a_Pipeline), m_DrawDataPtr(nullptr), m_LightIndices(nullptr), m_DrawDataCount(0),
              m_DirectionalCascades(0), m_DirectionalCascadeDistance(0)
        {
        }

    public:
        /*
         * Set the camera that is used to draw  the scene.
         */
        void SetCamera(const std::shared_ptr<Camera>& a_Camera);

        /*
         * Add a light to generate a shadowmap.
         * The index provided is the index into the array texture where the generated shadowmap will be stored.
         * Near and Far are used to construct the projection matrix for this light.
         */
        void AddLight(const std::shared_ptr<Light>& a_Light, std::uint32_t a_Index, float a_Near, float a_Far);

        /*
         * Set the geometry that should cast shadows.
         * a_DrawData is a pointer to the start of an array of all draw data that should cast a shadow.
         * a_LightIndexData is a pointer to the start of an array of all light index data. This determines which lights are affected by which geometry.
         * a_Count is the total amount of elements in both arrays. They have to be equal size.
         *
         * If a_LightIndexData is nullptr, all lights will affect all geometry.
         */
        void SetGeometry(const DrawData* a_DrawData, const LightIndexData* a_LightIndexData, const std::uint32_t a_Count);

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

        //Collection of lights with a position. and the texture to store the shadow maps in.
        std::vector<LightShadowData> m_PositionalLights;
        std::shared_ptr<Texture> m_ShadowMapsPositional;

        //Collection of lights with a direction and the texture to store the shadow maps in.
        std::vector<LightShadowData> m_DirectionalLights;
        std::shared_ptr<Texture> m_ShadowMapsDirectional;

        //Pointers to the draw data, which lights affect each piece of geometry and the size of both arrays.
        const DrawData* m_DrawDataPtr;
        const LightIndexData* m_LightIndices;
        std::uint32_t m_DrawDataCount;

        //The camera used to determine directional light positions.
        std::shared_ptr<Camera> m_Camera;

        //How many directional light cascades are used.
        std::uint32_t m_DirectionalCascades;
        std::uint32_t m_DirectionalCascadeDistance;

    };
}
