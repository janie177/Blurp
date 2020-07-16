#include "MeshShaderCache.h"

#include <utility>

#include "FileReader.h"
#include "RenderResourceManager.h"

namespace blurp
{
    MeshShaderCache::MeshShaderCache() : m_Init(false), m_ResourceManager(nullptr)
    {
        
    }

    void MeshShaderCache::Init(RenderResourceManager& a_ResourceManager, const std::vector<std::string>& a_PreProcessorDefinitions,
        const MeshShaderPaths& a_ShaderPaths)
    {
        //Store the resource manager instance.
        m_ResourceManager = &a_ResourceManager;

        //Load the sources that are set.
        if (!a_ShaderPaths.vertex.empty())
        {
            FileReader reader(a_ShaderPaths.vertex);
            if (!reader.Open())
            {
                throw std::exception("Could not find shader file.");
            }

            m_VertexSource = reader.ToArray();
        }

        if (!a_ShaderPaths.fragment.empty())
        {
            FileReader reader(a_ShaderPaths.fragment);
            if (!reader.Open())
            {
                throw std::exception("Could not find shader file.");
            }

            m_FragmentSource = reader.ToArray();
        }

        if (!a_ShaderPaths.geometry.empty())
        {
            FileReader reader(a_ShaderPaths.geometry);
            if (!reader.Open())
            {
                throw std::exception("Could not find shader file.");
            }

            m_GeometrySource = reader.ToArray();
        }

        if (!a_ShaderPaths.tessControl.empty())
        {
            FileReader reader(a_ShaderPaths.tessControl);
            if (!reader.Open())
            {
                throw std::exception("Could not find shader file.");
            }

            m_TessellationControlSource = reader.ToArray();
        }

        if (!a_ShaderPaths.tessEval.empty())
        {
            FileReader reader(a_ShaderPaths.tessEval);
            if (!reader.Open())
            {
                throw std::exception("Could not find shader file.");
            }

            m_TessellationEvaluationSource = reader.ToArray();
        }

        //Store the pointers in the settings object.
        m_Settings.type = ShaderType::GRAPHICS;
        m_Settings.vertexShaderSource = m_VertexSource.get();
        m_Settings.fragmentShaderSource = m_FragmentSource.get();
        m_Settings.geometryShaderSource = m_GeometrySource.get();
        m_Settings.tessellationDomainShaderSource = m_TessellationEvaluationSource.get();
        m_Settings.tessellationHullShaderSource = m_TessellationControlSource.get();
        m_PreProcessorDefinitions = a_PreProcessorDefinitions;

        //Set the class to initialized.
        m_Init = true;
    }

    std::shared_ptr<Shader> MeshShaderCache::GetShader(VertexAttribute a_Mask)
    {
        assert(m_Init);
        const auto found = m_ShaderRegistry.find(a_Mask);
        if(found == m_ShaderRegistry.end())
        {
            return LoadShader(a_Mask);
        }
        return found->second;
    }

    void MeshShaderCache::PreLoad(std::vector<VertexAttribute> a_Masks)
    {
        assert(m_Init);
        for(auto& mask : a_Masks)
        {
            const auto found = m_ShaderRegistry.find(mask);
            if(found == m_ShaderRegistry.end())
            {
                LoadShader(mask);
            }
        }
    }

    std::shared_ptr<Shader> MeshShaderCache::LoadShader(VertexAttribute a_Mask)
    {
        assert(m_Init);
        m_Settings.preprocessorDefinitions = m_PreProcessorDefinitions;

        for(auto& attribute : VERTEX_ATTRIBUTES)
        {
            //If enabled add the preprocessor definition.
            if((a_Mask & attribute) == attribute)
            {
                const auto info = VertexSettings::GetVertexAttributeInfo(attribute);
                m_Settings.preprocessorDefinitions.emplace_back(info.defineName);
            }
        }

        //Load the shader and add to the registry.
        auto shader = m_ResourceManager->CreateShader(m_Settings);
        m_ShaderRegistry.insert({ a_Mask, shader });

        return shader;
    }
}
