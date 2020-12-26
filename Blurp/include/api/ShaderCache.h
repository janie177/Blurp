#pragma once
#include "BlurpEngine.h"
#include "Shader.h"
#include "FileReader.h"
#include "RenderResourceManager.h"
#include <iostream>

namespace blurp
{
    /*
     * ShaderCache is a utility that allows a single shader to be loaded multiple times.
     * A bitmask can be provided that corresponds to preprocessor definitions.
     * This allows a single shader file to have a lot of different behaviors, inputs and outputs.
     *
     * T is the mask type. This has to be an integer or enumeration type.
     * INTERNAL_FORMAT is what the mask will be stored as. This 
     */
    template<typename T, typename INTERNAL_FORMAT>
    class ShaderCache
    {
        //Ensure that an enumeration and internal format are provided.
        //TODO use conditional to only enable underlying type when is_enum is true. Disabled for now because dark magic is at hand with these templates.
        ///* && std::is_convertible_v<std::underlying_type_t<T>, INTERNAL_FORMAT>) */
        static_assert((std::is_same_v<T, INTERNAL_FORMAT> || std::is_convertible_v<T, INTERNAL_FORMAT> || std::is_enum_v<T>) && std::is_integral_v<INTERNAL_FORMAT>, "T has to be convertible to the internal format provided which has to be interpretable as an integer.");

    public:
        ShaderCache();

        /*
         * Initialize this shader registry.
         * This stores a reference to the resource manager that will be used to construct the shaders.
         * The ShaderSettings object contains all settings that the shaders loaded in this cache will adhere to.
         * Definitions is an array of strings that correspond to preprocessor definitions in the shader.
         * The slot in which each string is stored corresponds to the bit in the bitmask that will enable that definition.
         *
         * Note: The shader settings object provided should contain the raw source of the shader.
         */
        void Init(RenderResourceManager& a_ResourceManager,const ShaderSettings& a_Settings, const std::vector<std::string>& a_Definitions);

        /* 
         * Get the shader with the given bit mask.
         * Returns the shader found, or loads and compiles a new one if not yet existing.
         */
        std::shared_ptr<Shader> GetOrLoad(T a_Mask);

        /*
         * Get the shader with the given mask.
         * If no shader could be found, returns an empty shared pointer.
         */
        std::shared_ptr<Shader> GetOrNull(T a_Mask) const;

        /*
         * Load the shader for each mask in the provided array.
         * If a mask was already loaded for this shader, then it will be skipped.
         */
        void PreLoad(const std::vector<T>& a_Masks);

        /*
         * Load the shader with the given mask and the additional provided defines.
         * This will overwrite any shader that is already present.
         */
        std::shared_ptr<Shader> LoadShader(T a_Mask, const std::vector<std::string>& a_AdditionalDefines = std::vector<std::string>());

    private:
        RenderResourceManager* m_ResourceManager;
        ShaderSettings m_Settings;
        std::unordered_map<INTERNAL_FORMAT, std::shared_ptr<Shader>> m_ShaderRegistry;

        //Preprocessor definition matching with each bit at the same index.
        std::vector<std::string> m_PreProcessorDefinitions;
        std::uint32_t m_BasePreprocessorCount;

        //Raw shader source before compilation.
        std::string m_VertexSource;
        std::string m_FragmentSource;
        std::string m_GeometrySource;
        std::string m_TessellationControlSource;
        std::string m_TessellationEvaluationSource;

        bool m_Init;
    };

    template <typename T, typename INTERNAL_FORMAT>
    ShaderCache<T, INTERNAL_FORMAT>::ShaderCache() : m_ResourceManager(nullptr), m_BasePreprocessorCount(0), m_Init(false)
    { 
    }

    template <typename T, typename INTERNAL_FORMAT>
    void ShaderCache<T, INTERNAL_FORMAT>::Init(RenderResourceManager& a_ResourceManager,
        const ShaderSettings& a_Settings, const std::vector<std::string>& a_Definitions)
    {
        //Ensure that the amount of definitions can fit in the specified bit mask size.
        std::cout << sizeof(INTERNAL_FORMAT) << std::endl;
        assert(a_Definitions.size() <= (sizeof(INTERNAL_FORMAT) * 8) && "Bitmask data type does not have enough bits to hold the specified amount of preprocessor definitions!");

        //Store the resource manager instance.
        m_ResourceManager = &a_ResourceManager;

        //Store the shader settings in the local settings object.
        m_Settings = a_Settings;

        //Copy each shader source locally and then set a pointer to it in the settings object.
        //Local copy is required because the original pointer may be invalidated later on, while this cache has to keep compiling from it.
        if (a_Settings.vertexShaderSource != nullptr)
        {
            m_VertexSource = std::string(a_Settings.vertexShaderSource);
            m_Settings.vertexShaderSource = m_VertexSource.c_str();
        }

        if (a_Settings.fragmentShaderSource != nullptr)
        {
            m_FragmentSource = std::string(a_Settings.fragmentShaderSource);
            m_Settings.fragmentShaderSource = m_FragmentSource.c_str();
        }

        if (a_Settings.geometryShaderSource != nullptr)
        {
            m_GeometrySource = std::string(a_Settings.geometryShaderSource);
            m_Settings.geometryShaderSource = m_GeometrySource.c_str();
        }

        if (a_Settings.tessellationHullShaderSource != nullptr)
        {
            m_TessellationControlSource = std::string(a_Settings.tessellationHullShaderSource);
            m_Settings.tessellationHullShaderSource = m_TessellationControlSource.c_str();
        }

        if (a_Settings.tessellationDomainShaderSource != nullptr)
        {
            m_TessellationEvaluationSource = std::string(a_Settings.tessellationDomainShaderSource);
            m_Settings.tessellationDomainShaderSource = m_TessellationEvaluationSource.c_str();
        }

        //Store the preprocessor definitions per bit.
        m_PreProcessorDefinitions = a_Definitions;

        //How many preprocessor definitions are present by default.
        m_BasePreprocessorCount = static_cast<std::uint32_t>(a_Settings.preprocessorDefinitions.size());

        //Set the class to initialized.
        m_Init = true;
    }

    template <typename T, typename INTERNAL_FORMAT>
    std::shared_ptr<Shader> ShaderCache<T, INTERNAL_FORMAT>::GetOrLoad(T a_Mask)
    {
        assert(m_Init);
        INTERNAL_FORMAT asInternalFormat = static_cast<INTERNAL_FORMAT>(a_Mask);
        const auto found = m_ShaderRegistry.find(asInternalFormat);
        if (found == m_ShaderRegistry.end())
        {
            return LoadShader(a_Mask);
        }
        return found->second;
    }

    template <typename T, typename INTERNAL_FORMAT>
    std::shared_ptr<Shader> ShaderCache<T, INTERNAL_FORMAT>::GetOrNull(T a_Mask) const
    {
        assert(m_Init);
        INTERNAL_FORMAT asInternalFormat = static_cast<INTERNAL_FORMAT>(a_Mask);
        const auto found = m_ShaderRegistry.find(asInternalFormat);
        if (found == m_ShaderRegistry.end())
        {
            return nullptr;
        }
        return found->second;
    }

    template <typename T, typename INTERNAL_FORMAT>
    void ShaderCache<T, INTERNAL_FORMAT>::PreLoad(const std::vector<T>& a_Masks)
    {
        assert(m_Init);
        for (auto& mask : a_Masks)
        {
            INTERNAL_FORMAT asInternalFormat = static_cast<INTERNAL_FORMAT>(mask);
            const auto found = m_ShaderRegistry.find(asInternalFormat);
            if (found == m_ShaderRegistry.end())
            {
                LoadShader(mask);
            }
        }
    }

    template <typename T, typename INTERNAL_FORMAT>
    std::shared_ptr<Shader> ShaderCache<T, INTERNAL_FORMAT>::LoadShader(T a_Mask,
        const std::vector<std::string>& a_AdditionalDefines)
    {
        assert(m_Init);
        auto internalFormatMask = static_cast<INTERNAL_FORMAT>(a_Mask);

        //Only keep the basic preprocessor definitions that are always present.
        if (m_Settings.preprocessorDefinitions.size() != m_BasePreprocessorCount)
        {
            m_Settings.preprocessorDefinitions.resize(m_BasePreprocessorCount);
        }

        for (int i = 0; i < static_cast<int>(m_PreProcessorDefinitions.size()); ++i)
        {
            //If the mask has the bit at index i set, enable that preprocessor definition in the shader.
            if (internalFormatMask & (static_cast<INTERNAL_FORMAT>(1) << i))
            {
                m_Settings.preprocessorDefinitions.emplace_back(m_PreProcessorDefinitions[i]);
            }
        }

        //Add the additional defines.
        if(!a_AdditionalDefines.empty())
        {
            m_Settings.preprocessorDefinitions.insert(m_Settings.preprocessorDefinitions.end(), a_AdditionalDefines.begin(), a_AdditionalDefines.end());
        }

        //Load the shader and add to the registry.
        auto shader = m_ResourceManager->CreateShader(m_Settings);
        m_ShaderRegistry.insert({ a_Mask, shader });

        return shader;
    }
}
