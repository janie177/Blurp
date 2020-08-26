#include "opengl/RenderPass_ShadowMap_GL.h"
#include "opengl/Texture_GL.h"
#include "Mesh.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



#include "opengl/GpuBuffer_GL.h"
#include "opengl/Mesh_GL.h"
#include "opengl/RenderPass_Forward_GL.h"
#include "opengl/Shader_GL.h"

namespace blurp
{
    bool RenderPass_ShadowMap_GL::OnLoad(BlurpEngine& a_BlurpEngine)
    {
        /*
         * Set up a shader cache.
         */
        auto shaderPath = a_BlurpEngine.GetEngineSettings().shadersPath + "opengl/";
        auto vertex = "Default_ShadowMap.vs";
        auto geometry = "Default_ShadowMap.gs";
        auto fragment = "Default_ShadowMap.fs";

        FileReader vertexReader(shaderPath + vertex);
        FileReader geometryReader(shaderPath + geometry);
        FileReader fragmentReader(shaderPath + fragment);

        if (!vertexReader.Open() || !fragmentReader.Open() || !geometryReader.Open())
        {
            throw std::exception("Could not find shadow map shaders at path specified.");
        }

        auto vertexSrc = vertexReader.ToArray();
        auto geometrySrc = geometryReader.ToArray();
        auto fragmentSrc = fragmentReader.ToArray();

        ShaderSettings sSettings;
        sSettings.vertexShaderSource = vertexSrc.get();
        sSettings.geometryShaderSource = geometrySrc.get();
        sSettings.fragmentShaderSource = fragmentSrc.get();
        sSettings.type = ShaderType::GRAPHICS;

        //Preprocessor definitions at their respective bit indices.
        std::vector<std::string> definitions;

        //TODO only enable the attributes that make sense: position, matrix. Get the stride etc from the vertexSettings per mesh and enable vbo/attribs manually.
        //Add every mesh bitmask to it as the first set of bits.
        for (auto& attrib : VERTEX_ATTRIBUTES)
        {
            definitions.emplace_back(VertexSettings::GetVertexAttributeInfo(attrib).defineName);
        }

        //Add the draw attribute definitions.
        for (auto& attrib : DRAW_ATTRIBUTES)
        {
            auto found = DRAW_ATTRIBUTE_INFO.find(attrib);
            assert(found != DRAW_ATTRIBUTE_INFO.end());
            definitions.emplace_back(found->second.defineName);
        }

        m_ShaderCache.Init(a_BlurpEngine.GetResourceManager(), sSettings, definitions);

        //Set up the framebuffer for the shadow depth rendering.
        glGenFramebuffers(1, &m_Fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        //Generate the UBO for positional lights and allocate enough space for the maximum number of lights.
        glGenBuffers(1, &m_PosLightUbo);
        glBindBuffer(GL_UNIFORM_BUFFER, m_PosLightUbo);
        glBufferData(GL_UNIFORM_BUFFER, MAX_NUM_LIGHTS * sizeof(PosLightData), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        //Generate the UBO to store light indices in.
        glGenBuffers(1, &m_LightIndicesUbo);
        glBindBuffer(GL_UNIFORM_BUFFER, m_LightIndicesUbo);
        glBufferData(GL_UNIFORM_BUFFER, MAX_NUM_LIGHTS + 1 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        return true;
    }

    bool RenderPass_ShadowMap_GL::OnDestroy(BlurpEngine& a_BlurpEngine)
    {
        glDeleteBuffers(1, &m_Fbo);
        return true;
    }

    void RenderPass_ShadowMap_GL::Execute()
    {
        //Ensure size if not exceeded.
        assert(m_PositionalLights.size() <= MAX_NUM_LIGHTS && "Max positional light count for shadow mapping exceeded!");

        //Render state
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glFrontFace(GL_CCW);

        /*
         * PointLights.
         */

        if(!m_PositionalLights.empty() && m_ShadowMapsPositional != nullptr)
        {
            //Bind the FBO and attach the depth texture to it.
            glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, std::static_pointer_cast<Texture_GL>(m_ShadowMapsPositional)->GetTextureId(), 0);
            const auto dimensions = m_ShadowMapsPositional->GetDimensions();
            glViewport(0, 0, static_cast<GLsizei>(dimensions.x), static_cast<GLsizei>(dimensions.y));
            glScissor(0, 0, static_cast<GLsizei>(dimensions.x), static_cast<GLsizei>(dimensions.y));

            //Calculate the light data.
            std::vector<PosLightData> posLightData;
            posLightData.resize(m_PositionalLights.size());

            for(int i = 0; i < static_cast<int>(m_PositionalLights.size()); ++i)
            {
                auto& data = m_PositionalLights[i];

                //Shadow map index.
                posLightData[i].shadowMapIndex.x = data.index;

                //Calculate the projection matrix. Aspect is 1 because cubemaps have equal width and height.
                glm::mat4 projection = glm::perspective(90.f, 1.f, data.nearPlane, data.farPlane);

                //PV matrices for each face.
                posLightData[i].matrices[0] = projection * glm::lookAt(data.data, data.data + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
                posLightData[i].matrices[1] = projection * glm::lookAt(data.data, data.data + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
                posLightData[i].matrices[2] = projection * glm::lookAt(data.data, data.data + glm::vec3(0.0, 1.0, 0.0), glm::vec3(1.0, 0.0, 0.0));
                posLightData[i].matrices[3] = projection * glm::lookAt(data.data, data.data + glm::vec3(0.0, -1.0, 0.0), glm::vec3(1.0, 0.0, 0.0));
                posLightData[i].matrices[4] = projection * glm::lookAt(data.data, data.data + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0));
                posLightData[i].matrices[5] = projection * glm::lookAt(data.data, data.data + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, 1.0, 0.0));
            }

            //Upload light data.
            glBindBuffer(GL_UNIFORM_BUFFER, m_PosLightUbo);
            glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_PosLightUbo);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(PosLightData) * posLightData.size(), &posLightData[0]);

            //Cached last shader mask.
            std::shared_ptr<Mesh> prevMesh;
            std::uint32_t prevMask = 0;

            //Loop over geometry and draw.
            for(std::uint32_t i = 0; i < m_DrawDataCount; ++i)
            {
                auto& drawData = m_DrawDataPtr[i];
                auto mesh = std::static_pointer_cast<Mesh_GL>(m_DrawDataPtr[i].mesh);
                auto& drawAttribs = m_DrawDataPtr[i].attributes;

                //TODO only mask the things that matter for this shader.
                std::uint32_t shaderMask = static_cast<std::uint32_t>(mesh->GetVertexAttributeMask()) | (drawAttribs.GetMask() << NUM_VERTEX_ATRRIBS);

                //Has the shader changed?
                const bool changedShader = shaderMask != prevMask;

                //If the current mask is not the same as the one needed, switch shader.
                if (changedShader)
                {
                    //Bind the new shader.
                    prevMask = shaderMask;

                    //Get the new shader. If not present, load a new one.
                    auto newShader = m_ShaderCache.GetOrNull(shaderMask);
                    if (newShader == nullptr)
                    {
                        newShader = m_ShaderCache.LoadShader(shaderMask, mesh->GetAttribLocations());
                    }
                    const std::shared_ptr<Shader_GL> currentShader = std::reinterpret_pointer_cast<Shader_GL>(newShader);
                    const GLuint currentProgramId = currentShader->GetProgramId();
                    glUseProgram(currentProgramId);
                }

                //Which DrawData is active?
                const bool matrixEnabled = drawData.attributes.IsAttributeEnabled(DrawAttribute::TRANSFORMATION_MATRIX);
                const bool normalMatrixEnabled = drawData.attributes.IsAttributeEnabled(DrawAttribute::NORMAL_MATRIX);

                //If transforms are uploaded, bind the transform buffer.
                if (drawData.transformData.dataBuffer != nullptr && (matrixEnabled || normalMatrixEnabled))
                {
                    //Bind the SSBO to the instance data slot (0).
                    const auto glTransformGpuBuffer = std::reinterpret_pointer_cast<GpuBuffer_GL>(drawData.transformData.dataBuffer);

                    //Set the binding point that the shader interface block reads from to contain a specific range from the GPU buffer.
                    //Shader is hard coded to use slot 0 for the buffer.
                    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, glTransformGpuBuffer->GetBufferId(), static_cast<GLintptr>(drawData.transformData.dataRange.start), drawData.transformData.dataRange.totalSize);
                }

                //Set the number of instances from the mesh itself in the uniform.
                glUniform1i(0, mesh->GetInstanceCount());

                //Calculate light indices. Has to be in vec4 format padded to vec4 size.
                const LightIndexData& indices = m_LightIndices[i];
                std::vector<std::int32_t> lightIndices;
                lightIndices.reserve(indices.posIndices.size() + 4);

                //Add the index.
                lightIndices.push_back(static_cast<std::int32_t>(indices.posIndices.size()));
                lightIndices.push_back(0);
                lightIndices.push_back(0);
                lightIndices.push_back(0);

                lightIndices.insert(lightIndices.end(), indices.posIndices.begin(), indices.posIndices.end());
                const int paddingRequired = (~lightIndices.size() + 1) & (4 - 1);
                for(int p = 0; p < paddingRequired; ++p)
                {
                    lightIndices.push_back(0);
                }

                //Upload light index data
                glBindBuffer(GL_UNIFORM_BUFFER, m_LightIndicesUbo);
                glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_LightIndicesUbo);
                glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(std::int32_t) * lightIndices.size(), &lightIndices[0]);

                //If the geometry changed, bind the new geometry.
                if (prevMesh != drawData.mesh)
                {
                    //TODO bind VBO manually and enable only required attributes.
                    //Bind the VAO of the mesh.
                    glBindVertexArray(mesh->GetVaoId());
                }

                //Finally draw instanced.
                glDrawElementsInstanced(GL_TRIANGLES, mesh->GetNumIndices(), mesh->GetIndexDataType(), nullptr, drawData.instanceCount * mesh->GetInstanceCount());
            }
            

        }

        //TODO directional light with cascading.
    }
}
