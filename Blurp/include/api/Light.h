#pragma once
#include "RenderResource.h"

namespace blurp
{
    struct DirectionalLightData
    {
        glm::vec4 colorIntensity;
        glm::vec4 directionShadowMapIndex;
    };

    struct SpotLightData
    {
        glm::vec4 colorIntensity;
        glm::vec4 directionAngle;
        glm::vec4 positionShadowMapIndex;
    };

    struct PointLightData
    {
        glm::vec4 colorIntensity;
        glm::vec4 positionShadowMapIndex;
    };

    /*
     * Normal ambient light.
     */
    class Light : public RenderResource
    {
    public:
        Light(const LightSettings& a_Settings) : m_Settings(a_Settings) {}
        virtual ~Light() = default;

        /*
         * Get the type of this light.
         */
        LightType GetType() const;

        /*
         * Get the color of this light.
         */
        virtual glm::vec3 GetColor() const = 0;

        /*
         * Set the color of the light.
         */
        virtual void SetColor(const glm::vec3& a_Color) = 0;

        /*
         * Get the intensity of the light.
         */
        virtual float GetIntensity() const = 0;

        /*
         * Set the intensity of the light.
         */
        virtual void SetIntensity(float a_Intensity) = 0;

        /*
         * Get the index of the shadowmap for this light.
         * When -1, no shadows are cast.
         *
         * Shadowmap indices are separate for positional and directional lights.
         * This means that a directional and spotlight can both use the index 0, as they refer to different shadow map arrays.
         */
        virtual std::int32_t GetShadowMapIndex() const = 0;

        /*
         * Set the index of the shadowmap that this light uses.
         * Set to -1 to not cast shadows.
         *
         * Shadowmap indices are separate for positional and directional lights.
         * This means that a directional and spotlight can both use the index 0, as they refer to different shadow map arrays.
         */
        virtual void SetShadowMapIndex(std::int32_t a_ShadowMapIndex) = 0;

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;

    protected:
        LightSettings m_Settings;
    };

    class AmbientLight : public Light
    {
    public:
        explicit AmbientLight(const LightSettings& a_Settings)
            : Light(a_Settings), m_Intensity(1.f), m_Color(0.f)
        {
        }

        glm::vec3 GetColor() const override;
        void SetColor(const glm::vec3& a_Color) override;
        float GetIntensity() const override;
        void SetIntensity(float a_Intensity) override;
        std::int32_t GetShadowMapIndex() const override;
        void SetShadowMapIndex(std::int32_t a_ShadowMapIndex) override;

    private:
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
            : Light(a_Settings), m_Data({ { a_Settings.color, a_Settings.intensity}, {a_Settings.directionalLight.direction, a_Settings.shadowMapIndex}})
        {
            //Ensure that the direction is normalized.
            assert(fabs(glm::length(glm::vec3(m_Data.directionShadowMapIndex)) - 1.f) <= std::numeric_limits<float>::epsilon() * 3.f);
        }

        /*
         * Get the direction of this directional light.
         */
        glm::vec3 GetDirection() const;

        /*
         * Set the direction of this directional light.
         * This has to be normalized.
         */
        void SetDirection(const glm::vec3& a_Direction);

        glm::vec3 GetColor() const override;
        void SetColor(const glm::vec3& a_Color) override;
        float GetIntensity() const override;
        void SetIntensity(float a_Intensity) override;
        std::int32_t GetShadowMapIndex() const override;
        void SetShadowMapIndex(std::int32_t a_ShadowMapIndex) override;

        DirectionalLightData GetData() const;

    private:
        DirectionalLightData m_Data;
    };

    /*
     * Spot Light.
     */
    class SpotLight : public Light
    {
    public:
        SpotLight(const LightSettings& a_Settings)
            : Light(a_Settings), m_Data({ {a_Settings.color, a_Settings.intensity}, {a_Settings.spotLight.direction, a_Settings.spotLight.angle}, {a_Settings.spotLight.position, a_Settings.shadowMapIndex}})
        {
            //Direction has to be normalized.
            assert(fabs(glm::length(glm::vec3(m_Data.directionAngle)) - 1.f) <= std::numeric_limits<float>::epsilon() * 3.f);
        }

        /*
         * Set the angle of this spotlight.
         * This is in radians and determines the light cone width.
         */
        void SetAngle(float a_Angle);

        /*
         * Get the angle of this spotlight.
         * This is in radians.
         */
        float GetAngle() const;

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

        glm::vec3 GetColor() const override;
        void SetColor(const glm::vec3& a_Color) override;
        float GetIntensity() const override;
        void SetIntensity(float a_Intensity) override;
        std::int32_t GetShadowMapIndex() const override;
        void SetShadowMapIndex(std::int32_t a_ShadowMapIndex) override;

        SpotLightData GetData() const;

    private:
        SpotLightData m_Data;
    };

    /*
     * Point Light.
     */
    class PointLight : public Light
    {
    public:
        PointLight(const LightSettings& a_Settings)
            : Light(a_Settings), m_Data({ { a_Settings.color, a_Settings.intensity},  {a_Settings.pointLight.position, a_Settings.shadowMapIndex}}) {}

        /*
         * Get the light position.
         */
        void SetPosition(const glm::vec3& a_Position);

        /*
         * Set the light position.
         */
        glm::vec3 GetPosition() const;

        glm::vec3 GetColor() const override;
        void SetColor(const glm::vec3& a_Color) override;
        float GetIntensity() const override;
        void SetIntensity(float a_Intensity) override;
        std::int32_t GetShadowMapIndex() const override;
        void SetShadowMapIndex(std::int32_t a_ShadowMapIndex) override;

        PointLightData GetData() const;

    private:
        PointLightData m_Data;
    };
}
