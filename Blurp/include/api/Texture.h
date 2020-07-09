#pragma once
#include "RenderResource.h"

namespace blurp
{
    class Texture : public RenderResource
    {
    public:
        Texture(const TextureSettings& a_Settings) : m_Settings(a_Settings) {}

    protected:
        TextureSettings m_Settings;

    };
}
