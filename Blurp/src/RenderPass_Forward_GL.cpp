#include "opengl/RenderPass_Forward_GL.h"
#include "BlurpEngine.h"
#include "FileReader.h"
#include "opengl/Mesh_GL.h"
#include "opengl/RenderTarget_GL.h"
#include "RenderResourceManager.h"

namespace blurp
{
    bool RenderPass_Forward_GL::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        //Load the shaders from file.
        auto shaderPath = a_BlurpEngine.GetEngineSettings().shadersPath + "opengl/";
        auto vertexShaderPath = shaderPath + "Default_Forward.vs";
        auto fragmentShaderPath = shaderPath + "Default_Forward.fs";

        FileReader vsReader(vertexShaderPath);
        FileReader fsReader(fragmentShaderPath);

        if(!vsReader.Open() || !fsReader.Open())
        {
            std::string str = "Can't find forward shaders! Is the shader path configured correctly? " + shaderPath;
            throw std::exception(str.c_str());
        }

        //Read the source from the files and compile the shaders.
        ShaderSettings shaderSettings;
        shaderSettings.type = ShaderType::GRAPHICS;

        auto vertexSrc = vsReader.ToArray();
        auto fragmentSrc = fsReader.ToArray();
        shaderSettings.vertexShaderSource = vertexSrc.get();
        shaderSettings.fragmentShaderSource = fragmentSrc.get();

        m_Shader = std::reinterpret_pointer_cast<Shader_GL>(a_BlurpEngine.GetResourceManager().CreateShader(shaderSettings));

        glGenBuffers(1, &m_Ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_Ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, 152, nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_Ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        

        return true;
    }

    bool RenderPass_Forward_GL::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        glDeleteBuffers(1, &m_Ssbo);
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
        glUseProgram(std::reinterpret_pointer_cast<Shader_GL>(m_Shader)->GetProgramId());

        //Camera matrices.
        const auto projection = m_Camera->GetProjectionMatrix();
        const auto view = m_Camera->GetViewMatrix();
        const auto pv = projection * view;

        //Bind the uniform buffer.
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_Ssbo);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        for(auto& instanceData : m_DrawQueue)
        {
            const Mesh_GL* mesh = static_cast<Mesh_GL*>(instanceData.mesh);
            glBindVertexArray(mesh->GetVaoId());

            std::vector<MeshInstanceData> transformed;
            transformed.reserve(instanceData.count);

            for(std::uint32_t i = 0; i < instanceData.count; ++i)
            {
                auto& t = transformed.emplace_back(MeshInstanceData());
                t.transform = pv * instanceData.transform[i];
            }

            glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(MeshInstanceData) * instanceData.count, &transformed[0], GL_DYNAMIC_DRAW);

            glDrawElementsInstanced(GL_TRIANGLES, mesh->GetNumIndices(), mesh->GetIndexDataType(), nullptr, instanceData.count);
        }

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        glBindVertexArray(0);
    }
}
