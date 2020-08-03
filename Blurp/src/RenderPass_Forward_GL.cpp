#include "opengl/RenderPass_Forward_GL.h"
#include "BlurpEngine.h"
#include "FileReader.h"
#include "opengl/GpuBuffer_GL.h"
#include "opengl/Mesh_GL.h"
#include "opengl/RenderTarget_GL.h"
#include "opengl/Shader_GL.h"
#include "RenderResourceManager.h"

namespace blurp
{
    bool RenderPass_Forward_GL::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        auto shaderPath = a_BlurpEngine.GetEngineSettings().shadersPath + "opengl/";
        auto vertex =  "Default_Forward.vs";
        auto fragment = "Default_Forward.fs";

        FileReader vertexReader(shaderPath + vertex);
        FileReader fragmentReader(shaderPath + fragment);

        if(!vertexReader.Open() || !fragmentReader.Open())
        {
            throw std::exception("Could not find forward shaders at path specified.");
        }

        auto vertexSrc = vertexReader.ToArray();
        auto fragmentSrc = fragmentReader.ToArray();

        ShaderSettings sSettings;
        sSettings.vertexShaderSource = vertexSrc.get();
        sSettings.fragmentShaderSource = fragmentSrc.get();
        sSettings.type = ShaderType::GRAPHICS;

        //Preprocessor definitions at their respective bit indices.
        std::vector<std::string> definitions;

        //Add every mesh bitmask to it as the first set of bits.
        for(auto& attrib : VERTEX_ATTRIBUTES)
        {
            definitions.emplace_back(VertexSettings::GetVertexAttributeInfo(attrib).defineName);
        }

        m_ShaderCache.Init(a_BlurpEngine.GetResourceManager(), sSettings, definitions);

        return true;
    }

    bool RenderPass_Forward_GL::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        return true;
    }

    void RenderPass_Forward_GL::Execute()
    {
        //Clear the target buffer.
        const auto fboId = reinterpret_cast<RenderTarget_GL*>(m_Output.get())->GetFrameBufferId();

        //Set output data targets and reset the framebuffer color and depth.
        glBindFramebuffer(GL_FRAMEBUFFER, fboId);
        const auto viewPort = m_Output->GetViewPort();
        glViewport(static_cast<int>(viewPort.r), static_cast<int>(viewPort.g), static_cast<int>(viewPort.b), static_cast<int>(viewPort.a));
        const auto scissorRect = m_Output->GetScissorRect();
        glScissor(static_cast<int>(scissorRect.r), static_cast<int>(scissorRect.g), static_cast<int>(scissorRect.b), static_cast<int>(scissorRect.a));
        const auto clearColor = m_Output->GetClearColor();
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Camera matrices.
        const auto projection = m_Camera->GetProjectionMatrix();
        const auto view = m_Camera->GetViewMatrix();
        const auto pv = projection * view;

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        //Shader attribute mask.
        auto mask = static_cast<VertexAttribute>(0);

        for(auto& instanceData : m_DrawQueue)
        {
            const Mesh_GL* mesh = static_cast<Mesh_GL*>(instanceData.mesh);

            const auto glGpuBuffer = std::reinterpret_pointer_cast<GpuBuffer_GL>(m_GpuBuffer);

            //If the current mask is not the same as the one needed, switch shader.
            if(mask != mesh->GetVertexAttributeMask())
            {
                //Bind the new shader.
                mask = mesh->GetVertexAttributeMask();
                const std::shared_ptr<Shader_GL> currentShader = std::reinterpret_pointer_cast<Shader_GL>(m_ShaderCache.GetShader(mesh->GetVertexAttributeMask()));
                glUseProgram(currentShader->GetProgramId());

                //Link the shader to the ssbo binding point.
                glShaderStorageBlockBinding(currentShader->GetProgramId(), 0, glGpuBuffer->GetBufferBaseBinding());
            }

            //Set the binding point that the shader interface block reads from to contain a specific range from the GPU buffer.
            glBindBufferRange(GL_SHADER_STORAGE_BUFFER, glGpuBuffer->GetBufferBaseBinding(), glGpuBuffer->GetBufferId(), static_cast<GLintptr>(instanceData.dataRange.start), instanceData.dataRange.size);

            //Set the number of instances from the mesh itself in the uniform.
            glUniform1i(0, mesh->GetInstanceCount());

            //Bind the VAO of the mesh.
            glBindVertexArray(mesh->GetVaoId());

            glDrawElementsInstanced(GL_TRIANGLES, mesh->GetNumIndices(), mesh->GetIndexDataType(), nullptr, instanceData.count * mesh->GetInstanceCount());
        }

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        glBindVertexArray(0);
    }
}
