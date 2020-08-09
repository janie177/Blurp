#include "opengl/RenderPass_Forward_GL.h"
#include "BlurpEngine.h"
#include "FileReader.h"
#include "opengl/GpuBuffer_GL.h"
#include "opengl/Mesh_GL.h"
#include "opengl/RenderTarget_GL.h"
#include "opengl/Shader_GL.h"
#include "opengl/Texture_GL.h"
#include "RenderResourceManager.h"

namespace blurp
{
    bool RenderPass_Forward_GL::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        auto shaderPath = a_BlurpEngine.GetEngineSettings().shadersPath + "opengl/";
        auto vertex =  "Default_Forward.vs";
        auto fragment = "Default_Forward.fs";

        FileReader vertexReader(shaderPath + vertex);
        FileReader fragmentReader(shaderPath + fragment);

        if(!vertexReader.Open() || !fragmentReader.Open())
        {
            throw std::exception("Could not find forward shaders at path specified.");
        }

        auto vertexSrc = vertexReader.ToArray();
        auto fragmentSrc = fragmentReader.ToArray();

        ShaderSettings sSettings;
        sSettings.vertexShaderSource = vertexSrc.get();
        sSettings.fragmentShaderSource = fragmentSrc.get();
        sSettings.type = ShaderType::GRAPHICS;

        //Preprocessor definitions at their respective bit indices.
        std::vector<std::string> definitions;

        //Add every mesh bitmask to it as the first set of bits.
        for(auto& attrib : VERTEX_ATTRIBUTES)
        {
            definitions.emplace_back(VertexSettings::GetVertexAttributeInfo(attrib).defineName);
        }

        //Add all the bitmask defines for the material settings.
        for (auto& attrib : MATERIAL_ATTRIBUTES)
        {
            auto found = MATERIAL_ATTRIBUTE_INFO.find(attrib);
            assert(found != MATERIAL_ATTRIBUTE_INFO.end());
            definitions.emplace_back(found->second.defineName);
        }

        //Add a define to determine whether to use a single or batch material.
        definitions.emplace_back("MAT_SINGLE_DEFINE");
        definitions.emplace_back("MAT_BATCH_DEFINE");

        m_ShaderCache.Init(a_BlurpEngine.GetResourceManager(), sSettings, definitions);

        return true;
    }

    bool RenderPass_Forward_GL::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        return true;
    }

    void RenderPass_Forward_GL::Execute()
    {
        //Clear the target buffer.
        const auto fboId = reinterpret_cast<RenderTarget_GL*>(m_Output.get())->GetFrameBufferId();

        //Set output data targets and reset the framebuffer color and depth.
        glBindFramebuffer(GL_FRAMEBUFFER, fboId);
        const auto viewPort = m_Output->GetViewPort();
        glViewport(static_cast<int>(viewPort.r), static_cast<int>(viewPort.g), static_cast<int>(viewPort.b), static_cast<int>(viewPort.a));
        const auto scissorRect = m_Output->GetScissorRect();
        glScissor(static_cast<int>(scissorRect.r), static_cast<int>(scissorRect.g), static_cast<int>(scissorRect.b), static_cast<int>(scissorRect.a));
        const auto clearColor = m_Output->GetClearColor();
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        //Shader attribute mask.
        std::uint32_t prevMask = 0;
        std::shared_ptr<Material> prevMaterial;
        std::shared_ptr<MaterialBatch> prevMaterialBatch;
        std::shared_ptr<Mesh> prevMesh;

        constexpr std::uint32_t matSingleBit = 1 << (NUM_MATERIAL_ATRRIBS + NUM_VERTEX_ATRRIBS);
        constexpr std::uint32_t matBatchBit = 1 << (NUM_MATERIAL_ATRRIBS + NUM_VERTEX_ATRRIBS + 1);

        for(auto& instanceData : m_DrawQueue)
        {
            Mesh_GL* mesh = static_cast<Mesh_GL*>(instanceData.mesh.get());

            const auto glGpuBuffer = std::reinterpret_pointer_cast<GpuBuffer_GL>(m_GpuBuffer);

            //Calculate the new mask.
            const bool useMaterial = instanceData.material != nullptr;
            const bool useMaterialBatch = instanceData.materialBatch != nullptr;

            //Has the material and materialbatch changed?
            const bool changedMaterial = prevMaterial != instanceData.material;
            const bool changedBatch = prevMaterialBatch != instanceData.materialBatch;

            //Ensure that either only one is enabled, or both are disabled. Never both enabled.
            assert((useMaterial != useMaterialBatch) || (!useMaterial && !useMaterialBatch));

            //Shader mask matching the vertex layout.
            std::uint32_t shaderMask = static_cast<std::uint32_t>(mesh->GetVertexAttributeMask());

            //If materials or batches are enabled, append those to the mask.
            if(useMaterialBatch)
            {
                //Ensure that material ID is enabled when using a batch (to fetch the material at the right index). 
                assert((instanceData.mesh->GetVertexAttributeMask() & VertexAttribute::MATERIAL_ID) == VertexAttribute::MATERIAL_ID && "To use a material batch, the provided mesh needs to have material IDs defined in its attributes!");

                shaderMask = shaderMask | matBatchBit | (static_cast<std::uint32_t>(instanceData.materialBatch->GetMask()) << NUM_VERTEX_ATRRIBS);
            }
            else if(useMaterial)
            {
                shaderMask = shaderMask | matSingleBit | (static_cast<std::uint32_t>(instanceData.material->GetSettings().GetMask()) << NUM_VERTEX_ATRRIBS);
            }

            //Has the shader changed?
            const bool changedShader = shaderMask != prevMask;

            //If the current mask is not the same as the one needed, switch shader.
            if(changedShader)
            {
                //Bind the new shader.
                prevMask = shaderMask;
                const std::shared_ptr<Shader_GL> currentShader = std::reinterpret_pointer_cast<Shader_GL>(m_ShaderCache.GetShader(shaderMask));
                glUseProgram(currentShader->GetProgramId());

                //Link the shader to the ssbo binding point.
                glShaderStorageBlockBinding(currentShader->GetProgramId(), 0, glGpuBuffer->GetBufferBaseBinding());
            }

            //If the current material is new or the shader changed, reupload the material data.
            if (useMaterial && (changedMaterial || changedShader))
            {
                auto& matSettings = instanceData.material->GetSettings();

                //DIFFUSE
                if(matSettings.IsAttributeEnabled(MaterialAttribute::DIFFUSE_TEXTURE) && matSettings.GetDiffuseTexture() != nullptr)
                {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, std::static_pointer_cast<Texture_GL>(matSettings.GetDiffuseTexture())->GetTextureId());
                }
                else if(matSettings.IsAttributeEnabled(MaterialAttribute::DIFFUSE_CONSTANT_VALUE))
                {
                    const auto diffuse = matSettings.GetDiffuseValue();
                    glUniform3f(1, diffuse.x, diffuse.y, diffuse.z);
                }

                //NORMAL
                if (matSettings.IsAttributeEnabled(MaterialAttribute::NORMAL_TEXTURE) && matSettings.GetNormalTexture() != nullptr)
                {
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, std::static_pointer_cast<Texture_GL>(matSettings.GetNormalTexture())->GetTextureId());
                }

                //EMISSIVE
                if (matSettings.IsAttributeEnabled(MaterialAttribute::EMISSIVE_TEXTURE) && matSettings.GetEmissiveTexture() != nullptr)
                {
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, std::static_pointer_cast<Texture_GL>(matSettings.GetEmissiveTexture())->GetTextureId());
                }
                else if (matSettings.IsAttributeEnabled(MaterialAttribute::EMISSIVE_CONSTANT_VALUE))
                {
                    const auto diffuse = matSettings.GetEmissiveValue();
                    glUniform3f(2, diffuse.x, diffuse.y, diffuse.z);
                }

                //METAL/ROUGHNESS/ALPHA
                if ((matSettings.IsAttributeEnabled(MaterialAttribute::METALLIC_TEXTURE) || matSettings.IsAttributeEnabled(MaterialAttribute::ROUGHNESS_TEXTURE) || matSettings.IsAttributeEnabled(MaterialAttribute::ALPHA_TEXTURE)) && matSettings.GetMRATexture() != nullptr)
                {
                    glActiveTexture(GL_TEXTURE3);
                    glBindTexture(GL_TEXTURE_2D, std::static_pointer_cast<Texture_GL>(matSettings.GetMRATexture())->GetTextureId());
                }
                if (matSettings.IsAttributeEnabled(MaterialAttribute::METALLIC_CONSTANT_VALUE))
                {
                    glUniform1f(3, matSettings.GetAlphaValue());
                }
                if (matSettings.IsAttributeEnabled(MaterialAttribute::ROUGHNESS_CONSTANT_VALUE))
                {
                    glUniform1f(4, matSettings.GetAlphaValue());
                }
                if (matSettings.IsAttributeEnabled(MaterialAttribute::ALPHA_CONSTANT_VALUE))
                {
                    glUniform1f(5, matSettings.GetAlphaValue());
                }

                //OCCLUSION/HEIGHT
                if ((matSettings.IsAttributeEnabled(MaterialAttribute::OCCLUSION_TEXTURE) || matSettings.IsAttributeEnabled(MaterialAttribute::HEIGHT_TEXTURE)) && matSettings.GetOHTexture() != nullptr)
                {
                    glActiveTexture(GL_TEXTURE4);
                    glBindTexture(GL_TEXTURE_2D, std::static_pointer_cast<Texture_GL>(matSettings.GetOHTexture())->GetTextureId());
                }
            }

            //If the material batch data needs to be bound, bind it.
            if (useMaterialBatch && (changedBatch || changedShader))
            {
                //TODO bind material batch.
                //TODO this includes array texture and the UBO.
            }

            //Set the binding point that the shader interface block reads from to contain a specific range from the GPU buffer.
            glBindBufferRange(GL_SHADER_STORAGE_BUFFER, glGpuBuffer->GetBufferBaseBinding(), glGpuBuffer->GetBufferId(), static_cast<GLintptr>(instanceData.dataRange.start), instanceData.dataRange.size);

            //Set the number of instances from the mesh itself in the uniform.
            glUniform1i(0, mesh->GetInstanceCount());

            //If the geometry changed, bind the new geometry.
            if(prevMesh != instanceData.mesh)
            {
                //Bind the VAO of the mesh.
                glBindVertexArray(mesh->GetVaoId());
            }

            //Finally draw instanced.
            glDrawElementsInstanced(GL_TRIANGLES, mesh->GetNumIndices(), mesh->GetIndexDataType(), nullptr, instanceData.count * mesh->GetInstanceCount());
        }

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        glBindVertexArray(0);
    }
}
