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
            if(m_Settings.projectionMode == ProjectionMode::PERSPECTIVE)
            {
                m_Projection = glm::perspective(m_Settings.fov, m_Settings.width / m_Settings.height, m_Settings.nearPlane, m_Settings.farPlane);
            }
            else
            {
                m_Projection = glm::ortho(0.0f, m_Settings.width, m_Settings.height, 0.0f, 0.1f, 100.0f);
            }
            
            m_DirtyFlag = false;
        }

        return m_Projection;
    }

    void Camera::SetProjection(const CameraSettings& a_Settings)
    {
        m_Settings = a_Settings;
    }

    Transform& Camera::GetTransform()
    {
        return m_Transform;
    }

    bool Camera::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        //Nothing here.
        return true;
    }

    bool Camera::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        //Nothing here.
        return true;
    }
}
