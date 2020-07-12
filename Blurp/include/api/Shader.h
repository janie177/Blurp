#pragma once
#include "RenderResource.h"

namespace blurp
{
    /*
     * Shader encapsulates all shader stages in a single pipeline.
     */
    class Shader : public RenderResource
    {
    public:
        Shader(const ShaderSettings& a_Settings) : m_Settings(a_Settings) {}

        ShaderType GetType() const
        {
            return m_Settings.type;
        }

    protected:
        ShaderSettings m_Settings;
    };
}
