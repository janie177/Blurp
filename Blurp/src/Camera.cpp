#include "Camera.h"

namespace blurp
{
    glm::mat4 Camera::GetViewMatrix() const
    {
        //Rebuild the inverse if required.
        if(m_RebuildView)
        {
           m_View = glm::inverse(m_Transform.GetTransformation());
           m_RebuildView = false;
        }

        return m_View;
    }

    glm::mat4 Camera::GetProjectionMatrix() const
    {
        if(m_DirtyFlag)
        {
            if(m_Settings.projectionMode == ProjectionMode::PERSPECTIVE)
            {
                m_Projection = glm::perspective(glm::radians(m_Settings.fov), m_Settings.width / m_Settings.height, m_Settings.nearPlane, m_Settings.farPlane);
            }
            else
            { 
                m_Projection = glm::ortho(0.0f, m_Settings.width, m_Settings.height, 0.0f, 0.1f, 100.0f);
            }
            
            m_DirtyFlag = false;
        }

        return m_Projection;
    }

    void Camera::UpdateSettings(const CameraSettings& a_Settings)
    {
        m_Settings = a_Settings;
        m_DirtyFlag = true;
    }

    const CameraSettings& Camera::GetSettings() const
    {
        return m_Settings;
    }

    Transform& Camera::GetTransform()
    {
        //Accessed from a non-const context. Changes may be made so rebuild.
        m_RebuildView = true;
        return m_Transform;
    }

    const Transform& Camera::GetTransform() const
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
