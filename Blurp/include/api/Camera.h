#pragma once
#include "RenderResource.h"
#include "Transform.h"

namespace blurp
{
    class Camera : public RenderResource
    {
    public:
        Camera(const CameraSettings& a_Settings) : m_Settings(a_Settings), m_DirtyFlag(true) {}

        /*
         * Get the view matrix of this camera.
         */
        glm::mat4 GetViewMatrix() const;

        /*
         * Get the projection matrix of this camera.
         */
        glm::mat4 GetProjectionMatrix() const;

        /*
         * Set the projection settings for this camera.
         */
        void SetProjection(float a_Fov, float a_AspectRatio, float a_NearPlane, float a_FarPlane);

        /*
         * Get the transform for this camera.
         */
        Transform& GetTransform();

    private:
        CameraSettings m_Settings;
        Transform m_Transform;
        mutable bool m_DirtyFlag;

        mutable glm::mat4 m_View;
        mutable glm::mat4 m_Projection;
    };
}
