#include "Camera.h"

namespace blurp
{
    glm::mat4 Camera::GetViewMatrix() const
    {
        //Rebuild the inverse if required.
        if(m_Transform.NeedsRebuilding())
        {
           m_View = glm::inverse(m_Transform.GetTransformation());
        }

        return m_View;
    }

    glm::mat4 Camera::GetProjectionMatrix() const
    {
        if(m_DirtyFlag)
        {
            m_Projection = glm::perspective(m_Settings.fov, m_Settings.aspectRatio, m_Settings.nearPlane, m_Settings.farPlane);
            m_DirtyFlag = false;
        }

        return m_Projection;
    }

    void Camera::SetProjection(float a_Fov, float a_AspectRatio, float a_NearPlane, float a_FarPlane)
    {
        m_DirtyFlag = true;
        m_Settings.fov = a_Fov;
        m_Settings.aspectRatio = a_AspectRatio;
        m_Settings.nearPlane = a_NearPlane;
        m_Settings.farPlane = a_FarPlane;
    }

    Transform& Camera::GetTransform()
    {
        return m_Transform;
    }
}
