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
        void FindVersionIndices(const char* a_Src, bool& a_HasVersion, const char*& a_SrcStart, const char*& a_VersionStart, std::uint16_t& a_VersionSize) const;

    private:
        GLuint m_Program;
    };
}