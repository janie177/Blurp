#pragma once
#include "Mesh.h"
#include "Camera.h"
#include "RenderPass.h"
#include "RenderTarget.h"

namespace blurp
{
    /*
     * Skybox draws a skybox into a render target from a cubemap.
     */
    class RenderPass_Skybox : public RenderPass
    {
    public:
        explicit RenderPass_Skybox(RenderPipeline& a_Pipeline)
            : RenderPass(a_Pipeline), m_Opacity(1.f), m_MixColor(0.f), m_ColorMultiplier(1.f)
        {
        }

    public:

        /*
         * Set the texture to be used for the skybox.
         */
        void SetTexture(const std::shared_ptr<Texture>& a_Texture);

        /*
         * Set the camera to render the skyboxes perspective from.
         */
        void SetCamera(const std::shared_ptr<Camera>& a_Camera);

        /*
         * Set the skybox opacity.
         * Value has to be between 0 and 1.
         */
        void SetOpacity(float a_Opacity);

        /*
         * Set the RGB color to mix the skybox with.
         * This is added on top of the skybox pixel color.
         */
        void SetMixColor(const glm::vec3& a_MixColor);

        /*
         * Set the color multiplier for the skybox.
         * The skybox pixel color is multiplied with this.
         */
        void SetColorMultiplier(const glm::vec3& a_ColorMultiplier);

        /*
         * Set the output render target.
         */
        void SetTarget(const std::shared_ptr<RenderTarget>& a_Target);

        RenderPassType GetType() override;
        void Reset() override;

    protected:
        bool IsStateValid() override;

    protected:
        std::shared_ptr<Texture> m_Texture;
        std::shared_ptr<RenderTarget> m_Target;
        std::shared_ptr<Camera> m_Camera;
        std::shared_ptr<Mesh> m_CubeMesh;

        //Opacity of the skybox, color to mix with and the color modifier to multiply the skybox with.
        float m_Opacity;
        glm::vec3 m_MixColor;
        glm::vec3 m_ColorMultiplier;

        //Static cube mesh data that is used to draw the skybox.
        inline  const static std::float_t CUBE_DATA[]
        {
            //      X    Y     Z             
                    -1.0f, -1.0f, 1.0f,
                    -1.0f, 1.0f, 1.0f,
                    1.0f, -1.0f, 1.0f,
                    1.0f, 1.0f, 1.0f,

                    1.0f, -1.0f, -1.0f,
                    1.0f, -1.0f, 1.0f,
                    1.0f, 1.0f, -1.0f,
                    1.0f, 1.0f, 1.0f,

                    -1.0f, -1.0f, -1.0f,
                    -1.0f, 1.0f, -1.0f,
                    1.0f, -1.0f, -1.0f,
                    1.0f, 1.0f, -1.0f,

                    -1.0f, -1.0f, -1.0f,
                    -1.0f, -1.0f, 1.0f,
                    -1.0f, 1.0f, -1.0f,
                    -1.0f, 1.0f, 1.0f,

                    -1.0f, 1.0f, -1.0f,
                    -1.0f, 1.0f, 1.0f,
                    1.0f, 1.0f, -1.0f,
                    1.0f, 1.0f, 1.0f,

                    -1.0f, -1.0f, -1.0f,
                    -1.0f, -1.0f, 1.0f,
                    1.0f, -1.0f, -1.0f,
                    1.0f, -1.0f, 1.0f,
        };

        inline const static std::uint16_t CUBE_INDICES[]
        {
            0, 3, 1, 0, 2, 3,	    //SOUTH
            4, 7, 5, 4, 6, 7,	    //EAST
            8, 11, 10, 8, 9, 11,	//NORTH
            12, 15, 14, 12, 13, 15,	//WEST
            16, 19, 18, 16, 17, 19,	//UP
            20, 23, 21, 20, 22, 23,	//DOWN
        };
    };
}
