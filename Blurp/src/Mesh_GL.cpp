#include "opengl/Mesh_GL.h"

#include "opengl/GLUtils.h"

namespace blurp
{
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
        const auto iboDataSize = sizeof(ToGL(m_Settings.indexDataType));

        //Upload the index buffer in the right format.
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Settings.numIndices * iboDataSize, m_Settings.indexData, memoryUsage);
        m_NumIndices = m_Settings.numIndices;

        //Upload the vertex data in the right format.
        glBufferData(GL_ARRAY_BUFFER, m_Settings.vertexDataSizeBytes, m_Settings.vertexData, memoryUsage);

        /*
         * Enable the vertex attributes that are set enabled.
         */
        int index = 0;
        for (const auto attrib : VERTEX_ATTRIBUTES)
        {
            if (m_Settings.vertexSettings.IsEnabled(attrib))
            {
                const auto info = VertexSettings::GetVertexAttributeInfo(attrib);
                const auto data = m_Settings.vertexSettings.GetAttributeData(attrib);
                const auto glDataType = ToGL(info.dataType);
                    
                glVertexAttribPointer(index, info.numElements, glDataType, ToGL(data.normalize), data.byteStride, (void*)data.byteOffset);
                glEnableVertexAttribArray(index);

                ++index;
            }
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
