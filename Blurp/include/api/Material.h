#pragma once
#include "RenderResource.h"

namespace blurp
{
    class Material : public RenderResource
    {
    public:
        Material(const MaterialSettings& a_Settings) : m_Settings(a_Settings){}

        /*
         * Update this material with new settings.
         */
        void UpdateSettings(const MaterialSettings& a_Settings)
        {
            m_Settings = a_Settings;
        }

        /*
         * Get a reference to the material settings for this material.
         */
        MaterialSettings& GetSettings() { return m_Settings; };

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override { return true; };
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override { return true; };
        MaterialSettings m_Settings;
    };
}