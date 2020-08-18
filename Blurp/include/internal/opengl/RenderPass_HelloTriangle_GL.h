#pragma once
#include "RenderPass_HelloTriangle.h"
#include <gl/glew.h>

#include "Shader.h"

namespace blurp
{
    inline const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec2 aUv;\n"
        "out vec2 uvCoord;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "   uvCoord = aUv;\n"
        "}\0";

    inline const char* fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec2 uvCoord;\n"
        "uniform sampler2D texSlot;\n"
        "uniform vec4 ucolor;\n"
        "void main()\n"
        "{\n"
        "   vec4 color = texture2D(texSlot, uvCoord);\n"
        "#ifdef RED\n"
        "   FragColor = color * vec4(1.0, 0.5, 0.5, 1.0);\n"
        "#else\n"
        "   FragColor = color * ucolor;\n"
        "#endif\n"
        "}\n\0";

    inline const char textureData[] = {
        (char)255, (char)2, (char)2, (char)255,
        (char)2, (char)255, (char)2, (char)255,
        (char)2, (char)2, (char)255, (char)255,

                (char)255, (char)2, (char)2, (char)255,
        (char)2, (char)255, (char)2, (char)255,
        (char)2, (char)2, (char)255, (char)255,

                (char)255, (char)2, (char)2, (char)255,
        (char)2, (char)255, (char)2, (char)255,
        (char)2, (char)2, (char)255, (char)255,

                (char)255, (char)2, (char)2, (char)255,
        (char)2, (char)255, (char)2, (char)255,
        (char)2, (char)2, (char)255, (char)255,

                (char)255, (char)2, (char)2, (char)255,
        (char)2, (char)255, (char)2, (char)255,
        (char)2, (char)2, (char)255, (char)255,
                (char)255, (char)2, (char)2, (char)255,
    };

    class RenderPass_HelloTriangle_GL : public RenderPass_HelloTriangle
    {
    public:
        RenderPass_HelloTriangle_GL(RenderPipeline& a_Pipeline) : RenderPass_HelloTriangle(a_Pipeline), m_Vbo(0), m_Vao(0), m_ColorUniformId(0) {}

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;
        void Execute() override;

    private:
        GLuint m_Vbo, m_Vao, m_ColorUniformId;
        std::shared_ptr<Shader> m_Shader;
        
    };
}
