#pragma once

#include <GL/glew.h>

#include "Mesh.h"

namespace blurp
{
    class Mesh_GL : public Mesh
    {
    public:
        Mesh_GL(const MeshSettings& a_Settings) : Mesh(a_Settings), m_Vao(0), m_Vbo(0), m_Ibo(0), m_NumIndices(0) {}

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;

    private:
        GLuint m_Vao;
        GLuint m_Vbo;
        GLuint m_Ibo;
        std::uint32_t m_NumIndices;
    };
}