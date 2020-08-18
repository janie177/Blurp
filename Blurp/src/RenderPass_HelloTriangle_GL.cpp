#include "opengl/RenderPass_HelloTriangle_GL.h"
#include "RenderTarget.h"
#include <iostream>

#include "opengl/RenderTarget_GL.h"

#include "BlurpEngine.h"
#include "RenderResourceManager.h"
#include "opengl/Shader_GL.h"
#include "opengl/Texture_GL.h"

namespace blurp
{
    bool RenderPass_HelloTriangle_GL::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        //Shader program
        ShaderSettings shaderSettings;
        shaderSettings.type = ShaderType::GRAPHICS;
        shaderSettings.vertexShaderSource = vertexShaderSource;
        shaderSettings.fragmentShaderSource = fragmentShaderSource;

        //Preprocessor definitions test here.
        bool red = false;
        if(red)
        {
            shaderSettings.preprocessorDefinitions.emplace_back("RED");
        }

        m_Shader = a_BlurpEngine.GetResourceManager().CreateShader(shaderSettings);

        float vertices[] = {
            -0.5f, -0.5f, 0.0f, //XYZ
             0.0f, 0.0f,          //UV
             0.5f, -0.5f, 0.0f,
             1.0, 0.0f,          //UV
             0.0f,  0.5f, 0.0f,
             0.5, 1.0f,          //UV
        };

        glGenVertexArrays(1, &m_Vao);
        glGenBuffers(1, &m_Vbo);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(m_Vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)12);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
        // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
        glBindVertexArray(0);

        //Get the uniform id
        m_ColorUniformId = glGetUniformLocation(std::reinterpret_pointer_cast<Shader_GL>(m_Shader)->GetProgramId(), "ucolor");

        return true;
    }

    bool RenderPass_HelloTriangle_GL::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        glDeleteVertexArrays(1, &m_Vao);
        glDeleteBuffers(1, &m_Vbo);

        return true;
    }

    void RenderPass_HelloTriangle_GL::Execute()
    {
        //Clear the target buffer.
        const auto fboId = reinterpret_cast<RenderTarget_GL*>(m_Target.get())->GetFrameBufferId();

        glBindFramebuffer(GL_FRAMEBUFFER, fboId);
        const auto viewPort = m_Target->GetViewPort();
        glViewport(static_cast<int>(viewPort.r), static_cast<int>(viewPort.g), static_cast<int>(viewPort.b), static_cast<int>(viewPort.a));

        const auto scissorRect = m_Target->GetScissorRect();
        glScissor(static_cast<int>(scissorRect.r), static_cast<int>(scissorRect.g), static_cast<int>(scissorRect.b), static_cast<int>(scissorRect.a));

        const auto clearColor = m_Target->GetClearColor();
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(std::reinterpret_pointer_cast<Shader_GL>(m_Shader)->GetProgramId());
        glBindVertexArray(m_Vao);

        glUniform4f(m_ColorUniformId, m_Color.r, m_Color.g, m_Color.b, m_Color.a);

        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
}
