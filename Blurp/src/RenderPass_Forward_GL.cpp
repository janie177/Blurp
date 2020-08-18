#include "opengl/RenderPass_Forward_GL.h"
#include "BlurpEngine.h"
#include "FileReader.h"
#include "opengl/GpuBuffer_GL.h"
#include "opengl/MaterialBatch_GL.h"
#include "opengl/Mesh_GL.h"
#include "opengl/RenderTarget_GL.h"
#include "opengl/Shader_GL.h"
#include "opengl/Texture_GL.h"

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
        definitions.emplace_back("INSTANCE_DATA_M");
        definitions.emplace_back("INSTANCE_DATA_IM");

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

        return true;
    }

    bool RenderPass_Forward_GL::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        glDeleteBuffers(1, &m_StaticDataUbo);
        return true;
    }

    void RenderPass_Forward_GL::Execute()
    {
        /*
         * Render state and target preparing.
         */

        //Clear the target buffer.
        const auto fboId = reinterpret_cast<RenderTarget_GL*>(m_Output.get())->GetFrameBufferId();

        //Set output data targets and reset the framebuffer color and depth.
        glBindFramebuffer(GL_FRAMEBUFFER, fboId);
        const auto viewPort = m_Output->GetViewPort();
        glViewport(static_cast<int>(viewPort.r), static_cast<int>(viewPort.g), static_cast<int>(viewPort.b), static_cast<int>(viewPort.a));
        const auto scissorRect = m_Output->GetScissorRect();
        glScissor(static_cast<int>(scissorRect.r), static_cast<int>(scissorRect.g), static_cast<int>(scissorRect.b), static_cast<int>(scissorRect.a));
        const auto clearColor = m_Output->GetClearColor();

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        //Dont clear to not kill the skybox. Move this to a clear pass. TODO
        //glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);


        /*
         * Global data setup that is used for all draw calls.
         */

        //Shader attribute mask.
        std::uint32_t prevMask = 0;
        std::shared_ptr<Material> prevMaterial;
        std::shared_ptr<MaterialBatch> prevMaterialBatch;
        std::shared_ptr<Mesh> prevMesh;

        //To set uniforms.
        GLuint currentProgramId = 0;

        //Bits used for materials and uploaded data.
        constexpr std::uint32_t matSingleBit = 1 << (NUM_MATERIAL_ATRRIBS + NUM_VERTEX_ATRRIBS);
        constexpr std::uint32_t matBatchBit = 1 << (NUM_MATERIAL_ATRRIBS + NUM_VERTEX_ATRRIBS + 1);
        constexpr std::uint32_t uploadMBit = 1 << (NUM_MATERIAL_ATRRIBS + NUM_VERTEX_ATRRIBS + 2);
        constexpr std::uint32_t uploadIMBit = 1 << (NUM_MATERIAL_ATRRIBS + NUM_VERTEX_ATRRIBS + 3);

        //Prepare the lights for uploading to the GPU in a packed format.
        if(m_ReuploadLights)
        {
            //Add the total number of point, spot and directional lights together.
            glm::vec3 combined = m_LightCounts + m_ShadowCounts;
            std::size_t totalLightCount = static_cast<std::size_t>(combined.x) + static_cast<std::size_t>(combined.y) + static_cast<std::size_t>(combined.z);

            assert(totalLightCount <= MAX_LIGHTS && "Maximum light count exceeded!");

            //Only upload light data if there is actually lights present.
            if (totalLightCount > 0)
            {
                std::vector<LightDataPacked> lightBuffer;
                lightBuffer.resize(totalLightCount);

                //Indices for each light data type. Shadow after non-shadow. Start index calculated by adding the last index + its size.
                int pIndex = 0;
                int pSIndex = pIndex + static_cast<int>(m_LightCounts.x);
                int sIndex = pSIndex + static_cast<int>(m_ShadowCounts.x);
                int sSIndex = sIndex + static_cast<int>(m_LightCounts.y);
                int dIndex = sSIndex + static_cast<int>(m_ShadowCounts.y);
                int dSIndex = dIndex + static_cast<int>(m_LightCounts.z);

                for (auto& lData : m_LightData)
                {
                    switch (lData.light->GetType())
                    {
                    case LightType::LIGHT_POINT:
                    {
                        PointLight* light = static_cast<PointLight*>(lData.light.get());

                        //No shadow.
                        if (lData.shadowMapIndex < 0)
                        {
                            LightDataPacked& data = lightBuffer[pIndex];

                            data.vec1 = glm::vec4(light->GetColor(), lData.light->GetIntensity());
                            data.vec2 = glm::vec4(light->GetPosition(), lData.shadowMapIndex);

                            ++pIndex;
                        }
                        //Shadow.
                        else
                        {
                            LightDataPacked& data = lightBuffer[pSIndex];

                            data.vec1 = glm::vec4(light->GetColor(), lData.light->GetIntensity());
                            data.vec2 = glm::vec4(light->GetPosition(), lData.shadowMapIndex);

                            data.shadowMatrix = lData.shadowMatrix;

                            ++pSIndex;
                        }
                    }
                    break;
                    case LightType::LIGHT_SPOT:
                    {
                        SpotLight* light = static_cast<SpotLight*>(lData.light.get());

                        //No shadow.
                        if (lData.shadowMapIndex < 0)
                        {
                            LightDataPacked& data = lightBuffer[sIndex];

                            data.vec1 = glm::vec4(light->GetColor(), lData.light->GetIntensity());
                            data.vec2 = glm::vec4(light->GetPosition(), lData.shadowMapIndex);
                            data.vec3 = glm::vec4(light->GetDirection(), light->GetAngle());

                            ++sIndex;
                        }
                        //Shadow.
                        else
                        {
                            LightDataPacked& data = lightBuffer[sSIndex];

                            data.vec1 = glm::vec4(light->GetColor(), lData.light->GetIntensity());
                            data.vec2 = glm::vec4(light->GetPosition(), lData.shadowMapIndex);
                            data.vec3 = glm::vec4(light->GetDirection(), light->GetAngle());

                            data.shadowMatrix = lData.shadowMatrix;

                            ++sSIndex;
                        }
                    }
                    break;
                    case LightType::LIGHT_DIRECTIONAL:
                    {
                        DirectionalLight* light = static_cast<DirectionalLight*>(lData.light.get());

                        //No shadow.
                        if (lData.shadowMapIndex < 0)
                        {
                            LightDataPacked& data = lightBuffer[dIndex];

                            data.vec1 = glm::vec4(light->GetColor(), lData.light->GetIntensity());
                            data.vec2 = glm::vec4(light->GetDirection(), lData.shadowMapIndex);

                            data.shadowMatrix = lData.shadowMatrix;

                            ++dIndex;
                        }
                        //Shadow.
                        else
                        {
                            LightDataPacked& data = lightBuffer[dSIndex];

                            data.vec1 = glm::vec4(light->GetColor(), lData.light->GetIntensity());
                            data.vec2 = glm::vec4(light->GetDirection(), lData.shadowMapIndex);

                            data.shadowMatrix = lData.shadowMatrix;

                            ++dSIndex;
                        }
                    }
                    break;
                    default:
                        break;
                    }
                }

                //Upload the data to the GPU.
                glBindBuffer(GL_UNIFORM_BUFFER, m_LightUbo);
                glBufferSubData(GL_UNIFORM_BUFFER, 0, lightBuffer.size() * sizeof(LightDataPacked), static_cast<void*>(&lightBuffer[0]));
            }

            //Lights uploaded so no need to do it again if they are not changed.
            m_ReuploadLights = false;
        }

        //Bind the shadow samplers if they are specified and there is lights that use shadows.
        if (m_DirectionalShadowMaps != nullptr && m_ShadowCounts.z)
        {
            glActiveTexture(GL_TEXTURE6);
            glBindTexture(GL_TEXTURE_2D_ARRAY, reinterpret_cast<Texture_GL*>(m_DirectionalShadowMaps.get())->GetTextureId());
        }
        if (m_PointSpotShadowMaps != nullptr && (m_ShadowCounts.x || m_ShadowCounts.y))
        {
            glActiveTexture(GL_TEXTURE7);
            glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, reinterpret_cast<Texture_GL*>(m_PointSpotShadowMaps.get())->GetTextureId());
        }
        

        //Upload the static data to the GPU such as the camera and light counts.
        StaticData staticData;
        staticData.pv = m_Camera->GetProjectionMatrix() * m_Camera->GetViewMatrix();
        staticData.camPos = glm::vec4(m_Camera->GetTransform().GetTranslation(), 0.f);
        staticData.numLights = glm::vec4(m_LightCounts, 0.f);
        staticData.numShadows = glm::vec4(m_ShadowCounts, 0.f);
        staticData.ambientLight = glm::vec4(m_AmbientLight, 0.f);

        glBindBuffer(GL_UNIFORM_BUFFER, m_StaticDataUbo);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(staticData), static_cast<void*>(&staticData));


        /*
         * Actual drawing.
         */


        for(auto& instanceData : m_DrawQueue)
        {
            assert(instanceData.mesh != nullptr && "Mesh cannot be nullptr!");
            assert(instanceData.count > 0 && "Cannot draw 0 instances of mesh!");
            Mesh_GL* mesh = static_cast<Mesh_GL*>(instanceData.mesh.get());

            //Calculate the new mask.
            const bool useMaterial = instanceData.materialData.material != nullptr;
            const bool useMaterialBatch = instanceData.materialData.materialBatch != nullptr;

            //Has the material and materialbatch changed?
            const bool changedMaterial = prevMaterial != instanceData.materialData.material;
            const bool changedBatch = prevMaterialBatch != instanceData.materialData.materialBatch;

            //Ensure that either only one is enabled, or both are disabled. Never both enabled.
            assert((useMaterial != useMaterialBatch) || (!useMaterial && !useMaterialBatch));

            //Shader mask matching the vertex layout.
            std::uint32_t shaderMask = static_cast<std::uint32_t>(mesh->GetVertexAttributeMask());

            //If materials or batches are enabled, append those to the mask.
            if(useMaterialBatch)
            {
                //Ensure that material ID is enabled when using a batch (to fetch the material at the right index). 
                assert((instanceData.mesh->GetVertexAttributeMask() & VertexAttribute::MATERIAL_ID) == VertexAttribute::MATERIAL_ID && "To use a material batch, the provided mesh needs to have material IDs defined in its attributes!");

                shaderMask = shaderMask | matBatchBit | (static_cast<std::uint32_t>(instanceData.materialData.materialBatch->GetMask()) << NUM_VERTEX_ATRRIBS);
            }
            else if(useMaterial)
            {
                shaderMask = shaderMask | matSingleBit | (static_cast<std::uint32_t>(instanceData.materialData.material->GetSettings().GetMask()) << NUM_VERTEX_ATRRIBS);
            }

            //Uploaded data for this shader masking
            if(instanceData.transformData.transform)
            {
                shaderMask |= uploadMBit;
            }
            if(instanceData.transformData.inverseTransform)
            {
                shaderMask |= uploadIMBit;
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
            if (useMaterial && (changedMaterial || changedShader))
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
            if (useMaterialBatch && (changedBatch || changedShader))
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

            //If transforms are uploaded, bind the transform buffer.
            if(m_TransformBuffer != nullptr && instanceData.transformData.transform || instanceData.transformData.inverseTransform)
            {
                //Bind the SSBO to the instance data slot (0).
                const auto glTransformGpuBuffer = std::reinterpret_pointer_cast<GpuBuffer_GL>(m_TransformBuffer);

                //Set the binding point that the shader interface block reads from to contain a specific range from the GPU buffer.
                //Shader is hard coded to use slot 0 for the buffer.
                glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, glTransformGpuBuffer->GetBufferId(), static_cast<GLintptr>(instanceData.transformData.dataRange.start), instanceData.transformData.dataRange.size);
            }


            //Check if Uv modifiers are enabled. Bind to slot 3 if used.
            const bool hasUvModifiers = (m_UvModifierBuffer != nullptr && (mesh->GetVertexAttributeMask() & VertexAttribute::UV_MODIFIER_ID) == VertexAttribute::UV_MODIFIER_ID);
            if(hasUvModifiers)
            {
                auto glUvModifierBuffer = static_cast<GpuBuffer_GL*>(m_UvModifierBuffer.get());
                glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 3, glUvModifierBuffer->GetBufferId(), static_cast<GLintptr>(instanceData.uvModifierData.dataRange.start), instanceData.uvModifierData.dataRange.size);
            }

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

        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        glBindVertexArray(0);
    }
}
