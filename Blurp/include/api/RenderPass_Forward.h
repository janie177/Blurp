#pragma once
#include "RenderTarget.h"
#include "Camera.h"
#include "Light.h"
#include "RenderPass.h"

#include <unordered_set>

namespace blurp
{
    /*
     * Struct containing information for drawing.
     */
    struct LightData
    {
        LightData()
        {
            shadowMapIndex = -1;
            light = nullptr;
        }

        LightData(const std::shared_ptr<Light>& a_Light, const std::int32_t a_ShadowMapIndex) : light(a_Light), shadowMapIndex(a_ShadowMapIndex) {}

        std::shared_ptr<Light> light;
        std::int32_t shadowMapIndex;
    };

    class RenderPass_Forward : public RenderPass
    {
    public:
        RenderPass_Forward(RenderPipeline& a_Pipeline)
            : RenderPass(a_Pipeline), m_DrawDataPtr(nullptr), m_DrawDataCount(0), m_NumDirCascades(0), m_DirCascadeDistance(0), m_LightCounts(0), m_ShadowCounts(0), m_AmbientLight(0), m_ReuploadLights(true)
        {
        }

        RenderPassType GetType() override;

        /*
         * Set the camera to render the scene with.
         */
        void SetCamera(const std::shared_ptr<Camera>& a_Camera);

        /*
         * Set the render target to draw into.
         */
        void SetTarget(const std::shared_ptr<RenderTarget>& a_RenderTarget);

        /*
         * Provide a pointer to the draw data collection that is to be drawn.
         * The amount of DrawData objects is to be specified as a_DrawDataCount.
         *
         * Note that this does not make any copies, and the data set is to remain valid until this render pass has finished executing.
         */
        void SetDrawData(DrawData* a_DrawDataPtr, std::uint32_t a_DrawDataCount);

        /*
         * Add a light to the scene with a corresponding shadowmap and light perspective matrix.
         */
        void AddLight(const std::shared_ptr<Light>& a_Light, const std::int32_t a_ShadowMapIndex);

        /*
         * Add a light with shadowmapping.
         */
        void AddLight(const std::shared_ptr<Light>& a_Light);

        /*
         * Set the shadowmaps used for pointlights.
         * This has to be a Cubemap Array texture.
         */
        void SetPointSpotShadowMaps(const std::shared_ptr<Texture>& a_ShadowMaps);

        /*
         * Set the shadow maps used for all directional lights.
         * This has to be a Texture2D array.
         * The GpuBuffer and GpuBufferView should contain the shadow matrix data.
         * NumCascades represents the amount of shadow cascades for directional shadows.
         * The GpuBufferView is passed by reference and stored as a pointer for reading at execution time.
         */
        void SetDirectionalShadowMaps(const std::shared_ptr<Texture>& a_Texture,
            const std::uint32_t a_NumCascades, const float a_CascadeDistance,
            const std::shared_ptr<GpuBuffer>& a_TransformBuffer,
            GpuBufferView& a_TransformViewPtr);

        /*
         * Reset all queued data. Call this to start a new fresh frame.
         * Not calling Reset means all resources from the last frame will still be drawn.
         */
        void Reset() override;

        /*
         * Reset the lights in this scene.
         */
        void ResetLights();

        /*
         * Reset all queued up draw data in this scene.
         */
        void ResetDrawData();

    protected:
        bool IsStateValid() override;

    protected:

        std::shared_ptr<Camera> m_Camera;
        std::shared_ptr<RenderTarget> m_Output;

        //Drawqueue containing all drawable data. Order is kept while drawing.
        DrawData* m_DrawDataPtr;
        std::uint32_t m_DrawDataCount;

        //All queued up light data.
        std::vector<LightData> m_LightData;

        //Shadowmaps for directional lights. This has to be a texture 2d array. Requires matrix transformations and cascade info.
        std::shared_ptr<Texture> m_DirectionalShadowMaps;
        std::shared_ptr<GpuBuffer> m_DirShadowBuffer;         //The buffer containing the transformation matrices for the lights and all the 
        GpuBufferView* m_DirShadowView;                      //The view into above buffer to where the matrices are stored.
        std::uint32_t m_NumDirCascades;                     //The amount of cascades used with directional shadows.
        float m_DirCascadeDistance;                        //The length of each shadow cascade. Measured in fragment position relative to the camera.

        //Shadowmap for pointlights and spotlights. This has to be a cube map texture array.
        std::shared_ptr<Texture> m_PointSpotShadowMaps;

        //Counters for the lights that have been added.
        glm::vec3 m_LightCounts;     //The amount of each light type in order of POINT, SPOT, DIR
        glm::vec3 m_ShadowCounts;    //The amount of shadowmaps per light in order of POINT, SPOT, DIR
        glm::vec3 m_AmbientLight;    //The sum of ambient lights.
        bool m_ReuploadLights;       //Flag to keep track of whether the uploaded light data is still valid or not to save bandwidth.
    };
}
