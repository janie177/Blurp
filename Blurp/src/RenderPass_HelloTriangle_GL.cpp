#include "opengl/RenderPass_HelloTriangle_GL.h"
#include "RenderTarget.h"
#include <iostream>

#include "opengl/RenderTarget_GL.h"

namespace blurp
{
    bool RenderPass_HelloTriangle_GL::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        std::cout << "Now loading hello triangle gl!" << std::endl;

        //Shader program
        int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        // check for shader compile errors
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        // fragment shader
        int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        // check for shader compile errors
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        // link shaders
        shader = glCreateProgram();
        glAttachShader(shader, vertexShader);
        glAttachShader(shader, fragmentShader);
        glLinkProgram(shader);
        // check for linking errors
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);


        float vertices[] = {
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
             0.0f,  0.5f, 0.0f
        };

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
        // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
        glBindVertexArray(0);

        //Get the uniform id
        colorUniformID = glGetUniformLocation(shader, "ucolor");


        std::cout << "Loaded hello triangle gl!" << std::endl;

        return true;
    }

    bool RenderPass_HelloTriangle_GL::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        std::cout << "Destroyed hello triangle gl!" << std::endl;

        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteProgram(shader);

        return true;
    }

    void RenderPass_HelloTriangle_GL::Execute()
    {
        //Clear the target buffer.
        glBindBuffer(GL_FRAMEBUFFER, reinterpret_cast<RenderTarget_GL*>(m_Target.get())->GetFrameBufferID());
        const auto viewPort = m_Target->GetViewPort();
        glViewport(viewPort.r, viewPort.g, viewPort.b, viewPort.a);

        const auto scissorRect = m_Target->GetScissorRect();
        glScissor(scissorRect.r, scissorRect.g, scissorRect.b, scissorRect.a);

        const auto clearColor = m_Target->GetClearColor();
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader);
        glBindVertexArray(vao);

        glUniform4f(colorUniformID, m_Color.r, m_Color.g, m_Color.b, m_Color.a);

        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
}
