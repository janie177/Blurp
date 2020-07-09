#pragma once
#include "RenderResource.h"

namespace blurp
{
    class Light : public RenderResource
    {
    public:
        Light(const LightSettings& a_Settings) : m_Settings(a_Settings){}

    protected:
        LightSettings m_Settings;
    };
}
