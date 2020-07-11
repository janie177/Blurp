#pragma once
#include "RenderPass_HelloTriangle.h"
#include <gl/glew.h>

namespace blurp
{
    inline const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";

    inline const char* fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "uniform vec4 ucolor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = ucolor;\n"
        "}\n\0";

    class RenderPass_HelloTriangle_GL : public RenderPass_HelloTriangle
    {
    public:
        RenderPass_HelloTriangle_GL(RenderPipeline& a_Pipeline) : RenderPass_HelloTriangle(a_Pipeline) {}

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;
        void Execute() override;

    private:
        GLuint vbo, vao, colorUniformID;
        GLuint shader;
        
    };
}
