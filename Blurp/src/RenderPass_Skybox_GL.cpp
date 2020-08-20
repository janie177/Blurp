#include "opengl/RenderPass_Skybox_GL.h"

#include <glm/gtc/type_ptr.hpp>


#include "FileReader.h"
#include "BlurpEngine.h"
#include "opengl/Mesh_GL.h"
#include "opengl/RenderTarget_GL.h"
#include "RenderResourceManager.h"
#include "opengl/Texture_GL.h"

namespace blurp
{
    bool RenderPass_Skybox_GL::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        auto shaderPath = a_BlurpEngine.GetEngineSettings().shadersPath + "opengl/";
        auto vertex = "Default_Skybox.vs";
        auto fragment = "Default_Skybox.fs";

        FileReader vertexReader(shaderPath + vertex);
        FileReader fragmentReader(shaderPath + fragment);

        if (!vertexReader.Open() || !fragmentReader.Open())
        {
            throw std::exception("Could not find forward shaders at path specified.");
        }

        auto vertexSrc = vertexReader.ToArray();
        auto fragmentSrc = fragmentReader.ToArray();

        ShaderSettings sSettings;
        sSettings.vertexShaderSource = vertexSrc.get();
        sSettings.fragmentShaderSource = fragmentSrc.get();
        sSettings.type = ShaderType::GRAPHICS;

        m_Shader = std::reinterpret_pointer_cast<Shader_GL>(a_BlurpEngine.GetResourceManager().CreateShader(sSettings));

        //Load the cube mesh.
        MeshSettings meshSettings;
        meshSettings.indexDataType = DataType::USHORT;
        meshSettings.indexData = CUBE_INDICES;
        meshSettings.numIndices = sizeof(CUBE_INDICES) / sizeof(CUBE_INDICES[0]);
        meshSettings.vertexData = CUBE_DATA;
        meshSettings.access = AccessMode::READ_ONLY;
        meshSettings.usage = MemoryUsage::GPU;
        meshSettings.vertexDataSizeBytes = sizeof(CUBE_DATA);
        meshSettings.vertexSettings.EnableAttribute(VertexAttribute::POSITION_3D, 0, 0, 0);

        m_CubeMesh = a_BlurpEngine.GetResourceManager().CreateMesh(meshSettings);

        return true;
    }

    bool RenderPass_Skybox_GL::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        return true;
    }

    void RenderPass_Skybox_GL::Execute()
    {
        //Clear the target buffer.
        const auto fbGl = reinterpret_cast<RenderTarget_GL*>(m_Target.get());

        //Bind the FBO and set the viewport / scissorrect.
        fbGl->Bind();

        //Don't do depth writing.
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);

        //Calculate the pv matrix.
        auto pv = m_Camera->GetProjectionMatrix() * glm::mat4(glm::mat3(m_Camera->GetViewMatrix()));

        //Bind the shader and upload the pv matrix.
        glUseProgram(m_Shader->GetProgramId());
        glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(pv));

        glUniform3f(1, m_MixColor.x, m_MixColor.y, m_MixColor.z);
        glUniform3f(2, m_ColorMultiplier.x, m_ColorMultiplier.y, m_ColorMultiplier.z);
        glUniform1f(3, m_Opacity);

        //Bind the texture
        auto texture = static_cast<Texture_GL*>(m_Texture.get());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture->GetTextureId());

        //Bind the cube mesh.
        Mesh_GL* mesh = static_cast<Mesh_GL*>(m_CubeMesh.get());
        glBindVertexArray(mesh->GetVaoId());

        //Draw
        glDrawElements(GL_TRIANGLES, mesh->GetNumIndices(), mesh->GetIndexDataType(), nullptr);

        //Reset state.
        glDepthMask(true);
        glBindVertexArray(0);

    }
}
