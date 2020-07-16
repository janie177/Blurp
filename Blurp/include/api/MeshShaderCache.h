#pragma once
#include "BlurpEngine.h"
#include "Shader.h"

namespace blurp
{
    struct MeshShaderPaths
    {
        std::string vertex;
        std::string geometry;
        std::string fragment;
        std::string tessControl;
        std::string tessEval;
    };

    /*
     * MeshShaderCache is a utility that allows a single mesh shader to be loaded multiple times.
     * Each instance will have a different configuration of attributes.
     * 
     */
    class MeshShaderCache
    {
    public:
        MeshShaderCache();

        /*
         * Initialize this shader registry.
         */
        void Init(RenderResourceManager& a_ResourceManager, const std::vector<std::string>& a_PreProcessorDefinitions, const MeshShaderPaths& a_ShaderPaths);

        /*
         * Get the shader with the given vertex attribute mask.
         * Returns the shader found, or loads and compiles a new one if not yet existing.
         */
        std::shared_ptr<Shader> GetShader(VertexAttribute a_Mask);

        /*
         * Load the shader for each mask in the provided array.
         * If a mask was already loaded for this shader, then it will be skipped.
         */
        void PreLoad(std::vector<VertexAttribute> a_Masks);

    private:
        std::shared_ptr<Shader> LoadShader(VertexAttribute a_Mask);

    private:
        RenderResourceManager* m_ResourceManager;
        ShaderSettings m_Settings;
        std::unordered_map<VertexAttribute, std::shared_ptr<Shader>> m_ShaderRegistry;
        std::vector<std::string> m_PreProcessorDefinitions;

        std::unique_ptr<char[]> m_VertexSource;
        std::unique_ptr<char[]> m_FragmentSource;
        std::unique_ptr<char[]> m_GeometrySource;
        std::unique_ptr<char[]> m_TessellationControlSource;
        std::unique_ptr<char[]> m_TessellationEvaluationSource;

        bool m_Init;
    };
}
