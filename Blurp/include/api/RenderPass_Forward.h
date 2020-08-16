#pragma once
#include "RenderTarget.h"
#include "Camera.h"
#include "GpuBuffer.h"
#include "Light.h"
#include "Material.h"
#include "MaterialBatch.h"
#include "Mesh.h"
#include "RenderPass.h"

namespace blurp
{
    /*
     * Object containing information about a draw call.
     * The mesh specified will be drawn count times.
     * The data related to this instance will be fetched from the GPU buffer bound using the GpuBufferView offsets.
     * The pointers to the start and size of the data in the gpu buffer are found in dataRange.
     *
     * Enable transform and inverse transform to indicate which data is inside the GpuBuffer.
     */
    struct ForwardDrawData
    {
        ForwardDrawData()
        {
            transformData.transform = false;
            transformData.inverseTransform = false;
            count = 0;
        }


        //Pointer to the mesh to draw.
        std::shared_ptr<Mesh> mesh;

        //The amount of instances.
        std::uint32_t count;


        struct
        {
            //Pointer to the material to use for drawing.
            std::shared_ptr<Material> material;

            //Pointer to the material batch to use for drawing.
            std::shared_ptr<MaterialBatch> materialBatch;
        } materialData;

        struct
        {
            //GpuBufferView object containing pointers to the offsets in the GPU buffer where the data is stored (bound as transformBuffer).
            GpuBufferView dataRange;

            //Set to true if the data range contains the transform. ORDER MATTERS: TRANSFORM > INVERSE_TRANSFORM.
            bool transform;

            //Set to true if the data range contains the inverse transform. ORDER MATTERS: TRANSFORM > INVERSE_TRANSFORM.
            bool inverseTransform;
        } transformData;

        struct
        {
            //The Gpubufferview into the bound GpuBuffer for uv coordinate modifications.
            GpuBufferView dataRange;
        } uvModifierData;
    };

    /*
     * A modifier vor uv coordinates.
     * This multiplies the UV coordinates with "multiply" and adds "add" on top.
     */
    struct UvModifier
    {
        UvModifier()
        {
            multiply = { 1.f, 1.f };
            add = { 0.f, 0.f };
        }

        UvModifier(glm::vec2 a_Multiply, glm::vec2 a_Add) : multiply(a_Multiply), add(a_Add)
        { 
        }

        //Multiply the UV coords with this.
        glm::vec2 multiply;

        //Add this onto the UV coordinates.
        glm::vec2 add;
    };

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

        LightData(const std::shared_ptr<Light>& a_Light, const std::int32_t a_ShadowMapIndex, const glm::mat4& a_ShadowMatrix) : light(a_Light), shadowMapIndex(a_ShadowMapIndex), shadowMatrix(a_ShadowMatrix) {}

        std::shared_ptr<Light> light;
        std::int32_t shadowMapIndex;
        glm::mat4 shadowMatrix;
    };

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
         * Set the GPU buffer to read the transform data from.
         */
        void SetTransformBuffer(const std::shared_ptr<GpuBuffer>& a_Buffer);

        /*
         * Set the GPU buffer to read uv modifications from.
         */
        void SetUvModifierBuffer(const std::shared_ptr<GpuBuffer>& a_Buffer);

        /*
         * Add one or more meshes to the drawing queue.
         * The InstanceData object contains a pointer to the mesh to use.
         * It also contains the number of instances, and a pointer to the start of the instance data.
         * Data is rendered in the order provided.
         */
        void QueueForDraw(ForwardDrawData a_Data);

        /*
         * Add a light to the scene with a corresponding shadowmap and light perspective matrix.
         */
        void AddLight(const std::shared_ptr<Light>& a_Light, const std::int32_t a_ShadowMapIndex, const glm::mat4& a_ShadowMatrix);

        /*
         * Add a light with shadowmapping.
         */
        void AddLight(const std::shared_ptr<Light>& a_Light);

        /*
         * Set the shadowmaps used for pointlights.
         * This has to be a Cubemap Array texture.
         */
        void SetPointShadowMaps(const std::shared_ptr<Texture>& a_ShadowMaps);

        /*
         * Set the shadow maps used for all directional lights.
         * This has to be a Texture2D array.
         */
        void SetDirectionalShadowMaps(const std::shared_ptr<Texture>& a_ShadowMaps);

        /*
         * Reset all queued data. Call this to start a new fresh frame.
         * Not calling Reset means all resources from the last frame will still be drawn.
         */
        void Reset() override;

    protected:
        bool IsStateValid() override;

        std::vector<std::pair<Lockable*, LockType>> GetLockableResources() const override;

    protected:

        std::shared_ptr<Camera> m_Camera;
        std::shared_ptr<RenderTarget> m_Output;

        //Drawqueue containing all drawable data. Order is kept while drawing.
        std::vector<ForwardDrawData> m_DrawQueue;

        std::shared_ptr<GpuBuffer> m_TransformBuffer;
        std::shared_ptr<GpuBuffer> m_UvModifierBuffer;

        //All queued up light data.
        std::vector<LightData> m_LightData;

        //Shadowmaps for directional lights. This has to be a texture 2d array.
        std::shared_ptr<Texture> m_DirectionalShadowMaps;

        //Shadowmap for pointlights and spotlights. This has to be a cubemat texture array.
        std::shared_ptr<Texture> m_PointShadowMaps;

        //Ambient light.
        glm::vec3 m_AmbientColor;
        float m_AmbientIntensity;
    };
}
