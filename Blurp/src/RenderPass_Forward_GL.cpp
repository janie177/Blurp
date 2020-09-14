#include "opengl/RenderPass_Forward_GL.h"

#include <glm/gtc/type_ptr.hpp>

#include "BlurpEngine.h"
#include "FileReader.h"
#include "opengl/GpuBuffer_GL.h"
#include "opengl/MaterialBatch_GL.h"
#include "opengl/Mesh_GL.h"
#include "opengl/RenderTarget_GL.h"
#include "opengl/Shader_GL.h"
#include "opengl/Texture_GL.h"
#include "Material.h"
#include "GpuBuffer.h"
#include "MaterialBatch.h"


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

        //Add all the draw attribute defines  to the shader cache.
        for (auto& attrib : DRAW_ATTRIBUTES)
        {
            auto found = DRAW_ATTRIBUTE_INFO.find(attrib);
            assert(found != DRAW_ATTRIBUTE_INFO.end());
            definitions.emplace_back(found->second.defineName);
        }

        //Add all the bitmask defines for the material settings.
        for (auto& attrib : MATERIAL_ATTRIBUTES)
        {
            auto found = MATERIAL_ATTRIBUTE_INFO.find(attrib);
            assert(found != MATERIAL_ATTRIBUTE_INFO.end());
            definitions.emplace_back(found->second.defineName);
        }

        //Add a define for shadows enabled or not.
        definitions.emplace_back("USE_POS_SHADOWS_DEFINE");
        definitions.emplace_back("USE_DIR_SHADOWS_DEFINE");

        m_ShaderCache.Init(a_BlurpEngine.GetResourceManager(), sSettings, definitions);

        //Create the static data buffer. Also bind the buffer to slot 1. The shader is hard coded to read camera data from slot 1.
        glGenBuffers(1, &m_StaticDataUbo);
        glBindBuffer(GL_UNIFORM_BUFFER, m_StaticDataUbo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(StaticData), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_StaticDataUbo);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        //Create the UBO used to upload light data to the GPU. Hard coded to slot 4.
        glGenBuffers(1, &m_LightUbo);
        glBindBuffer(GL_UNIFORM_BUFFER, m_LightUbo);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(LightDataPacked) * MAX_LIGHTS, nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 4, m_LightUbo);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        //Create samplers to sample shadows maps.
        glGenSamplers(1, &m_ShadowSampler);
        glSamplerParameteri(m_ShadowSampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glSamplerParameteri(m_ShadowSampler, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
        glSamplerParameteri(m_ShadowSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glSamplerParameteri(m_ShadowSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(m_ShadowSampler, GL_TEXTURE_CUBE_MAP_SEAMLESS, GL_TRUE);
        glSamplerParameteri(m_ShadowSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glSamplerParameteri(m_ShadowSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        float border[]{1.f, 1.f, 1.f, 1.f};
        glSamplerParameterfv(m_ShadowSampler, GL_TEXTURE_BORDER_COLOR, border);

        return true;
    }

    bool RenderPass_Forward_GL::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        glDeleteBuffers(1, &m_StaticDataUbo);
        glDeleteSamplers(1, &m_ShadowSampler);
        return true;
    }

    void RenderPass_Forward_GL::Execute()
    {
        /*
         * Render state and target preparing.
         */

        //Clear the target buffer.
        const auto rtGL = reinterpret_cast<RenderTarget_GL*>(m_Output.get());

        //Bind the render target.
        rtGL->Bind();

        //TODO move this into a RenderState object or something.
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);


        /*
         * Global data setup that is used for all draw calls.
         */

        //Cache last used bound data to prevent binding it again unless it changes.
        std::uint64_t prevMask = 0;
        std::shared_ptr<Material> prevMaterial;
        std::shared_ptr<MaterialBatch> prevMaterialBatch;
        std::shared_ptr<Mesh> prevMesh;

        //To set uniforms.
        GLuint currentProgramId = 0;

        //Bits used for materials and uploaded data.
        constexpr std::uint64_t usePosShadowsBit = static_cast<std::uint64_t>(1) << (NUM_MATERIAL_ATRRIBS + NUM_VERTEX_ATRRIBS + NUM_DRAW_ATTRIBS);
        constexpr std::uint64_t useDirShadowsBit = usePosShadowsBit << 1;

        //Bind lights

        if(m_LightData.pointLights.count > 0 || m_LightData.pointLights.shadowCount > 0)
        {
            constexpr GLuint slot = 5;
            auto bufferId = static_cast<GpuBuffer_GL*>(m_LightData.pointLights.dataBuffer.get())->GetBufferId();
            auto start = m_LightData.pointLights.dataRange.start;
            auto size = m_LightData.pointLights.dataRange.totalSize;
            glBindBufferRange(GL_SHADER_STORAGE_BUFFER, slot, bufferId, static_cast<GLintptr>(start), size);
        }

        if (m_LightData.spotLights.count > 0 || m_LightData.spotLights.shadowCount > 0)
        {
            constexpr GLuint slot = 6;
            auto bufferId = static_cast<GpuBuffer_GL*>(m_LightData.spotLights.dataBuffer.get())->GetBufferId();
            auto start = m_LightData.spotLights.dataRange.start;
            auto size = m_LightData.spotLights.dataRange.totalSize;
            glBindBufferRange(GL_SHADER_STORAGE_BUFFER, slot, bufferId, static_cast<GLintptr>(start), size);
        }

        if (m_LightData.directionalLights.count > 0 || m_LightData.directionalLights.shadowCount > 0)
        {
            constexpr GLuint slot = 7;
            auto bufferId = static_cast<GpuBuffer_GL*>(m_LightData.directionalLights.dataBuffer.get())->GetBufferId();
            auto start = m_LightData.directionalLights.dataRange.start;
            auto size = m_LightData.directionalLights.dataRange.totalSize;
            glBindBufferRange(GL_SHADER_STORAGE_BUFFER, slot, bufferId, static_cast<GLintptr>(start), size);
        }

        auto numPosShadows = m_LightData.pointLights.shadowCount + m_LightData.spotLights.shadowCount;
        auto numDirShadows = m_LightData.directionalLights.shadowCount;

        //Bind the shadow samplers if they are specified and there is lights that use shadows.
        if (m_ShadowData.directional.shadowMaps != nullptr && numDirShadows > 0 && m_ShadowData.directional.dataBuffer != nullptr)
        {
            //Sampler and shadowmaps
            glActiveTexture(GL_TEXTURE0 + 6);
            glBindTexture(GL_TEXTURE_2D_ARRAY, reinterpret_cast<Texture_GL*>(m_ShadowData.directional.shadowMaps.get())->GetTextureId());
            glBindSampler(6, m_ShadowSampler);

            //Bind the buffer containing light space transformations.
            auto glBuffer = static_cast<GpuBuffer_GL*>(m_ShadowData.directional.dataBuffer.get());
            glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 4, glBuffer->GetBufferId(), static_cast<GLintptr>(m_ShadowData.directional.dataRange->start), m_ShadowData.directional.dataRange->totalSize);
        }

        if (m_ShadowData.positional.shadowMaps != nullptr && (numPosShadows > 0))
        {
            glActiveTexture(GL_TEXTURE0 + 7);
            glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, reinterpret_cast<Texture_GL*>(m_ShadowData.positional.shadowMaps.get())->GetTextureId());
            glBindSampler(7, m_ShadowSampler);
        }
        

        //Upload the static data to the GPU such as the camera and light counts.
        StaticData staticData;
        staticData.pv = m_Camera->GetProjectionMatrix() * m_Camera->GetViewMatrix();
        staticData.camPosFarPlane = glm::vec4(m_Camera->GetTransform().GetTranslation(), m_Camera->GetSettings().farPlane);
        staticData.numLightsNumCascades = glm::vec4(m_LightData.pointLights.count, m_LightData.spotLights.count, m_LightData.directionalLights.count, m_ShadowData.directional.numCascades);
        staticData.numShadows = glm::vec4(m_LightData.pointLights.shadowCount, m_LightData.spotLights.shadowCount, m_LightData.directionalLights.shadowCount, 0.f);
        staticData.ambientLight = glm::vec4(m_LightData.ambient, 0.f);

        glBindBuffer(GL_UNIFORM_BUFFER, m_StaticDataUbo);
        glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_StaticDataUbo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(staticData), static_cast<void*>(&staticData));


        /*
         * Actual drawing.
         */


        for(auto i = 0u; i < m_DrawDataSet.drawDataCount; ++i)
        {
            auto& instanceData = m_DrawDataSet.drawDataPtr[i];

            assert(instanceData.mesh != nullptr && "Mesh cannot be nullptr!");
            assert(instanceData.instanceCount > 0 && "Cannot draw 0 instances of mesh!");

            //TODO lock using ResourceLock for all currently used resources.

            Mesh_GL* mesh = static_cast<Mesh_GL*>(instanceData.mesh.get());

            //Shadows active?
            const bool usePosShadows = numPosShadows != 0;
            const bool useDirShadows = numDirShadows != 0;

            //Has the material and materialbatch changed?
            const bool changedMaterial = prevMaterial != instanceData.materialData.material;
            const bool changedBatch = prevMaterialBatch != instanceData.materialData.materialBatch;

            //Cache last used batch/material
            prevMaterial = instanceData.materialData.material;
            prevMaterialBatch = instanceData.materialData.materialBatch;

            //See if materials or batches are enabled.
            const bool material = instanceData.attributes.IsAttributeEnabled(DrawAttribute::MATERIAL_SINGLE);
            const bool materialBatch = instanceData.attributes.IsAttributeEnabled(DrawAttribute::MATERIAL_BATCH);

            //Ensure that either only one is enabled, or both are disabled. Never both enabled.
            assert((material != materialBatch) || (!material && !materialBatch));

            //Shader mask matching the vertex layout.
            std::uint64_t shaderMask = static_cast<std::uint64_t>(mesh->GetVertexAttributeMask()) | (instanceData.attributes.GetMask() << NUM_VERTEX_ATRRIBS);

            if(material && instanceData.materialData.material != nullptr)
            {
                shaderMask = shaderMask | (static_cast<std::uint64_t>(instanceData.materialData.material->GetSettings().GetMask()) << (NUM_VERTEX_ATRRIBS + NUM_DRAW_ATTRIBS));
            }
            else if(materialBatch && instanceData.materialData.materialBatch != nullptr)
            {
                shaderMask = shaderMask | (static_cast<std::uint64_t>(instanceData.materialData.materialBatch->GetMask()) << (NUM_VERTEX_ATRRIBS + NUM_DRAW_ATTRIBS));
            }

            //Mask for shadow usage.
            if(usePosShadows)
            {
                shaderMask |= usePosShadowsBit;
            }
            if(useDirShadows)
            {
                shaderMask |= useDirShadowsBit;
            }

            //Has the shader changed?
            const bool changedShader = shaderMask != prevMask;

            //If the current mask is not the same as the one needed, switch shader.
            if(changedShader)
            {
                //Bind the new shader.
                prevMask = shaderMask;

                //Get the new shader. If not present, load a new one.
                auto newShader = m_ShaderCache.GetOrNull(shaderMask);
                if(newShader == nullptr)
                {
                    newShader = m_ShaderCache.LoadShader(shaderMask, mesh->GetAttribLocations());
                }
                const std::shared_ptr<Shader_GL> currentShader = std::reinterpret_pointer_cast<Shader_GL>(newShader);
                currentProgramId = currentShader->GetProgramId();
                glUseProgram(currentProgramId);
            }

            //If the current material is new or the shader changed, re-upload the material data.
            if (material && (changedMaterial || changedShader))
            {
                auto& matSettings = instanceData.materialData.material->GetSettings();

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
            if (materialBatch && (changedBatch || changedShader))
            {
                auto batchGl = static_cast<MaterialBatch_GL*>(instanceData.materialData.materialBatch.get());

                //Bind the texture
                if(batchGl->HasTexture())
                {
                    auto texture = std::static_pointer_cast<Texture_GL>(batchGl->GetTexture());
                    glActiveTexture(GL_TEXTURE5);
                    glBindTexture(GL_TEXTURE_2D_ARRAY, texture->GetTextureId());

                    //Set the stride uniform
                    glUniform1i(6, batchGl->GetActiveTextureCount());
                }
                if(batchGl->HasUbo())
                {
                    auto uboId = batchGl->GetUboID();
                    glBindBuffer(GL_UNIFORM_BUFFER, uboId);
                    glBindBufferBase(GL_UNIFORM_BUFFER, 2, uboId);
                }
            }

            //Which DrawData is active?
            const bool matrixEnabled = instanceData.attributes.IsAttributeEnabled(DrawAttribute::TRANSFORMATION_MATRIX);
            const bool normalMatrixEnabled = instanceData.attributes.IsAttributeEnabled(DrawAttribute::NORMAL_MATRIX);
            const bool uvModEnabled = instanceData.attributes.IsAttributeEnabled(DrawAttribute::UV_MODIFIER);

            //If transforms are uploaded, bind the transform buffer.
            if(matrixEnabled || normalMatrixEnabled)
            {
                //Make sure the buffer and view are valid.
                assert(instanceData.transformData.dataBuffer != nullptr);

                //Bind the SSBO to the instance data slot (0).
                const auto glTransformGpuBuffer = std::reinterpret_pointer_cast<GpuBuffer_GL>(instanceData.transformData.dataBuffer);

                //Set the binding point that the shader interface block reads from to contain a specific range from the GPU buffer.
                //Shader is hard coded to use slot 0 for the buffer.
                glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, glTransformGpuBuffer->GetBufferId(), static_cast<GLintptr>(instanceData.transformData.dataRange.start), instanceData.transformData.dataRange.totalSize);
            }


            //Check if Uv modifiers are enabled. Bind to slot 3 if used.
            const bool hasUvModifiers = (uvModEnabled && (mesh->GetVertexAttributeMask() & VertexAttribute::UV_MODIFIER_ID) == VertexAttribute::UV_MODIFIER_ID) && instanceData.uvModifierData.dataBuffer != nullptr;
            if(hasUvModifiers)
            {
                assert(instanceData.uvModifierData.dataBuffer != nullptr);

                auto glUvModifierBuffer = static_cast<GpuBuffer_GL*>(instanceData.uvModifierData.dataBuffer.get());
                glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 3, glUvModifierBuffer->GetBufferId(), static_cast<GLintptr>(instanceData.uvModifierData.dataRange.start), instanceData.uvModifierData.dataRange.totalSize);
            }

            //Set the number of instances from the mesh itself in the uniform.
            glUniform1i(0, mesh->GetInstanceCount());

            //If the geometry changed, bind the new geometry.
            if(prevMesh != instanceData.mesh)
            {
                //Bind the VAO of the mesh.
                glBindVertexArray(mesh->GetVaoId());
                prevMesh = instanceData.mesh;
            }

            //Finally draw instanced.
            glDrawElementsInstanced(GL_TRIANGLES, mesh->GetNumIndices(), mesh->GetIndexDataType(), nullptr, instanceData.instanceCount * mesh->GetInstanceCount());
        }

        //Unbind state that may affect other rendering.
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        glBindVertexArray(0);
    }
}
