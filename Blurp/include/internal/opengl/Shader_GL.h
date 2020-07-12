#pragma once
#include <GL/glew.h>

#include "Shader.h"

namespace blurp
{
    class Shader_GL : public Shader
    {
    public:
        Shader_GL(const ShaderSettings& a_Settings) : Shader(a_Settings), m_Program(0) {}

        /*
         * Get the shader program ID for this shader.
         */
        GLuint GetProgramId() const;

    protected:
        bool OnLoad(BlurpEngine& a_BlurpEngine) override;
        bool OnDestroy(BlurpEngine& a_BlurpEngine) override;

    private:
        bool CompileShader(GLuint a_ShaderId);

    private:
        GLuint m_Program;
    };
}