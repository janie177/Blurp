#pragma once
#include "RenderResource.h"

namespace blurp
{
    class Mesh : public RenderResource
    {
    public:
        Mesh(const MeshSettings& a_Settings) : m_Settings(a_Settings){}

    protected:
        MeshSettings m_Settings;
    };

}
