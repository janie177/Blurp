#include "Light.h"

namespace blurp
{
    LightType Light::GetType() const
    {
        return m_Settings.type;
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

    glm::vec3 AmbientLight::GetColor() const
    {
        return m_Color;
    }

    void AmbientLight::SetColor(const glm::vec3& a_Color)
    {
        assert(a_Color.x >= 0.f && a_Color.x <= 1.f);
        assert(a_Color.y >= 0.f && a_Color.y <= 1.f);
        assert(a_Color.z >= 0.f && a_Color.z <= 1.f);
        m_Color = a_Color;
    }

    float AmbientLight::GetIntensity() const
    {
        return m_Intensity;
    }

    void AmbientLight::SetIntensity(float a_Intensity)
    {
        assert(a_Intensity >= 0.f);
        m_Intensity = a_Intensity;
    }

    std::int32_t AmbientLight::GetShadowMapIndex() const
    {
        return -1;
    }

    void AmbientLight::SetShadowMapIndex(std::int32_t a_ShadowMapIndex)
    {
        //Nothing here.
    }

    glm::vec3 DirectionalLight::GetDirection() const
    {
        return m_Data.directionShadowMapIndex;
    }

    void DirectionalLight::SetDirection(const glm::vec3& a_Direction)
    {
        //Ensure normalized.
        assert(fabs(glm::length(a_Direction) - 1.f) <= std::numeric_limits<float>::epsilon() * 3.f);
        m_Data.directionShadowMapIndex.x = a_Direction.x;
        m_Data.directionShadowMapIndex.y = a_Direction.y;
        m_Data.directionShadowMapIndex.z = a_Direction.z;
    }

    glm::vec3 DirectionalLight::GetColor() const
    {
        return m_Data.colorIntensity;
    }

    void DirectionalLight::SetColor(const glm::vec3& a_Color)
    {
        //Ensure clamped between 0 and 1.
        assert(a_Color.x >= 0.f && a_Color.x <= 1.f);
        assert(a_Color.y >= 0.f && a_Color.y <= 1.f);
        assert(a_Color.z >= 0.f && a_Color.z <= 1.f);
        m_Data.colorIntensity.x = a_Color.x;
        m_Data.colorIntensity.y = a_Color.y;
        m_Data.colorIntensity.z = a_Color.z;
    }

    float DirectionalLight::GetIntensity() const
    {
        return m_Data.colorIntensity.w;
    }

    void DirectionalLight::SetIntensity(float a_Intensity)
    {
        //Ensure intensity is positive.
        assert(a_Intensity >= 0);
        m_Data.colorIntensity.w = a_Intensity;
    }

    std::int32_t DirectionalLight::GetShadowMapIndex() const
    {
        return static_cast<std::int32_t>(m_Data.directionShadowMapIndex.w);
    }

    void DirectionalLight::SetShadowMapIndex(std::int32_t a_ShadowMapIndex)
    {
        m_Data.directionShadowMapIndex.w = static_cast<float>(a_ShadowMapIndex);
    }

    DirectionalLightData DirectionalLight::GetData() const
    {
        return m_Data;
    }

    void SpotLight::SetAngle(float a_Angle)
    {
        m_Data.directionAngle.w = a_Angle;
    }

    float SpotLight::GetAngle() const
    {
        return m_Data.directionAngle.w;
    }

    void SpotLight::SetPosition(const glm::vec3& a_Position)
    {
        m_Data.positionShadowMapIndex.x = a_Position.x;
        m_Data.positionShadowMapIndex.y = a_Position.y;
        m_Data.positionShadowMapIndex.z = a_Position.z;
    }

    glm::vec3 SpotLight::GetPosition() const
    {
        return m_Data.positionShadowMapIndex;
    }

    void SpotLight::SetDirection(const glm::vec3& a_Direction)
    {
        //Ensure normalized.
        assert(fabs(glm::length(a_Direction) - 1.f) <= std::numeric_limits<float>::epsilon() * 3.f);
        m_Data.directionAngle.x = a_Direction.x;
        m_Data.directionAngle.y = a_Direction.y;
        m_Data.directionAngle.z = a_Direction.z;
    }

    glm::vec3 SpotLight::GetDirection() const
    {
        return m_Data.directionAngle;
    }

    glm::vec3 SpotLight::GetColor() const
    {
        return m_Data.colorIntensity;
    }

    void SpotLight::SetColor(const glm::vec3& a_Color)
    {
        //Ensure clamped between 0 and 1.
        assert(a_Color.x >= 0.f && a_Color.x <= 1.f);
        assert(a_Color.y >= 0.f && a_Color.y <= 1.f);
        assert(a_Color.z >= 0.f && a_Color.z <= 1.f);
        m_Data.colorIntensity.x = a_Color.x;
        m_Data.colorIntensity.y = a_Color.y;
        m_Data.colorIntensity.z = a_Color.z;
    }

    float SpotLight::GetIntensity() const
    {
        return m_Data.colorIntensity.w;
    }

    void SpotLight::SetIntensity(float a_Intensity)
    {
        assert(a_Intensity >= 0.f);
        m_Data.colorIntensity.w = a_Intensity;
    }

    std::int32_t SpotLight::GetShadowMapIndex() const
    {
        return static_cast<std::int32_t>(m_Data.positionShadowMapIndex.w);
    }

    void SpotLight::SetShadowMapIndex(std::int32_t a_ShadowMapIndex)
    {
        m_Data.positionShadowMapIndex.w = static_cast<float>(a_ShadowMapIndex);
    }

    SpotLightData SpotLight::GetData() const
    {
        return m_Data;
    }

    void PointLight::SetPosition(const glm::vec3& a_Position)
    {
        m_Data.positionShadowMapIndex.x = a_Position.x;
        m_Data.positionShadowMapIndex.y = a_Position.y;
        m_Data.positionShadowMapIndex.z = a_Position.z;
    }

    glm::vec3 PointLight::GetPosition() const
    {
        return m_Data.positionShadowMapIndex;
    }

    glm::vec3 PointLight::GetColor() const
    {
        return m_Data.colorIntensity;
    }

    void PointLight::SetColor(const glm::vec3& a_Color)
    {
        //Ensure clamped between 0 and 1.
        assert(a_Color.x >= 0.f && a_Color.x <= 1.f);
        assert(a_Color.y >= 0.f && a_Color.y <= 1.f);
        assert(a_Color.z >= 0.f && a_Color.z <= 1.f);
        m_Data.colorIntensity.x = a_Color.x;
        m_Data.colorIntensity.y = a_Color.y;
        m_Data.colorIntensity.z = a_Color.z;
    }

    float PointLight::GetIntensity() const
    {
        return m_Data.colorIntensity.w;
    }

    void PointLight::SetIntensity(float a_Intensity)
    {
        assert(a_Intensity >= 0.f);
        m_Data.colorIntensity.w = a_Intensity;
    }

    std::int32_t PointLight::GetShadowMapIndex() const
    {
        return static_cast<std::int32_t>(m_Data.positionShadowMapIndex.w);
    }

    void PointLight::SetShadowMapIndex(std::int32_t a_ShadowMapIndex)
    {
        m_Data.positionShadowMapIndex.w = static_cast<float>(a_ShadowMapIndex);
    }

    PointLightData PointLight::GetData() const
    {
        return m_Data;
    }
}
