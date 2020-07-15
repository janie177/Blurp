#pragma once
#include "RenderResource.h"
#include "Transform.h"

namespace blurp
{
    class Camera : public RenderResource
    {
    public:
        Camera(const CameraSettings& a_Settings) : m_Settings(a_Settings), m_DirtyFlag(true), m_View(1.f), m_Projection(1.f) {}

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
        void SetProjection(const CameraSettings& a_Settings);

        /*
         * Get the transform for this camera.
         */
        Transform& GetTransform();

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;

    private:
        CameraSettings m_Settings;
        Transform m_Transform;
        mutable bool m_DirtyFlag;

        mutable glm::mat4 m_View;
        mutable glm::mat4 m_Projection;
    };
}
