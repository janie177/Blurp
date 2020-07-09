#pragma once
#include "RenderResource.h"

namespace blurp
{
    class Material : public RenderResource
    {
    public:
        Material(const MaterialSettings& a_Settings) : m_Settings(a_Settings){}

    protected:
        MaterialSettings m_Settings;
    };
}
