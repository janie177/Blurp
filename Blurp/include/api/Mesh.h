#pragma once
#include "RenderResource.h"

namespace blurp
{
    class Mesh : public RenderResource
    {
    public:
        Mesh(const MeshSettings& a_Settings) : m_Settings(a_Settings), m_Mask(a_Settings.vertexSettings.GetMask()){}

        /*
         * Get the vertex attribute mask for this mesh.
         * This is a combination of all enabled vertex attributes.
         */
        VertexAttribute GetVertexAttributeMask() const
        {
            return m_Mask;
        }

    protected:
        MeshSettings m_Settings;
        VertexAttribute m_Mask;
    };

}
