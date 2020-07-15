#pragma once
#include "RenderTarget.h"
#include "Camera.h"
#include "Light.h"
#include "Mesh.h"
#include "RenderPass.h"
#include "Material.h"

namespace blurp
{
    /*
     * Passing this struct to the forward renderer guarantees in to be drawn
     * in the order it was provided.
     * The order of transforms is also kept.
     */
    struct InstanceData
    {
        //Pointer to the mesh to draw.
        Mesh* mesh;

        //The amount of instances.
        std::uint32_t count;

        //Pointer to the start of the transforms.
        glm::mat4* transform;
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
         * Add one or more meshes to the drawing queue.
         * The InstanceData object contains a pointer to the mesh to use.
         * It also contains the number of instances, and a pointer to the start of the instance data.
         * Data is rendered in the order provided.
         */
        void QueueForDraw(InstanceData a_Data);

        /*
         * Add a light to be taken into consideration when drawing.
         */
        void AddLight(const std::shared_ptr<Light>& a_Light, const std::shared_ptr<Texture>& a_ShadowMap);

        void Reset() override;

    protected:
        bool IsStateValid() override;

    protected:
        std::shared_ptr<Camera> m_Camera;
        std::shared_ptr<RenderTarget> m_Output;

        //Drawqueue sorted by meshes{materials{transforms}}
        //std::unordered_map<Mesh*, std::unordered_map<Material*, std::vector<glm::mat4>>> m_DrawQueue;
        std::vector<InstanceData> m_DrawQueue;

        //Light data.
        std::vector<PointLightData> m_PointLights;
        std::vector<SpotLightData> m_SpotLights;
        std::vector<DirectionalLightData> m_DirectionalLights;

        //Ambient light.
        glm::vec3 m_AmbientColor;
        float m_AmbientIntensity;
    };
}
