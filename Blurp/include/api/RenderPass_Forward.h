#pragma once
#include "RenderTarget.h"
#include "Camera.h"
#include "Light.h"
#include "RenderPass.h"

#include <unordered_set>

namespace blurp
{
    class RenderPass_Forward : public RenderPass
    {
    public:
        RenderPass_Forward(RenderPipeline& a_Pipeline)
            : RenderPass(a_Pipeline)
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
        void SetDrawData(const DrawDataSet& a_DrawDataSet);

        /*
         * Set the light to be used for this scene.
         */
        void SetLights(const LightData& a_LightData);

        /*
         * Set the shadow maps for this scene and other relevant information.
         */
        void SetShadowData(const ShadowData& a_ShadowData);

        /*
         * Reset for the next frame.
         */
        void Reset() override;

    protected:
        bool IsStateValid() override;

    protected:

        std::shared_ptr<Camera> m_Camera;
        std::shared_ptr<RenderTarget> m_Output;

        //Draw queue containing all drawable data. Order is kept while drawing.
        DrawDataSet m_DrawDataSet;

        //Shadow information for all lights.
        ShadowData m_ShadowData;

        //The light data object containing all information about the lights in the scene.
        LightData m_LightData;
    };
}
