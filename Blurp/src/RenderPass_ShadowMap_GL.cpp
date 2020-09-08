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

        definitions.emplace_back("POSITIONAL");
        definitions.emplace_back("DIRECTIONAL");

        m_ShaderCache.Init(a_BlurpEngine.GetResourceManager(), sSettings, definitions);

        //Set up the framebuffer for the shadow depth rendering.
        glGenFramebuffers(1, &m_Fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        //Generate the UBO for lights and allocate enough space for the maximum number of lights.
        glGenBuffers(1, &m_LightUbo);
        glBindBuffer(GL_UNIFORM_BUFFER, m_LightUbo);
        glBufferData(GL_UNIFORM_BUFFER, MAX_NUM_LIGHTS * sizeof(PosLightData), nullptr, GL_DYNAMIC_DRAW);   //NOTE: PosLightData is used because it is far bigger than dir light data.
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
        assert(m_DirectionalLights.size() <= MAX_LIGHTS && "Max number of directional lights exceeded!");

        //Calculate bit masks for positional/directional use.
        constexpr std::uint32_t POSITIONAL_BIT = 1 << (NUM_VERTEX_ATRRIBS + NUM_DRAW_ATTRIBS);
        constexpr std::uint32_t DIRECTIONAL_BIT = POSITIONAL_BIT << 1;

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
            //Ensure enough space.
            assert(m_ShadowMapsPositional->GetDimensions().z >= m_PositionalLights.size() * 6 && "Shadow map array has not enough layers for this many lights!");

            //Bind the FBO and attach the depth texture to it.
            glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, std::static_pointer_cast<Texture_GL>(m_ShadowMapsPositional)->GetTextureId(), 0);
            const auto dimensions = m_ShadowMapsPositional->GetDimensions();
            glViewport(0, 0, static_cast<GLsizei>(dimensions.x), static_cast<GLsizei>(dimensions.y));
            glScissor(0, 0, static_cast<GLsizei>(dimensions.x), static_cast<GLsizei>(dimensions.y));

            //Calculate the light data.
            std::vector<PosLightData> posLightData;
            posLightData.resize(m_PositionalLights.size());

            //Use the camera near and far plane for this light projection. Store light depth within the near-far range.
            const auto nearPlane = m_Camera->GetSettings().nearPlane;
            const auto farPlane = m_Camera->GetSettings().farPlane;

            for(int i = 0; i < static_cast<int>(m_PositionalLights.size()); ++i)
            {
                auto& data = m_PositionalLights[i];

                //Shadow map index.
                posLightData[i].shadowMapIndex.x = data.index;

                //Store the light position as well to calculate the light distance from the fragment for depth storing.
                posLightData[i].lightPosition = glm::vec4(data.data, 1.0);

                //Calculate the projection matrix. Aspect is 1 because cubemaps have equal width and height.
                glm::mat4 projection = glm::perspective(glm::radians(90.f), 1.f, nearPlane, farPlane);

                //PV matrices for each face.
                posLightData[i].matrices[0] = projection * glm::lookAt(data.data, data.data + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
                posLightData[i].matrices[1] = projection * glm::lookAt(data.data, data.data + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
                posLightData[i].matrices[2] = projection * glm::lookAt(data.data, data.data + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
                posLightData[i].matrices[3] = projection * glm::lookAt(data.data, data.data + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
                posLightData[i].matrices[4] = projection * glm::lookAt(data.data, data.data + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
                posLightData[i].matrices[5] = projection * glm::lookAt(data.data, data.data + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));
            }

            //Upload light data.
            glBindBuffer(GL_UNIFORM_BUFFER, m_LightUbo);
            glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_LightUbo);
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
                std::uint32_t shaderMask = static_cast<std::uint32_t>(mesh->GetVertexAttributeMask()) | (drawAttribs.GetMask() << NUM_VERTEX_ATRRIBS) | POSITIONAL_BIT;

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

                //Set the uniform for the far plane.
                glUniform1f(1, farPlane);

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

        /*
         * Directional lights.
         */

        if (!m_DirectionalLights.empty() && m_ShadowMapsDirectional != nullptr)
        {
            //Ensure there's enough space in the texture.
            assert(m_ShadowMapsDirectional->GetDimensions().z >= m_DirectionalLights.size() * m_NumDirectionalCascades && "Shadow map array has not enough layers for this many lights!");

            //Bind the FBO and attach the depth texture to it.
            glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, std::static_pointer_cast<Texture_GL>(m_ShadowMapsDirectional)->GetTextureId(), 0);
            const auto dimensions = m_ShadowMapsDirectional->GetDimensions();
            glViewport(0, 0, static_cast<GLsizei>(dimensions.x), static_cast<GLsizei>(dimensions.y));
            glScissor(0, 0, static_cast<GLsizei>(dimensions.x), static_cast<GLsizei>(dimensions.y));

            //Vector containing the padded data to be uploaded to the GPU.
            //Format: NumCascades(vec4), CamPosCascadeDistance(vec4)
            DirLightData data;
            data.numCascades.x = m_NumDirectionalCascades;
            int lIndex = 0;
            for(auto& dirLight : m_DirectionalLights)
            {
                data.shadowIndices[lIndex].x = dirLight.index;
                ++lIndex;
            }

            /*const auto nearPlane = m_Camera->GetSettings().nearPlane;
            const auto farPlane = m_Camera->GetSettings().farPlane;*/

            //Iterate over all directional lights and set up their matrices.
            std::vector<DirCascade> cascades;
            cascades.reserve(m_NumDirectionalCascades * m_DirectionalLights.size());

            //Matrix used to convert a point from camera space to world space.
            glm::mat4 camToWorld = m_Camera->GetTransform().GetTransformation();

            //Horizontal and vertical FOV.
            auto& camSettings = m_Camera->GetSettings();
            float verticalFovTanHalved = tanf(glm::radians(camSettings.fov / 2.0f));
            float aspectRatio = camSettings.width / camSettings.height;
            float horizontalFovTanHalved = verticalFovTanHalved * aspectRatio;

            for (int i = 0; i < static_cast<int>(m_DirectionalLights.size()); ++i)
            {
                auto& lightData = m_DirectionalLights[i];

                //Find an up vector.
                glm::vec3 up = Transform::GetWorldUp();
                if (fabsf(glm::dot(up, lightData.data)) <= 0.f + std::numeric_limits<float>::epsilon())
                {
                    up = Transform::GetWorldRight();
                }

                //Matrices transforming Z to light direction, and camera to light space.
                glm::mat4 lightMatrix = glm::lookAt(glm::vec3(0.f, 0.f, 0.f), lightData.data, up);
                glm::mat4 camToLight = lightMatrix * camToWorld;

                float lastFar = 0.f;

                for (std::uint32_t cascade = 0; cascade < m_NumDirectionalCascades; ++cascade)
                {
                    glm::vec4 cameraFrustumCorners[8];      //The 8 corners of the frustum.

                    //Calculate the far and near Z positions of this cascade. The last cascade is goes all the way to the far plane.
                    float nearZ = lastFar;
                    float farZ = nearZ + m_DirectionalCascadeDistances[cascade];
                    lastFar = farZ;

                    //Calculate near and far X using some trigonometry.
                    float nearX = nearZ * horizontalFovTanHalved;
                    float farX = farZ * horizontalFovTanHalved;

                    //Calculate Y using the vertical FOV.
                    float nearY = nearZ * verticalFovTanHalved;
                    float farY = farZ * verticalFovTanHalved;

                    //Because the camera frustum points in negative Z direction, invert the Z coordinates.
                    farZ *= -1.f;
                    nearZ *= -1.f;

                    //All 8 corners.
                    cameraFrustumCorners[0] = { -nearX, -nearY, nearZ, 1.f };
                    cameraFrustumCorners[1] = { +nearX, -nearY, nearZ, 1.f };
                    cameraFrustumCorners[2] = { -nearX, +nearY, nearZ, 1.f };
                    cameraFrustumCorners[3] = { +nearX, +nearY, nearZ, 1.f };
                    cameraFrustumCorners[4] = { -farX, -farY, farZ, 1.f };
                    cameraFrustumCorners[5] = { +farX, -farY, farZ, 1.f };
                    cameraFrustumCorners[6] = { -farX, +farY, farZ, 1.f };
                    cameraFrustumCorners[7] = { +farX, +farY, farZ, 1.f };

                    //Min and max coordinates on each axis.
                    glm::vec3 min(std::numeric_limits<float>::max());
                    glm::vec3 max(-std::numeric_limits<float>::max());

                    //Transform to world space and then to light space. All corners are now aligned with the light (Z = light dir).
                    //Then compare the coordinates to find the min and max of each axis.
                    for(int corner = 0; corner < 8; ++corner)
                    {
                        cameraFrustumCorners[corner] = camToLight * cameraFrustumCorners[corner];

                        min.x = std::fmin(min.x, cameraFrustumCorners[corner].x);
                        min.y = std::fmin(min.y, cameraFrustumCorners[corner].y);
                        min.z = std::fmin(min.z, cameraFrustumCorners[corner].z);

                        max.x = std::fmax(max.x, cameraFrustumCorners[corner].x);
                        max.y = std::fmax(max.y, cameraFrustumCorners[corner].y);
                        max.z = std::fmax(max.z, cameraFrustumCorners[corner].z);
                    }

                    ////Dimensions of the frustum from the lights perspective.
                    //float widthH = fabsf(max.x - min.x) / 2.f;
                    //float heightH = fabsf(max.y - min.y) / 2.f;
                    //float depth = fabsf(max.z - min.z);
                    //const float depthH = depth / 2.f;

                    //const float cascadeDepthH = fabsf((farZ - nearZ) / 2.f);
                    //const float frustumCenterDistance = -nearZ + cascadeDepthH;

                    ////Light is always far dist behind the camera. Added on top of that is the frustum center distance projected onto the light direction.
                    ////The dot product is -1 when the light is right against the camera, which means only FAR_DIST is required.
                    ////The dot product is 1 when the camera is facing away from the light, which means the FAR_DIST has to be twice as big.
                    //const float scaleFactor = glm::dot(lightData.data, m_Camera->GetTransform().GetBack());

                    ////The light is at least FAR_DIST away to cast shadows of objects behind the camera.
                    ////When the camera looks into the light, the dot product cancels this out by being negative.
                    //const float lightDistance = camSettings.farPlane + depthH + (frustumCenterDistance * scaleFactor);
                    //const float farPlaneDistance = lightDistance + depthH;  //Far plane need to see the light distance + whatever is left of the frustum cascade.


                    //glm::vec3 frustumCenterWorld = m_Camera->GetTransform().GetTranslation() + (m_Camera->GetTransform().GetBack() * frustumCenterDistance);
                    //glm::vec3 lightPosWorld = frustumCenterWorld - (lightData.data * lightDistance);   //Move far plane + edge of frustum from light perspective.

                    //lightMatrix = glm::lookAt(lightPosWorld, frustumCenterWorld, up);
                    //auto projectionMatrix = glm::ortho(-widthH, widthH, -heightH, heightH, 0.1f, farPlaneDistance); //This distance will cover from the light till the end of the frustum. Added 1 for bias.



                    //Simpler way in world space without light translation. 
                    const float depth = max.z - min.z;
                    const float depthH = depth / 2.f;
                    const float scaleFactor = glm::dot(lightData.data, m_Camera->GetTransform().GetBack());
                    const float cascadeDepthH = fabsf((farZ - nearZ) / 2.f);
                    const float frustumCenterDistance = -nearZ + cascadeDepthH;
                    const float lightDistance = camSettings.farPlane + depthH + (frustumCenterDistance * scaleFactor);
                    auto projectionMatrix = glm::ortho(min.x, max.x, min.y, max.y, min.z - lightDistance, max.z + camSettings.farPlane); //This distance will cover from the light till the end of the frustum. Added 1 for bias.



                    auto pv = projectionMatrix * lightMatrix;

                    //Combine PV matrices.
                    auto cascadeClipDepth = (m_Camera->GetProjectionMatrix() * glm::vec4(0.f, 0.f, farZ, 1.f));
                    cascades.push_back(DirCascade{ cascadeClipDepth, pv });
                }
            }

            //Upload directional light matrices.
            glBindBuffer(GL_UNIFORM_BUFFER, m_LightUbo);
            glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_LightUbo);
            glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(DirLightData), &data);


            //Upload the directional matrices for each light and cascade. Store the result in the view that was provided. Bind to the right shader slot and range.
            (*m_DirShadowTransformView) = m_DirShadowTransformBuffer->WriteData<DirCascade>(m_DirShadowTransformOffset, static_cast<std::uint32_t>(cascades.size()), 16, &cascades[0]);
            glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 3, static_cast<GpuBuffer_GL*>(m_DirShadowTransformBuffer.get())->GetBufferId(), static_cast<GLintptr>(m_DirShadowTransformView->start), m_DirShadowTransformView->totalSize);


            /*
             * Actual drawing.
             */


            //Cached last shader mask.
            std::shared_ptr<Mesh> prevMesh;
            std::uint32_t prevMask = 0;

            //Loop over geometry and draw.
            for (std::uint32_t i = 0; i < m_DrawDataCount; ++i)
            {
                auto& drawData = m_DrawDataPtr[i];
                auto mesh = std::static_pointer_cast<Mesh_GL>(m_DrawDataPtr[i].mesh);
                auto& drawAttribs = m_DrawDataPtr[i].attributes;

                //TODO only mask the things that matter for this shader.
                std::uint32_t shaderMask = static_cast<std::uint32_t>(mesh->GetVertexAttributeMask()) | (drawAttribs.GetMask() << NUM_VERTEX_ATRRIBS) | DIRECTIONAL_BIT;

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
                lightIndices.reserve(indices.dirIndices.size() + 4);

                //Add the index.
                lightIndices.push_back(static_cast<std::int32_t>(indices.dirIndices.size()));
                lightIndices.push_back(0);
                lightIndices.push_back(0);
                lightIndices.push_back(0);

                lightIndices.insert(lightIndices.end(), indices.dirIndices.begin(), indices.dirIndices.end());
                const int paddingRequired = (~lightIndices.size() + 1) & (4 - 1);
                for (int p = 0; p < paddingRequired; ++p)
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
    }
}
