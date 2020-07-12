#include "opengl/Shader_GL.h"

#include <iostream>

namespace blurp
{
    GLuint Shader_GL::GetProgramId() const
    {
        return m_Program;
    }

    bool Shader_GL::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        //TODO have a separate compute and graphics shader type. Below is graphics.

        //Append the #define tag to each preprocessor definition.
        std::vector<std::string> defines;
        for(auto& s : m_Settings.preprocessorDefinitions)
        {
            defines.emplace_back("#define " + s + "\n");
        }

        //Convert strings to const char* array.
        std::vector<const char*> cStrings;
        cStrings.resize(defines.size() + 1, nullptr);

        for(int i = 0; i < static_cast<int>(defines.size()); ++i)
        {
            cStrings[i] = defines[i].c_str();
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
                cStrings[cStrings.size() - 1] = m_Settings.vertexShaderSource;
                glShaderSource(vertex, cStrings.size(), &cStrings[0], NULL);
                bool success = CompileShader(vertex);
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
                cStrings[cStrings.size() - 1] = m_Settings.fragmentShaderSource;
                glShaderSource(fragment, cStrings.size(), &cStrings[0], NULL);
                const bool success = CompileShader(fragment);
                assert(success);
            }

            //Tessellation hull/control shader
            if (hasTessHull)
            {
                //Create a shader, set the correct source and then compile.
                tess_hull = glCreateShader(GL_TESS_CONTROL_SHADER);
                cStrings[cStrings.size() - 1] = m_Settings.tessellationHullShaderSource;
                glShaderSource(tess_hull, cStrings.size(), &cStrings[0], NULL);
                const bool success = CompileShader(tess_hull);
                assert(success);
            }

            //Tessellation domain/evaluation shader
            if (hasTessDomain)
            {
                //Create a shader, set the correct source and then compile.
                tess_domain = glCreateShader(GL_TESS_EVALUATION_SHADER);
                cStrings[cStrings.size() - 1] = m_Settings.tessellationDomainShaderSource;
                glShaderSource(tess_domain, cStrings.size(), &cStrings[0], NULL);
                const bool success = CompileShader(tess_domain);
                assert(success);
            }

            //Geometry shader
            if (hasGeometry)
            {
                //Create a shader, set the correct source and then compile.
                geometry = glCreateShader(GL_GEOMETRY_SHADER);
                cStrings[cStrings.size() - 1] = m_Settings.geometryShaderSource;
                glShaderSource(geometry, cStrings.size(), &cStrings[0], NULL);
                const bool success = CompileShader(geometry);
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
            cStrings[cStrings.size() - 1] = m_Settings.computeShaderSource;
            glShaderSource(compute, cStrings.size(), &cStrings[0], NULL);
            const bool compiled = CompileShader(compute);
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

    bool Shader_GL::CompileShader(GLuint a_ShaderId)
    {
        glCompileShader(a_ShaderId);
        int success;
        char infoLog[512];
        glGetShaderiv(a_ShaderId, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(a_ShaderId, 512, NULL, infoLog);
            std::cout << "Error Could not compile shader: \n" << infoLog << std::endl;
            return false;
        }

        return true;
    }
}
