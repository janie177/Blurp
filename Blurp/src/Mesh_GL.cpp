#include "opengl/Mesh_GL.h"


#include <iostream>
#include <ostream>


#include "opengl/GLUtils.h"

namespace blurp
{
    GLuint Mesh_GL::GetVaoId() const
    {
        return m_Vao;
    }

    GLuint Mesh_GL::GetVboId() const
    {
        return m_Vbo;
    }

    GLuint Mesh_GL::GetIboId() const
    {
        return m_Ibo;
    }

    std::uint32_t Mesh_GL::GetNumIndices() const
    {
        return m_NumIndices;
    }

    GLenum Mesh_GL::GetIndexDataType() const
    {
        return m_IndexDataType;
    }

    bool Mesh_GL::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        assert((m_Settings.indexDataType == DataType::USHORT || m_Settings.indexDataType == DataType::UINT) && "Index buffer data type has to be either UINT or USHORT.");

        //Create the vao and add the data to it.
        glGenVertexArrays(1, &m_Vao);
        glBindVertexArray(m_Vao);

        //Create the vbo containing all data.
        glGenBuffers(1, &m_Vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);

        //How will this memory be used? Will it be written to often or is it static?
        const auto memoryUsage = ToGL(m_Settings.usage);

        glGenBuffers(1, &m_Ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ibo);
        m_IndexDataType = ToGL(m_Settings.indexDataType);
        const auto iboDataSize = sizeof(m_IndexDataType);

        //Upload the index buffer in the right format.
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Settings.numIndices * iboDataSize, m_Settings.indexData, memoryUsage);
        m_NumIndices = m_Settings.numIndices;

        //Upload the vertex data in the right format.
        glBufferData(GL_ARRAY_BUFFER, m_Settings.vertexDataSizeBytes, m_Settings.vertexData, memoryUsage);

        /*
         * Enable the vertex attributes that are set enabled.
         * The index is always the same for each attribute, no matter which ones are enabled.
         * GLSL shaders should always specify each one at the order they are specified in VertexAttribute.
         * Disabled ones will simply not have a value in the shader.
         * Logic related to using these variables can be enabled and disabled through the use of ifdefs.
         */
        int index = 0;

        for (const auto attrib : VERTEX_ATTRIBUTES)
        {
            const auto info = VertexSettings::GetVertexAttributeInfo(attrib);
            const auto numIndicesRequired = ((info.numElements - 1) / 4) + 1;

            if (m_Settings.vertexSettings.IsEnabled(attrib))
            {
                const auto data = m_Settings.vertexSettings.GetAttributeData(attrib);
                const auto glDataType = ToGL(info.dataType);
                int elementsLeft = info.numElements;
                GLenum normalize = ToGL(data.normalize);
                for(std::uint32_t i = 0; i < numIndicesRequired; ++i)
                {
                    glVertexAttribPointer(index + static_cast<int>(i), elementsLeft <= 4 ? elementsLeft : 4, glDataType, normalize, data.byteStride, reinterpret_cast<void*>(static_cast<std::uint64_t>(data.byteOffset + (static_cast<std::uint64_t>(i) * 4L * sizeof(glDataType)))));
                    glEnableVertexAttribArray(index + i);

                    if(attrib == VertexAttribute::MATRIX)
                    {
                        std::cout << "Enabling matrix at index: " << (index + i) << " with " << (elementsLeft <= 4 ? elementsLeft : 4) << " elements. Stride is " << data.byteStride << " bytes and offset is " << static_cast<std::uint64_t>(data.byteOffset + (static_cast<std::uint64_t>(i) * 4L * sizeof(glDataType))) << std::endl;
                    }

                    elementsLeft -= 4;

                    //Enable instancing if specified.
                    if (data.instanceDivisor != 0)
                    {
                        glVertexAttribDivisor(index + i, data.instanceDivisor);
                    }
                }
            }

            //Increment index.
            index += numIndicesRequired;
        }

        //Unbind.
        glBindVertexArray(0);

        return true;
    }

    bool Mesh_GL::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        glDeleteBuffers(1, &m_Vbo);
        glDeleteBuffers(1, &m_Ibo);
        glDeleteBuffers(1, &m_Vao);
        return true;
    }
}
