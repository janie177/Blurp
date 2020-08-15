#include "opengl/Shader_GL.h"

#include <iostream>
#include <sstream>

namespace blurp
{
    GLuint Shader_GL::GetProgramId() const
    {
        return m_Program;
    }

    bool Shader_GL::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        //Append the #define tag to each preprocessor definition.
        std::vector<std::string> defines;
        for(auto& s : m_Settings.preprocessorDefinitions)
        {
            defines.emplace_back("#define " + s + "\n");
        }

        //Convert strings to const char* array.
        //The size of this array is two larger that the amount of defines:
        //The first slot is reserved for the shader version.
        //The last slot is the rest of the shader source.
        std::vector<const char*> cStrings;
        cStrings.resize(defines.size() + 2, nullptr);

        for(size_t i = 0; i < defines.size(); ++i)
        {
            cStrings[i + 1L] = defines[i].c_str();
        }

        //Graphics shader.
        if (m_Settings.type == ShaderType::GRAPHICS)
        {
            GLuint vertex = 0, fragment = 0, tess_hull = 0, tess_domain = 0, geometry = 0;

            bool hasVertex = m_Settings.vertexShaderSource != nullptr;
            bool hasFragment = m_Settings.fragmentShaderSource != nullptr;
            bool hasTessHull = m_Settings.tessellationHullShaderSource != nullptr;
            bool hasTessDomain = m_Settings.tessellationDomainShaderSource != nullptr;
            bool hasGeometry = m_Settings.geometryShaderSource != nullptr;


            //Vertex shader is always required.
            if (hasVertex)
            {
                //Create a shader, set the correct source and then compile.
                vertex = glCreateShader(GL_VERTEX_SHADER);

                //Find the version tag in the shader source, then correctly set the indices for each.
                bool hasVersion;
                const char* srcStart;
                const char* versionStart;
                std::uint16_t versionSize;
                FindVersionIndices(m_Settings.vertexShaderSource, hasVersion, srcStart, versionStart, versionSize);
                assert(hasVersion && "GLSL shaders must specify their shader version!");

                //Append the version info to the front. All defines are now between the source and version info.
                const std::string versionInfo = std::string(versionStart, versionSize);
                cStrings[0] = versionInfo.c_str();

                //Start the source from AFTER the version info.
                cStrings[cStrings.size() - 1] = srcStart;

                glShaderSource(vertex, static_cast<GLsizei>(cStrings.size()), &cStrings[0], NULL);
                bool success = CompileShader(vertex, &cStrings[0], cStrings.size());
                assert(success);
            }
            else
            {
                throw std::exception("Vertex shader is always required!");
            }

            //Fragment shader
            if (hasFragment)
            {
                //Create a shader, set the correct source and then compile.
                fragment = glCreateShader(GL_FRAGMENT_SHADER);

                //Find the version tag in the shader source, then correctly set the indices for each.
                bool hasVersion;
                const char* srcStart;
                const char* versionStart;
                std::uint16_t versionSize;
                FindVersionIndices(m_Settings.fragmentShaderSource, hasVersion, srcStart, versionStart, versionSize);
                assert(hasVersion && "GLSL shaders must specify their shader version!");

                //Append the version info to the front. All defines are now between the source and version info.
                const std::string versionInfo = std::string(versionStart, versionSize);
                cStrings[0] = versionInfo.c_str();

                //Start the source from AFTER the version info.
                cStrings[cStrings.size() - 1] = srcStart;

                glShaderSource(fragment, static_cast<GLsizei>(cStrings.size()), &cStrings[0], NULL);
                const bool success = CompileShader(fragment, &cStrings[0], cStrings.size());
                assert(success);
            }

            //Tessellation hull/control shader
            if (hasTessHull)
            {
                //Create a shader, set the correct source and then compile.
                tess_hull = glCreateShader(GL_TESS_CONTROL_SHADER);

                //Find the version tag in the shader source, then correctly set the indices for each.
                bool hasVersion;
                const char* srcStart;
                const char* versionStart;
                std::uint16_t versionSize;
                FindVersionIndices(m_Settings.tessellationHullShaderSource, hasVersion, srcStart, versionStart, versionSize);
                assert(hasVersion && "GLSL shaders must specify their shader version!");

                //Append the version info to the front. All defines are now between the source and version info.
                const std::string versionInfo = std::string(versionStart, versionSize);
                cStrings[0] = versionInfo.c_str();

                //Start the source from AFTER the version info.
                cStrings[cStrings.size() - 1] = srcStart;

                glShaderSource(tess_hull, static_cast<GLsizei>(cStrings.size()), &cStrings[0], NULL);
                const bool success = CompileShader(tess_hull, &cStrings[0], cStrings.size());
                assert(success);
            }

            //Tessellation domain/evaluation shader
            if (hasTessDomain)
            {
                //Create a shader, set the correct source and then compile.
                tess_domain = glCreateShader(GL_TESS_EVALUATION_SHADER);

                //Find the version tag in the shader source, then correctly set the indices for each.
                bool hasVersion;
                const char* srcStart;
                const char* versionStart;
                std::uint16_t versionSize;
                FindVersionIndices(m_Settings.tessellationDomainShaderSource, hasVersion, srcStart, versionStart, versionSize);
                assert(hasVersion && "GLSL shaders must specify their shader version!");

                //Append the version info to the front. All defines are now between the source and version info.
                const std::string versionInfo = std::string(versionStart, versionSize);
                cStrings[0] = versionInfo.c_str();

                //Start the source from AFTER the version info.
                cStrings[cStrings.size() - 1] = srcStart;

                glShaderSource(tess_domain, static_cast<GLsizei>(cStrings.size()), &cStrings[0], NULL);
                const bool success = CompileShader(tess_domain, &cStrings[0], cStrings.size());
                assert(success);
            }

            //Geometry shader
            if (hasGeometry)
            {
                //Create a shader, set the correct source and then compile.
                geometry = glCreateShader(GL_GEOMETRY_SHADER);

                //Find the version tag in the shader source, then correctly set the indices for each.
                bool hasVersion;
                const char* srcStart;
                const char* versionStart;
                std::uint16_t versionSize;
                FindVersionIndices(m_Settings.geometryShaderSource, hasVersion, srcStart, versionStart, versionSize);
                assert(hasVersion && "GLSL shaders must specify their shader version!");

                //Append the version info to the front. All defines are now between the source and version info.
                const std::string versionInfo = std::string(versionStart, versionSize);
                cStrings[0] = versionInfo.c_str();

                //Start the source from AFTER the version info.
                cStrings[cStrings.size() - 1] = srcStart;

                glShaderSource(geometry, static_cast<GLsizei>(cStrings.size()), &cStrings[0], NULL);
                const bool success = CompileShader(geometry, &cStrings[0], cStrings.size());
                assert(success);
            }


            // link shaders
            m_Program = glCreateProgram();

            if(hasVertex) glAttachShader(m_Program, vertex);
            if (hasFragment) glAttachShader(m_Program, fragment);
            if (hasTessHull) glAttachShader(m_Program, tess_hull);
            if (hasTessDomain) glAttachShader(m_Program, tess_domain);
            if (hasGeometry) glAttachShader(m_Program, geometry);

            glLinkProgram(m_Program);

            // check for linking errors
            int success;
            char infoLog[512];
            glGetProgramiv(m_Program, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(m_Program, 512, NULL, infoLog);
                std::cout << "Error could not link shaders.\n" << infoLog << std::endl;
            }

            //Clean up individual sources.
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            glDeleteShader(geometry);
            glDeleteShader(tess_domain);
            glDeleteShader(tess_hull);
        }

        //Compute pipeline
        else
        {
            if(m_Settings.computeShaderSource == nullptr)
            {
                throw std::exception("Error: trying to make compute shader with null source.");
                return false;
            }

            //Create a shader, set the correct source and then compile.
            GLuint compute = glCreateShader(GL_COMPUTE_SHADER);

            //Find the version tag in the shader source, then correctly set the indices for each.
            bool hasVersion;
            const char* srcStart;
            const char* versionStart;
            std::uint16_t versionSize;
            FindVersionIndices(m_Settings.computeShaderSource, hasVersion, srcStart, versionStart, versionSize);
            assert(hasVersion && "GLSL shaders must specify their shader version!");

            //Append the version info to the front. All defines are now between the source and version info.
            const std::string versionInfo = std::string(versionStart, versionSize);
            cStrings[0] = versionInfo.c_str();

            //Start the source from AFTER the version info.
            cStrings[cStrings.size() - 1] = srcStart;

            glShaderSource(compute, static_cast<GLsizei>(cStrings.size()), &cStrings[0], NULL);
            const bool compiled = CompileShader(compute, &cStrings[0], cStrings.size());
            assert(compiled);

            m_Program = glCreateProgram();
            glAttachShader(m_Program, compute);
            glLinkProgram(m_Program);

            // check for linking errors
            int success;
            char infoLog[512];
            glGetProgramiv(m_Program, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(m_Program, 512, NULL, infoLog);
                std::cout << "Error could not link shaders.\n" << infoLog << std::endl;
                return false;
            }

            //Clean up individual sources.
            glDeleteShader(compute);
        }

        return true;
    }

    bool Shader_GL::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        glDeleteProgram(m_Program);
        return true;
    }

    bool Shader_GL::CompileShader(GLuint a_ShaderId, const char** a_Src, std::size_t a_Size)
    {
        glCompileShader(a_ShaderId);
        int success;
        char infoLog[512];
        glGetShaderiv(a_ShaderId, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(a_ShaderId, 512, NULL, infoLog);
            std::cout << "Error Could not compile shader: \n" << infoLog << std::endl;
            std::cout << "Source code: \n";

            int line = 1;

            if(a_Src != nullptr)
            {
                for(std::size_t i = 0; i < a_Size; ++i)
                {
                    std::stringstream ss(a_Src[i]);
                    std::string to;

                    if (a_Src[i] != NULL)
                    {
                        while (std::getline(ss, to, '\n')) {
                            std::cout << line << ".     " << to << std::endl;
                            ++line;
                        }
                    }
                }
            }
            
            return false;
        }

        return true;
    }

    void Shader_GL::FindVersionIndices(const char* a_Src, bool& a_HasVersion, const char*& a_SrcStart,
        const char*& a_VersionStart, std::uint16_t& a_VersionSize) const
    {
        //Find the version line. If not specified, return.
        const auto versionStart = strstr(a_Src, "#version ");
        const auto versionEnd = strstr(a_Src, "\n");
        if(versionStart == nullptr || versionEnd == nullptr)
        {
            a_HasVersion = false;
            a_SrcStart = a_Src;
            a_VersionStart = nullptr;
            a_VersionSize = 0;
            return;
        }

        //Version is found. Return the positions.
        a_HasVersion = true;
        a_SrcStart = versionEnd + 1;
        a_VersionStart = versionStart;
        a_VersionSize = static_cast<std::uint16_t>(versionEnd - versionStart + 1L);
    }
}
