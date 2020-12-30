#pragma once

#include <GL/glew.h>

#include "Mesh.h"

namespace blurp
{
    class Mesh_GL : public Mesh
    {
    public:
        Mesh_GL(const MeshSettings& a_Settings) : Mesh(a_Settings), m_Vao(0), m_Vbo(0), m_Ibo(0), m_NumIndices(0), m_IndexDataType(GL_UNSIGNED_SHORT) {}

        /*
         * Get the VAO for this mesh.
         */
        GLuint GetVaoId() const;

        /*
         * Get the vbo for this mesh.
         */
        GLuint GetVboId() const;

        /*
         * Get the index buffer for this mesh.
         */
        GLuint GetIboId() const;

        /*
         * Get the number of indices for this mesh.
         */
        std::uint32_t GetNumIndices() const;

        /*
         * Get the data type used for the index buffer.
         */
        GLenum GetIndexDataType() const;

        /*
         * Get the attribute location defines for the current mask.
         */
        const std::vector<std::string>& GetAttribLocations() const;

        /*
         * Get a reference to the vector containing instance divisors used by this mesh.
         * The pairs in the vector are laid out by <index, divisor>.
         * Index is the vertex attribute index, while divisor is the instance divisor.
         */
        const std::vector<std::pair<std::uint32_t, std::uint32_t>>& GetInstanceDivisors() const;

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;

    private:
        GLuint m_Vao;
        GLuint m_Vbo;
        GLuint m_Ibo;
        std::uint32_t m_NumIndices;
        GLenum m_IndexDataType;

        //Shader compiling flags to set the right layout index per attribute.
        std::vector<std::string> m_VertexPosDefines;

        //Pairs of <index, divisor> for instancing. This has to be multiplied dynamically when rendering with dynamic matrices.
        std::vector<std::pair<std::uint32_t, std::uint32_t>> m_InstancedVertexAttributes;
    };
}