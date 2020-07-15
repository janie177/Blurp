#include "Light.h"

namespace blurp
{
    LightType Light::GetType() const
    {
        return m_Settings.type;
    }

    glm::vec3 Light::GetColor() const
    {
        return m_Color;
    }

    void Light::SetColor(const glm::vec3& a_Color)
    {
        m_Color = a_Color;
    }

    float Light::GetIntensity() const
    {
        return m_Intensity;
    }

    void Light::SetIntensity(float a_Intensity)
    {
        m_Intensity = a_Intensity;
    }

    bool Light::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        //Nothing to do here.
        return true;
    }

    bool Light::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        //Nothing to do here.
        return true;
    }

    glm::vec3 DirectionalLight::GetDirection() const
    {
        return m_Direction;
    }

    void DirectionalLight::SetDirection(const glm::vec3& a_Direction)
    {
        m_Direction = a_Direction;
    }

    void SpotLight::SetAngle(float a_Angle)
    {
        m_Angle = a_Angle;
    }

    float SpotLight::GetAngle(float a_Angle) const
    {
        return m_Angle;
    }

    void SpotLight::SetPosition(const glm::vec3& a_Position)
    {
        m_Position = a_Position;
    }

    glm::vec3 SpotLight::GetPosition() const
    {
        return m_Position;
    }

    void SpotLight::SetDirection(const glm::vec3& a_Direction)
    {
        m_Direction = a_Direction;
    }

    glm::vec3 SpotLight::GetDirection() const
    {
        return m_Direction;
    }

    void PointLight::SetPosition(const glm::vec3& a_Position)
    {
        m_Position = a_Position;
    }

    glm::vec3 PointLight::GetPosition() const
    {
        return m_Position;
    }
}
