#pragma once
#include "RenderResource.h"

namespace blurp
{
    /*
     * Normal ambient light.
     */
    class Light : public RenderResource
    {
    public:
        Light(const LightSettings& a_Settings) : m_Settings(a_Settings), m_Color(m_Settings.color), m_Intensity(m_Settings.intensity){}

        /*
         * Get the type of this light.
         */
        LightType GetType() const;

        /*
         * Get the color of this light.
         */
        glm::vec3 GetColor() const;

        /*
         * Set the color of the light.
         */
        void SetColor(const glm::vec3& a_Color);

        /*
         * Get the intensity of the light.
         */
        float GetIntensity() const;

        /*
         * Set the intensity of the light.
         */
        void SetIntensity(float a_Intensity);

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;

    protected:
        LightSettings m_Settings;
        glm::vec3 m_Color;
        float m_Intensity;
    };

    /*
     * Directional Light.
     */
    class DirectionalLight : public Light
    {
    public:
        DirectionalLight(const LightSettings& a_Settings)
            : Light(a_Settings), m_Direction({-1.f, -1.f, -1.f}) {}

        /*
         * Get the direction of this directional light.
         */
        glm::vec3 GetDirection() const;

        /*
         * Set the direction of this directional light.
         * This has to be normalized.
         */
        void SetDirection(const glm::vec3& a_Direction);

    private:
        glm::vec3 m_Direction;
    };

    /*
     * Spot Light.
     */
    class SpotLight : public Light
    {
    public:
        SpotLight(const LightSettings& a_Settings)
            : Light(a_Settings), m_Position(0.f, 0.f, 0.f), m_Direction({ 0.f, -1.f, 0.f }), m_Angle(45.f) {}

        /*
         * Set the angle of this spotlight.
         * This is in degrees and determines the light cone width.
         */
        void SetAngle(float a_Angle);

        /*
         * Get the angle of this spotlight.
         * This is in degrees.
         */
        float GetAngle(float a_Angle) const;

        /*
         * Get the light position.
         */
        void SetPosition(const glm::vec3& a_Position);

        /*
         * Set the light position.
         */
        glm::vec3 GetPosition() const;

        /*
         * Set the light direction.
         * This has to be normalized.
         */
        void SetDirection(const glm::vec3& a_Direction);

        /*
         * Get the light direction.
         */
        glm::vec3 GetDirection() const;

    private:
        glm::vec3 m_Position;
        glm::vec3 m_Direction;
        float m_Angle;
    };

    /*
     * Point Light.
     */
    class PointLight : public Light
    {
    public:
        PointLight(const LightSettings& a_Settings)
            : Light(a_Settings), m_Position(0.f, 0.f, 0.f) {}

        /*
         * Get the light position.
         */
        void SetPosition(const glm::vec3& a_Position);

        /*
         * Set the light position.
         */
        glm::vec3 GetPosition() const;

    private:
        glm::vec3 m_Position;
    };
}
