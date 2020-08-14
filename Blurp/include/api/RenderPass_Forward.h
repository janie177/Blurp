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
            data.transform = false;
            data.inverseTransform = false;
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
            //GpuBufferView object containing pointers to the offsets in the GPU buffer where the data is stored.
            GpuBufferView dataRange;

            //Set to true if the data range contains the transform. ORDER MATTERS: TRANSFORM > INVERSE_TRANSFORM.
            bool transform;

            //Set to true if the data range contains the inverse transform. ORDER MATTERS: TRANSFORM > INVERSE_TRANSFORM.
            bool inverseTransform;
        } data;
    };

    /*
     * Struct containing information for drawing.
     */
    struct LightData
    {
        LightData()
        {
            light = nullptr;
            shadowMap = nullptr;
        }

        LightData(Light* a_Light, Texture* a_ShadowMap) : light(a_Light), shadowMap(a_ShadowMap) {}

        Light* light;
        Texture* shadowMap;
    };

    struct SpotLightData
    {
        glm::vec3 color;
        glm::vec3 position;
        glm::vec3 direction;
        float angle;
        float intensity;
    };

    struct PointLightData
    {
        glm::vec3 color;
        glm::vec3 position;
        float intensity;
    };

    struct DirectionalLightData
    {
        glm::vec3 color;
        glm::vec3 direction;
        float intensity;
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
         * Set the GPU buffer to read from in the shaders.
         */
        void SetGpuBuffer(const std::shared_ptr<GpuBuffer>& a_Buffer);

        /*
         * Add one or more meshes to the drawing queue.
         * The InstanceData object contains a pointer to the mesh to use.
         * It also contains the number of instances, and a pointer to the start of the instance data.
         * Data is rendered in the order provided.
         */
        void QueueForDraw(ForwardDrawData a_Data);

        /*
         * Add a light to be taken into consideration when drawing.
         */
        void AddLight(const std::shared_ptr<Light>& a_Light, const std::shared_ptr<Texture>& a_ShadowMap);

        //TODO reset may need to have a virtual layer. In D3D12 it could be used to keep the command list to have efficient drawing.
        //TODO example: all static lights and geometry are drawn without rebuilding anything. Then on top of that image the dynamic stuff happens.
        //TODO but that may cause issues with shadows from dynamic objects. Idk. Blend? Deferred?
        void Reset() override;

    protected:
        bool IsStateValid() override;

        //Get all resources used in this pass that should not be modified while the GPU is busy rendering.
        std::vector<Lockable*> GetLockableResources() const override;

    protected:
        std::shared_ptr<Camera> m_Camera;
        std::shared_ptr<RenderTarget> m_Output;

        //Drawqueue sorted by meshes{materials{transforms}}
        //std::unordered_map<Mesh*, std::unordered_map<Material*, std::vector<glm::mat4>>> m_DrawQueue;
        std::vector<ForwardDrawData> m_DrawQueue;

        std::shared_ptr<GpuBuffer> m_GpuBuffer;

        //Light data.
        std::vector<PointLightData> m_PointLights;
        std::vector<SpotLightData> m_SpotLights;
        std::vector<DirectionalLightData> m_DirectionalLights;

        //Ambient light.
        glm::vec3 m_AmbientColor;
        float m_AmbientIntensity;
    };
}
