#include "MeshLoader.h"
#include <stb_image.h>
#include <MaterialFile.h>
#include <filesystem>
#include <iostream>
#include <MeshFile.h>

#include "../Blurp/Include/api/Transform.h"

bool hasEnding(std::string const& fullString, std::string const& ending)
{
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    }
    else {
        return false;
    }
}

GLTFScene LoadMesh(const MeshLoaderSettings& a_Settings, blurp::RenderResourceManager& a_ResourceManager, bool a_BakeTransforms, bool a_ForceRecompileMaterials, bool a_ForceRecompileMeshes)
{
    fx::gltf::Document file;
    GLTFScene output;

    //Allow unlimited file size.
    std::uint32_t maxUInt = std::numeric_limits<std::uint32_t>::max();
    fx::gltf::ReadQuotas quotas{ maxUInt, maxUInt, maxUInt };

    std::string fileNameLowerCase = a_Settings.fileName;
    std::for_each(fileNameLowerCase.begin(), fileNameLowerCase.end(), [](char& c) {
        c = ::tolower(c);
    });

    if (hasEnding(fileNameLowerCase, ".gltf"))
    {
        try
        {
            file = fx::gltf::LoadFromText((a_Settings.path + a_Settings.fileName), quotas);
        }
        catch (std::exception e)
        {
            std::cout << e.what() << std::endl;
            return output;
        }
    }
    else if (hasEnding(fileNameLowerCase, ".glb"))
    {
        try
        {
            file = fx::gltf::LoadFromBinary((a_Settings.path + a_Settings.fileName), quotas);
        }
        catch (std::exception e)
        {
            std::cout << e.what() << std::endl;
            return output;
        }
    }

    //Keep track of the materials that are reused.
    std::vector<std::shared_ptr<blurp::Material>> materials;

    for(auto& material : file.materials)
    {
        const std::string materialFileName = a_Settings.fileName + "_Material_" + std::to_string(materials.size());
        const std::string path = a_Settings.path + "blurpmats/";
        const std::string materialFileFullPath = path + materialFileName;

        //If the file already exits, load it and continue. This prevents regenerating every time.
        if(!a_ForceRecompileMaterials && std::filesystem::exists((materialFileFullPath + ".blurpmat")))
        {
            materials.push_back(blurp::LoadMaterial(a_ResourceManager, materialFileFullPath));
            continue;
        }

        //NOTE: GLTF does not support bumpmapping/parallaxmapping/heightmaps.
        blurp::MaterialInfo materialInfo;

        LoadedImageInformation imagePtrs[5];

        //Diffuse Texture and Alpha channel
        if (!material.pbrMetallicRoughness.baseColorTexture.empty())
        {
            materialInfo.mask.EnableAttribute(blurp::MaterialAttribute::DIFFUSE_TEXTURE);

            imagePtrs[0] = LoadTexture(file, material.pbrMetallicRoughness.baseColorTexture.index, a_Settings.path, 0);

            auto& p = imagePtrs[0];
            assert(p.channels == 3 || p.channels == 4);

            //Enable alpha if an A channel is provided.
            if(imagePtrs[0].channels == 4)
            {
                materialInfo.mask.EnableAttribute(blurp::MaterialAttribute::ALPHA_TEXTURE);
            }

            auto samplerId = file.textures[material.pbrMetallicRoughness.baseColorTexture.index].sampler;
            if (samplerId > -1)
            {
                auto& sampler = file.samplers[samplerId];
                materialInfo.settings.diffuse.minFilter = MinFromGL(static_cast<std::uint16_t>(sampler.minFilter));
                materialInfo.settings.diffuse.magFilter = MagFromGL(static_cast<std::uint16_t>(sampler.magFilter));
                materialInfo.settings.diffuse.wrapMode = WrapFromGL(static_cast<std::uint16_t>(sampler.wrapS));

                //Enable mipmapping if used for minification.
                if(materialInfo.settings.diffuse.minFilter == blurp::MinFilterType::MIPMAP_NEAREST || materialInfo.settings.diffuse.minFilter == blurp::MinFilterType::MIPMAP_LINEAR)
                {
                    materialInfo.settings.diffuse.generateMipMaps = true;
                }
                else
                {
                    materialInfo.settings.diffuse.generateMipMaps = false;
                }
            }
        }
        //diffuse and alpha constant.
        else if (NotEmpty(material.pbrMetallicRoughness.baseColorFactor))
        {
            materialInfo.mask.EnableAttribute(blurp::MaterialAttribute::DIFFUSE_CONSTANT_VALUE);
            materialInfo.mask.EnableAttribute(blurp::MaterialAttribute::ALPHA_CONSTANT_VALUE);
            materialInfo.alpha.constant = material.pbrMetallicRoughness.baseColorFactor[3];
            float r = material.pbrMetallicRoughness.baseColorFactor[0];
            float g = material.pbrMetallicRoughness.baseColorFactor[1];
            float b = material.pbrMetallicRoughness.baseColorFactor[2];
            materialInfo.diffuse.constant = { r, g, b };
        }

        //Normal Texture
        if (!material.normalTexture.empty())
        {
            materialInfo.mask.EnableAttribute(blurp::MaterialAttribute::NORMAL_TEXTURE);
            imagePtrs[1] = LoadTexture(file, material.normalTexture.index, a_Settings.path, 0);

            auto& p = imagePtrs[1];
            assert(p.channels == 3);

            auto samplerId = file.textures[material.normalTexture.index].sampler;
            if (samplerId > -1)
            {
                auto& sampler = file.samplers[samplerId];
                materialInfo.settings.normal.minFilter = MinFromGL(static_cast<std::uint16_t>(sampler.minFilter));
                materialInfo.settings.normal.magFilter = MagFromGL(static_cast<std::uint16_t>(sampler.magFilter));
                materialInfo.settings.normal.wrapMode = WrapFromGL(static_cast<std::uint16_t>(sampler.wrapS));

                //Enable mipmapping if used for minification.
                if (materialInfo.settings.normal.minFilter == blurp::MinFilterType::MIPMAP_NEAREST || materialInfo.settings.normal.minFilter == blurp::MinFilterType::MIPMAP_LINEAR)
                {
                    materialInfo.settings.normal.generateMipMaps = true;
                }
                else
                {
                    materialInfo.settings.normal.generateMipMaps = false;
                }
            }
        }

        //Emissive Texture
        if (!material.emissiveTexture.empty())
        {
            materialInfo.mask.EnableAttribute(blurp::MaterialAttribute::EMISSIVE_TEXTURE);
            imagePtrs[2] = LoadTexture(file, material.emissiveTexture.index, a_Settings.path, 3);

            auto& p = imagePtrs[2];
            assert(p.channels == 3);

            auto samplerId = file.textures[material.emissiveTexture.index].sampler;
            if (samplerId > -1)
            {
                auto& sampler = file.samplers[samplerId];
                materialInfo.settings.emissive.minFilter = MinFromGL(static_cast<std::uint16_t>(sampler.minFilter));
                materialInfo.settings.emissive.magFilter = MagFromGL(static_cast<std::uint16_t>(sampler.magFilter));
                materialInfo.settings.emissive.wrapMode = WrapFromGL(static_cast<std::uint16_t>(sampler.wrapS));

                //Enable mipmapping if used for minification.
                if (materialInfo.settings.emissive.minFilter == blurp::MinFilterType::MIPMAP_NEAREST || materialInfo.settings.emissive.minFilter == blurp::MinFilterType::MIPMAP_LINEAR)
                {
                    materialInfo.settings.emissive.generateMipMaps = true;
                }
                else
                {
                    materialInfo.settings.emissive.generateMipMaps = false;
                }
            }
        }
        //Emissive constant
        else if(NotEmpty(material.emissiveFactor))
        {
            materialInfo.mask.EnableAttribute(blurp::MaterialAttribute::EMISSIVE_CONSTANT_VALUE);
            materialInfo.emissive.constant = { material.emissiveFactor[0], material.emissiveFactor[1], material.emissiveFactor[2] };
        }

        //MetalRoughness
        if(!material.pbrMetallicRoughness.metallicRoughnessTexture.empty())
        {
            materialInfo.mask.EnableAttribute(blurp::MaterialAttribute::METALLIC_TEXTURE);
            materialInfo.mask.EnableAttribute(blurp::MaterialAttribute::ROUGHNESS_TEXTURE);

            imagePtrs[3] = LoadTexture(file, material.pbrMetallicRoughness.metallicRoughnessTexture.index, a_Settings.path, 0);

            auto& p = imagePtrs[3];

            auto samplerId = file.textures[material.pbrMetallicRoughness.metallicRoughnessTexture.index].sampler;
            if (samplerId > -1)
            {
                auto& sampler = file.samplers[samplerId];
                materialInfo.settings.metalRoughAlpha.minFilter = MinFromGL(static_cast<std::uint16_t>(sampler.minFilter));
                materialInfo.settings.metalRoughAlpha.magFilter = MagFromGL(static_cast<std::uint16_t>(sampler.magFilter));
                materialInfo.settings.metalRoughAlpha.wrapMode = WrapFromGL(static_cast<std::uint16_t>(sampler.wrapS));

                //Enable mipmapping if used for minification.
                if (materialInfo.settings.metalRoughAlpha.minFilter == blurp::MinFilterType::MIPMAP_NEAREST || materialInfo.settings.metalRoughAlpha.minFilter == blurp::MinFilterType::MIPMAP_LINEAR)
                {
                    materialInfo.settings.metalRoughAlpha.generateMipMaps = true;
                }
                else
                {
                    materialInfo.settings.metalRoughAlpha.generateMipMaps = false;
                }
            }
        }
        else if (material.pbrMetallicRoughness.metallicFactor != 1 || material.pbrMetallicRoughness.roughnessFactor != 1)
        {
            materialInfo.mask.EnableAttribute(blurp::MaterialAttribute::METALLIC_CONSTANT_VALUE);
            materialInfo.mask.EnableAttribute(blurp::MaterialAttribute::ROUGHNESS_CONSTANT_VALUE);
            materialInfo.metallic.constant = material.pbrMetallicRoughness.metallicFactor;
            materialInfo.roughness.constant = material.pbrMetallicRoughness.roughnessFactor;
        }

        //Occlusion
        if(!material.occlusionTexture.empty())
        {
            imagePtrs[4] = LoadTexture(file, material.occlusionTexture.index, a_Settings.path, 0);

            auto samplerId = file.textures[material.occlusionTexture.index].sampler;
            if (samplerId > -1)
            {
                auto& sampler = file.samplers[samplerId];
                materialInfo.settings.ambientOcclusionHeight.minFilter = MinFromGL(static_cast<std::uint16_t>(sampler.minFilter));
                materialInfo.settings.ambientOcclusionHeight.magFilter = MagFromGL(static_cast<std::uint16_t>(sampler.magFilter));
                materialInfo.settings.ambientOcclusionHeight.wrapMode = WrapFromGL(static_cast<std::uint16_t>(sampler.wrapS));

                //Enable mipmapping if used for minification.
                if (materialInfo.settings.ambientOcclusionHeight.minFilter == blurp::MinFilterType::MIPMAP_NEAREST || materialInfo.settings.ambientOcclusionHeight.minFilter == blurp::MinFilterType::MIPMAP_LINEAR)
                {
                    materialInfo.settings.ambientOcclusionHeight.generateMipMaps = true;
                }
                else
                {
                    materialInfo.settings.ambientOcclusionHeight.generateMipMaps = false;
                }
            }
        }

        //Buffers containing all the required data.
        std::vector<std::uint8_t> diffuse;
        std::vector<std::uint8_t> emissive;
        std::vector<std::uint8_t> normal;
        std::vector<std::uint8_t> metal;
        std::vector<std::uint8_t> roughness;
        std::vector<std::uint8_t> alpha;
        std::vector<std::uint8_t> oh;

        //DIFFUSE
        if(imagePtrs[0].data != nullptr)
        {
            auto& data = imagePtrs[0];
            const int size = data.w * data.h * 3;
            const int srcSize = data.w * data.h * data.channels;
            diffuse.reserve(size);

            for (int i = 0; i < srcSize; i += data.channels)
            {
                diffuse.push_back(data.data[i + 0]);
                diffuse.push_back(data.data[i + 1]);
                diffuse.push_back(data.data[i + 2]);
            }

            materialInfo.diffuse.data = &diffuse[0];

            materialInfo.settings.diffuse.textureType = blurp::TextureType::TEXTURE_2D;
            materialInfo.settings.diffuse.dataType = blurp::DataType::UBYTE;
            materialInfo.settings.diffuse.pixelFormat = blurp::PixelFormat::RGB;
            materialInfo.settings.diffuse.dimensions = { data.w, data.h, 1 };
        }

        //Normals
        if (imagePtrs[1].data != nullptr)
        {
            auto& data = imagePtrs[1];
            const int size = data.w * data.h * 3;
            const int srcSize = data.w * data.h * data.channels;
            normal.reserve(size);

            for (int i = 0; i < srcSize; i += data.channels)
            {
                normal.push_back(data.data[i + 0]);
                normal.push_back(data.data[i + 1]);
                normal.push_back(data.data[i + 2]);
            }

            materialInfo.normal.data = &normal[0];
            materialInfo.settings.normal.textureType = blurp::TextureType::TEXTURE_2D;
            materialInfo.settings.normal.dataType = blurp::DataType::UBYTE;
            materialInfo.settings.normal.pixelFormat = blurp::PixelFormat::RGB;
            materialInfo.settings.normal.dimensions = { data.w, data.h, 1 };
        }

        //Emissive
        if (imagePtrs[2].data != nullptr)
        {
            auto& data = imagePtrs[2];
            const int size = data.w * data.h * 3;
            const int srcSize = data.w * data.h * data.channels;
            emissive.reserve(size);

            for (int i = 0; i < srcSize; i += data.channels)
            {
                emissive.push_back(data.data[i + 0]);
                emissive.push_back(data.data[i + 1]);
                emissive.push_back(data.data[i + 2]);
            }

            materialInfo.emissive.data = &emissive[0];
            materialInfo.settings.emissive.textureType = blurp::TextureType::TEXTURE_2D;
            materialInfo.settings.emissive.dataType = blurp::DataType::UBYTE;
            materialInfo.settings.emissive.pixelFormat = blurp::PixelFormat::RGB;
            materialInfo.settings.emissive.dimensions = { data.w, data.h, 1 };
        }

        //MRA
        if (imagePtrs[3].data != nullptr)
        {
            auto& data = imagePtrs[3];
            const int size = data.w * data.h;
            const int srcSize = data.w * data.h * data.channels;
            metal.reserve(size);
            roughness.reserve(size);

            for (int i = 0; i < srcSize; i += data.channels)
            {
                metal.push_back(data.data[i + 2]);
                roughness.push_back(data.data[i + 1]);
            }

            materialInfo.metallic.data = &metal[0];
            materialInfo.roughness.data = &roughness[0];
            materialInfo.settings.metalRoughAlpha.textureType = blurp::TextureType::TEXTURE_2D;
            materialInfo.settings.metalRoughAlpha.dataType = blurp::DataType::UBYTE;
            materialInfo.settings.metalRoughAlpha.pixelFormat = blurp::PixelFormat::RGB;
            materialInfo.settings.metalRoughAlpha.dimensions = { data.w, data.h, 1 };
        }

        //Add alpha component as well if present
        if (imagePtrs[0].data != nullptr && imagePtrs[0].channels == 4)
        {
            auto& data = imagePtrs[0];
            const int size = data.w * data.h;
            alpha.reserve(size);
            for (int i = 0; i < size; ++i)
            {
                alpha.push_back(data.data[data.channels * i + 3]);
            }

            if(imagePtrs[3].data != nullptr)
            {
                auto& p = imagePtrs[3];
                assert(p.w == data.w && p.h == data.h && "Metal Roughness and Alpha all need the same dimensions.");
            }

            materialInfo.alpha.data = &alpha[0];
            materialInfo.settings.metalRoughAlpha.textureType = blurp::TextureType::TEXTURE_2D;
            materialInfo.settings.metalRoughAlpha.dataType = blurp::DataType::UBYTE;
            materialInfo.settings.metalRoughAlpha.pixelFormat = blurp::PixelFormat::RGB;
            materialInfo.settings.metalRoughAlpha.dimensions = { data.w, data.h, 1 };
        }

        //Occlusion
        if (imagePtrs[4].data != nullptr)
        {
            auto& data = imagePtrs[4];
            const int size = data.w * data.h * 3;
            const int srcSize = data.w * data.h * data.channels;
            oh.reserve(size);

            for (int i = 0; i < srcSize; i += data.channels)
            {
                oh.push_back(data.data[i]);
                oh.push_back(0);
                oh.push_back(0);
            }

            materialInfo.ao.data = &oh[0];

            materialInfo.settings.ambientOcclusionHeight.textureType = blurp::TextureType::TEXTURE_2D;
            materialInfo.settings.ambientOcclusionHeight.dataType = blurp::DataType::UBYTE;
            materialInfo.settings.ambientOcclusionHeight.pixelFormat = blurp::PixelFormat::RGB;
            materialInfo.settings.ambientOcclusionHeight.dimensions = { data.w, data.h, 1 };
        }

        //Path to load the data from.
        materialInfo.path = a_Settings.path;

        //Create the material at the right index.
        bool saved = blurp::CreateMaterialFile(materialInfo, path, materialFileName, true); //Compress for size sake.
        assert(saved && "Could not export material for some reason.");

        auto blurpMat = blurp::LoadMaterial(a_ResourceManager, materialFileFullPath);
        materials.push_back(blurpMat);

        //Clean up STB.
        for (auto& img : imagePtrs)
        {
            if (img.data != nullptr)
            {
                stbi_image_free(img.data);
            }
        }

        std::cout << "Material compiled for GLTF file: " << materialFileName << std::endl;
    }

    for (size_t meshId = 0; meshId < file.meshes.size(); ++meshId)
    {
        /*
         * A mesh contains many primitives.
         * Primitives exist to separate materials or vertex limits.
         */
        const auto& mesh = file.meshes[meshId];

        //Remember which indices the drawables are stored at.
        std::vector<int> drawableIds;
        std::vector<int> transparenDrawableIds;

        for (size_t primitiveId = 0; primitiveId < mesh.primitives.size(); ++primitiveId)
        {
            /*
             * A primitive corresponds to a single draw call:
             * - The topology within a primitive is the same.
             * - One material is used per primitive.
             *
             * Each primitive is processed into a mesh, material and stored as a DrawData object.
             */
            const auto& primitive = mesh.primitives[primitiveId];

            blurp::DrawData drawData;
            blurp::MeshSettings blurpMesh;
            blurp::MaterialSettings blurpMaterial;
            std::shared_ptr<blurp::Mesh> compiledMesh;

            //These need to be here in scope or bad things happen.
            BufferInfo bufferInfo[4];
            BufferInfo indexBuffer;
            blurp::VertexAttribute attribs[4]{ blurp::VertexAttribute::POSITION_3D, blurp::VertexAttribute::NORMAL, blurp::VertexAttribute::TANGENT, blurp::VertexAttribute::UV_COORDS };
            std::vector<unsigned int> srcIndices;
            std::vector<glm::vec3> generatedTangents;
            std::vector<char> indices;

            //Buffer that will contain all vertex info.
            std::vector<float> data;
            size_t totalSize = 0;
            size_t totalStride = 0;

            const std::string meshFilePath = a_Settings.path + "meshes/";
            const std::string meshFileName = "mesh_" + std::to_string(meshId) + "_" + std::to_string(primitiveId);

            //If not recompiling meshes and the mesh file exists.
            if (a_BakeTransforms && !a_ForceRecompileMeshes && std::filesystem::exists((meshFilePath + meshFileName + ".blurpmesh")))
            {
                compiledMesh = blurp::LoadMeshFile(a_ResourceManager, meshFilePath + meshFileName);
            }
            else
            {

                for (auto const& attrib : primitive.attributes)
                {
                    if (attrib.first == "POSITION")
                    {
                        bufferInfo[0] = GLTFUtil::ReadBufferData(file, attrib.second);
                        totalSize += bufferInfo[0].totalSize;
                        totalStride += bufferInfo[0].dataSize;
                    }
                    else if (attrib.first == "NORMAL")
                    {
                        bufferInfo[1] = GLTFUtil::ReadBufferData(file, attrib.second);
                        totalSize += bufferInfo[1].totalSize;
                        totalStride += bufferInfo[1].dataSize;
                    }
                    else if (attrib.first == "TANGENT")
                    {
                        bufferInfo[2] = GLTFUtil::ReadBufferData(file, attrib.second);
                        totalSize += bufferInfo[2].totalSize;
                        totalStride += bufferInfo[2].dataSize;
                    }
                    else if (attrib.first == "TEXCOORD_0")
                    {
                        bufferInfo[3] = GLTFUtil::ReadBufferData(file, attrib.second);
                        totalSize += bufferInfo[3].totalSize;
                        totalStride += bufferInfo[3].dataSize;
                    }
                }

                //Generate indices if not given.
                if (primitive.indices < 0)
                {
                    assert(bufferInfo[0].numElements > 0 && "Positions are required to generate missing indices.");
                    for (int i = 0; i < bufferInfo[0].numElements; ++i)
                    {
                        srcIndices.push_back(i);
                    }

                    indexBuffer.dataSize = sizeof(unsigned int);
                    indexBuffer.numElements = srcIndices.size();
                    indexBuffer.totalSize = indexBuffer.numElements * indexBuffer.dataSize;
                    indexBuffer.data = reinterpret_cast<std::uint8_t*>(&srcIndices[0]);
                }
                else
                {
                    indexBuffer = GLTFUtil::ReadBufferData(file, primitive.indices);
                }

                //If the primitive uses a material with normal mapping enabled, and normals are provided but not tangents, calculate them.
                if (primitive.material >= 0 && !file.materials[primitive.material].normalTexture.empty() && bufferInfo[1].HasData() && !bufferInfo[2].HasData() && bufferInfo[3].HasData())
                {
                    if (primitive.mode != fx::gltf::Primitive::Mode::Triangles)
                    {
                        std::cout << "Warning: Loading a mesh with normal mapping without tangents. The mesh does not use a triangle list so tangents could not be calculated because I am lazy." << std::endl;
                    }

                    generatedTangents.resize(bufferInfo[0].numElements);
                    for (auto index = 0u; index < indexBuffer.numElements; index += 3)
                    {
                        unsigned int index1;
                        unsigned int index2;
                        unsigned int index3;
                        if (indexBuffer.dataSize == 2)
                        {
                            index1 = *indexBuffer.GetElement<unsigned short>(index);
                            index2 = *indexBuffer.GetElement<unsigned short>(index + 1);
                            index3 = *indexBuffer.GetElement<unsigned short>(index + 2);
                        }
                        else
                        {
                            index1 = *indexBuffer.GetElement<unsigned int>(index);
                            index2 = *indexBuffer.GetElement<unsigned int>(index + 1);
                            index3 = *indexBuffer.GetElement<unsigned int>(index + 2);
                        }


                        //Thanks to opengl-tutorial.com
                        const glm::vec3* v0 = bufferInfo[0].GetElement<glm::vec3>(index1);
                        const glm::vec3* v1 = bufferInfo[0].GetElement<glm::vec3>(index2);
                        const glm::vec3* v2 = bufferInfo[0].GetElement<glm::vec3>(index3);
                        const glm::vec2* uv0 = bufferInfo[3].GetElement<glm::vec2>(index1);
                        const glm::vec2* uv1 = bufferInfo[3].GetElement<glm::vec2>(index2);
                        const glm::vec2* uv2 = bufferInfo[3].GetElement<glm::vec2>(index3);

                        glm::vec3 deltaPos1 = *v1 - *v0;
                        glm::vec3 deltaPos2 = *v2 - *v0;

                        glm::vec2 deltaUV1 = *uv1 - *uv0;
                        glm::vec2 deltaUV2 = *uv2 - *uv0;
                        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
                        glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
                        generatedTangents[index1] = tangent;
                        generatedTangents[index2] = tangent;
                        generatedTangents[index3] = tangent;
                    }

                    bufferInfo[2].numElements = generatedTangents.size();
                    bufferInfo[2].dataSize = sizeof(glm::vec3);
                    bufferInfo[2].data = reinterpret_cast<const std::uint8_t*>(&generatedTangents[0]);
                    bufferInfo[2].totalSize = generatedTangents.size() * sizeof(glm::vec3);
                    bufferInfo[2].emptySpace = 0;
                    totalSize += bufferInfo[2].totalSize;
                    totalStride += bufferInfo[2].dataSize;
                }

                //Reserve enough memory. Divide by four because the total size is measured in bytes, and a float is four bytes.
                data.resize(totalSize / 4);

                //Note: Currently tangents are not calculated if absent.
                //If model makers are lazy and not including them for meshes that require them, implement generation.
                size_t offset = 0;

                for (int buffer = 0; buffer < 4; ++buffer)
                {
                    if (bufferInfo[buffer].HasData())
                    {
                        int floatStride = totalStride / 4;
                        int floatOffset = offset / 4;
                        for (int i = 0; i < bufferInfo[buffer].numElements; ++i)
                        {
                            auto element = bufferInfo[buffer].GetElement<float>(i);

                            //This loop interprets it as float array, so for vec3 it'll add 3 elements and for vec2 only two.
                            for (int q = 0; q < bufferInfo[buffer].dataSize / 4; ++q)
                            {
                                data[i * floatStride + floatOffset + q] = element[q];
                            }
                        }

                        blurpMesh.vertexSettings.EnableAttribute(attribs[buffer], offset, totalStride, 0);
                        offset += bufferInfo[buffer].dataSize;
                    }
                }

                assert(offset == totalStride && "Uhh this should always be the same??");

                indices.resize(indexBuffer.numElements * indexBuffer.dataSize);

                //Extract index buffer.
                if (indexBuffer.dataSize == 2)
                {
                    std::uint16_t* asShort = reinterpret_cast<std::uint16_t*>(&indices[0]);
                    for (std::uint32_t i = 0; i < indexBuffer.numElements; ++i)
                    {
                        auto element = indexBuffer.GetElement<std::uint16_t>(i);
                        asShort[i] = *element;
                    }
                }
                else
                {
                    std::uint32_t* asInt = reinterpret_cast<std::uint32_t*>(&indices[0]);

                    for (std::uint32_t i = 0; i < indexBuffer.numElements; ++i)
                    {
                        auto element = indexBuffer.GetElement<std::uint32_t>(i);
                        asInt[i] = *element;
                    }
                }

                //Reverse winding order.
                //for (auto it = indices.begin(); it != indices.end(); it += 3)
                //{
                //    std::swap(*it, *(it + 2));
                //}

                //Insert instance matrices if specified.
                if (a_Settings.numVertexInstances > 0)
                {
                    size_t matrixOffset = data.size() * sizeof(float);
                    float* start = reinterpret_cast<float*>(a_Settings.vertexInstances);
                    float* end = reinterpret_cast<float*>(reinterpret_cast<std::uintptr_t>(start) + (static_cast<size_t>(a_Settings.numVertexInstances) * 16));
                    data.insert(data.end(), start, end);

                    blurpMesh.vertexSettings.EnableAttribute(blurp::VertexAttribute::MATRIX, matrixOffset, 0, 1);
                    blurpMesh.instanceCount = a_Settings.numVertexInstances;
                }

                /*
                 * If set to true, go down the node hierarchy to find all instances of this mesh.
                 * Then chain the transforms along the way and return all transforms that ultimately affect this mesh.
                 */
                if (a_BakeTransforms)
                {
                    blurpMesh.vertexSettings.EnableAttribute(blurp::VertexAttribute::MATRIX, data.size() * sizeof(float), 16 * sizeof(float), 1);
                    std::vector<glm::mat4> transforms;
                    for (int sceneId = 0; sceneId < file.scenes.size(); ++sceneId)
                    {
                        auto& scene = file.scenes[sceneId];

                        for (auto& rootNodeId : scene.nodes)
                        {
                            auto& rootNode = file.nodes[rootNodeId];
                            glm::mat4 rootTransform = glm::make_mat4(&rootNode.matrix[0]);
                            FindTransforms(meshId, file, rootNodeId, rootTransform, transforms);
                        }

                        //Add the transforms to the vertex buffer.
                        for (auto& mat : transforms)
                        {
                            float* ptr = reinterpret_cast<float*>(&mat);

                            for (int i = 0; i < 16; ++i)
                            {
                                data.push_back(ptr[i]);
                            }
                        }
                    }
                    blurpMesh.instanceCount = transforms.size();
                }

                //Setup the rest of the blurpMesh object.
                blurpMesh.indexData = &indices[0];
                blurpMesh.indexDataType = indexBuffer.dataSize == 2 ? blurp::DataType::USHORT : blurp::DataType::UINT;
                blurpMesh.numIndices = indexBuffer.numElements;

                blurpMesh.vertexData = &data[0];
                blurpMesh.access = blurp::AccessMode::READ_ONLY;
                blurpMesh.usage = blurp::MemoryUsage::GPU;
                blurpMesh.vertexDataSizeBytes = data.size() * sizeof(float);

                //Compile into mesh on the GPU and then store a reference.
                compiledMesh = a_ResourceManager.CreateMesh(blurpMesh);

                std::cout << "Mesh compiled for gltf file: " << meshId << std::endl;
            }

            drawData.mesh = compiledMesh;

            //Save the mesh file if forced or not existing.
            if(a_BakeTransforms && (!std::filesystem::exists((meshFilePath + meshFileName + ".blurpmesh")) || a_ForceRecompileMeshes))
            {
                blurp::CreateMeshFile(blurpMesh, meshFilePath, meshFileName);
                std::cout << "Mesh saved to file: " << meshFileName << std::endl;
            }

            //Enable transformations through dynamic matrix.
            drawData.attributes.EnableAttribute(blurp::DrawAttribute::TRANSFORMATION_MATRIX);

            /*
             * Set the material to use and enable it.
             */
            if(primitive.material >= 0)
            {
                drawData.materialData.material = materials[primitive.material];
                drawData.attributes.EnableAttribute(blurp::DrawAttribute::MATERIAL_SINGLE);
            }

            //Set instance count to 0! Important because it defaults to 1.
            //When baking, the default of 1 is required.
            if(!a_BakeTransforms)
            {
                drawData.instanceCount = 0;
            }
            else
            {
                //Not really needed because it default to 1, but if that every changes this won't break.
                drawData.instanceCount = 1;
            }

            //Compile the pipeline state for this draw call.
            blurp::BlendData blending;
            blending.blend = false;

            blurp::DepthStencilData depthData;
            depthData.enableDepth = true;
            depthData.depthFunction = blurp::ComparisonFunction::COMPARISON_FUNC_LESS;
            depthData.enableStencil = false;
            depthData.depthWrite = true;

            blurp::TopologyType topology = ToBlurp(primitive.mode);
            blurp::CullMode culling = blurp::CullMode::CULL_BACK;
            blurp::WindingOrder winding = blurp::WindingOrder::COUNTER_CLOCKWISE;

            //Get the raw material data to extract rendering properties.
            if (primitive.material >= 0)
            {
                auto& material = file.materials[primitive.material];

                if(material.alphaMode == fx::gltf::Material::AlphaMode::Blend)
                {
                    blending.blend = true;
                    blending.blendOperation = blurp::BlendOperation::ADD;
                    blending.blendOperationAlpha = blurp::BlendOperation::ADD;
                    blending.srcBlend = blurp::BlendType::BLEND_SRC_ALPHA;
                    blending.dstBlend = blurp::BlendType::BLEND_INV_SRC_ALPHA;
                    blending.srcBlendAlpha = blurp::BlendType::BLEND_SRC_ALPHA;
                    blending.dstBlendAlpha = blurp::BlendType::BLEND_INV_SRC_ALPHA;
                }
            }

            blurp::PipelineState pState = blurp::PipelineState::Compile(blending, topology, culling, winding, depthData);

            //Add data to the right set.
            if(blending.blend)
            {
                output.transparentDrawDatas.push_back(drawData);
                transparenDrawableIds.push_back(output.transparentDrawDatas.size() - 1);
                output.transparentPipelineStates.push_back(pState);
            }
            else
            {
                output.drawDatas.push_back(drawData);
                drawableIds.push_back(output.drawDatas.size() - 1);
                output.pipelineStates.push_back(pState);
            }

            std::cout << "Mesh loaded with ID: " << meshId << std::endl;
        }

        //Add the indices of the primitives for this mesh.
        output.meshes.push_back(GLTFMesh{ drawableIds, transparenDrawableIds});
    }

    //Iterate over the scene nodes in the scenes to add the correct transformations and instance count to each drawable.
    //Only do this if baking is disabled, because when baked the hierarchy is already part of each mesh.
    if (!a_BakeTransforms)
    {
        for (int sceneId = 0; sceneId < file.scenes.size(); ++sceneId)
        {
            auto& scene = file.scenes[sceneId];

            for (auto& rootNodeId : scene.nodes)
            {
                auto& rootNode = file.nodes[rootNodeId];
                glm::mat4 rootTransform = glm::make_mat4(&rootNode.matrix[0]);
                ResolveNode(output, file, rootNodeId, rootTransform);
            }
        }
    }

    //Link the correct pipeline state to each draw data object (because resizing that vector causes the memory addresses to change).
    for(int i = 0; i < output.drawDatas.size(); ++i)
    {
        output.drawDatas[i].pipelineState = &output.pipelineStates[i];
    }
    for(int i = 0; i < output.transparentDrawDatas.size(); ++i)
    {
        output.transparentDrawDatas[i].pipelineState = &output.transparentPipelineStates[i];
    }

    return output;
}

void ResolveNode(GLTFScene& a_Scene, fx::gltf::Document& a_File, int a_NodeIndex, glm::mat4 a_ParentTransform)
{
    auto& node = a_File.nodes[a_NodeIndex];
    glm::mat4 transform = glm::make_mat4(&node.matrix[0]);

    if (transform == glm::identity<glm::mat4>())
    {
        glm::vec3 translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
        glm::quat rotation = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
        glm::vec3 scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);

        blurp::Transform t;
        t.Translate(translation);
        t.Rotate(rotation);
        t.Scale(scale);
        transform = t.GetTransformation();
    }

    const glm::mat4 chainedTransform = a_ParentTransform * transform;

    //Add mesh to scene if mesh is attached to this node.
    if(node.mesh > -1)
    {
        GLTFMesh& mesh = a_Scene.meshes[node.mesh];
        for (auto id : mesh.drawableIds)
        {
            auto& drawable = a_Scene.drawDatas[id];
            //Add one more instance.
            ++drawable.instanceCount;
        }
        for (auto id : mesh.transparentDrawableIds)
        {
            auto& drawable = a_Scene.transparentDrawDatas[id];
            //Add one more instance.
            ++drawable.instanceCount;
        }
        mesh.transforms.push_back(chainedTransform);
    }

    //If there is child meshes, recursively call.
    if(!node.children.empty())
    {
        for(auto& id : node.children)
        {
            ResolveNode(a_Scene, a_File, id, chainedTransform);
        }
    }
}

void FindTransforms(int a_MeshIndex, fx::gltf::Document& a_File, int a_NodeIndex,
    glm::mat4 a_ParentTransform, std::vector<glm::mat4>& a_Output)
{
    assert(a_MeshIndex >= 0);
    assert(a_NodeIndex >= 0);

    constexpr static auto identity = glm::identity<glm::mat4>();
    auto& node = a_File.nodes[a_NodeIndex];
    glm::mat4 transform = glm::make_mat4(&node.matrix[0]);

    if(transform == identity)
    {
        glm::vec3 translation = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
        glm::quat rotation = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
        glm::vec3 scale = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);

        blurp::Transform t;
        t.Translate(translation);
        t.Rotate(rotation);
        t.Scale(scale);
        transform = t.GetTransformation();
    }

    const glm::mat4 chainedTransform = a_ParentTransform * transform;

    //Add mesh to scene if mesh is attached to this node.
    if (node.mesh == a_MeshIndex)
    {
        a_Output.push_back(chainedTransform);
    }

    //If there is child meshes, recursively call.
    if (!node.children.empty())
    {
        for (auto& id : node.children)
        {
            FindTransforms(a_MeshIndex, a_File, id, chainedTransform, a_Output);
        }
    }
}

LoadedImageInformation LoadTexture(fx::gltf::Document& a_File, int a_TextureId, const std::string& a_Path, int numChannels)
{
    assert(a_TextureId >= 0);

    std::uint8_t* imgData = nullptr;
    int w = 0;
    int h = 0;
    int channels = 0;


    ImageData data(a_File, a_TextureId, a_Path);
    auto info = data.Info();

    stbi_set_flip_vertically_on_load(false);
    if (info.IsBinary())
    {
        //Load from raw
        imgData = stbi_load_from_memory(info.BinaryData, info.BinarySize, &w, &h, &channels, numChannels);
    }
    else
    {
        //Load from file.
        imgData = stbi_load(info.FileName.c_str(), &w, &h, &channels, numChannels);
    }
    assert(imgData != nullptr && "Could not load and decode image for some reason.");

    //If a specific number of channels was requested, overwrite the "would have been" channels count.
    if (numChannels != 0)
    {
        channels = numChannels;
    }
    return LoadedImageInformation{ imgData, w, h, channels };
}

blurp::WrapMode WrapFromGL(int glEnum)
{
    switch (glEnum)
    {
    case 33071:
        return blurp::WrapMode::CLAMP_TO_EDGE;
    case 33648:
        return blurp::WrapMode::MIRRORED_REPEAT;
    case 10497:
        return blurp::WrapMode::REPEAT;
    default:
        return blurp::WrapMode::REPEAT;
    }
}

blurp::MagFilterType MagFromGL(int glEnum)
{
    switch (glEnum)
    {
    case 9728:
        return blurp::MagFilterType::NEAREST;
    case 9729:
        return blurp::MagFilterType::LINEAR;
    default:
        return blurp::MagFilterType::NEAREST;
    }
}

blurp::MinFilterType MinFromGL(int glEnum)
{
    switch (glEnum)
    {
    case 9728:
        return blurp::MinFilterType::NEAREST;
    case 9729:
        return blurp::MinFilterType::LINEAR;
    case 9984:
        return blurp::MinFilterType::MIPMAP_NEAREST;
    case 9985:
        return blurp::MinFilterType::MIPMAP_NEAREST;
    case 9986:
        return blurp::MinFilterType::MIPMAP_NEAREST;
    case 9987:
        return blurp::MinFilterType::MIPMAP_LINEAR;
    default:
        return blurp::MinFilterType::LINEAR;
    }
}

blurp::TopologyType ToBlurp(fx::gltf::Primitive::Mode a_Mode)
{
    switch (a_Mode)
    {
    case fx::gltf::Primitive::Mode::Points:
        return blurp::TopologyType::POINTS;

    case fx::gltf::Primitive::Mode::Lines:
        return blurp::TopologyType::LINES;

    case fx::gltf::Primitive::Mode::Triangles:
        return blurp::TopologyType::TRIANGLES;

    case fx::gltf::Primitive::Mode::LineStrip:
        return blurp::TopologyType::LINE_STRIP;

    case fx::gltf::Primitive::Mode::TriangleStrip:
        return blurp::TopologyType::TRIANGLE_STRIP;

    default:
        assert(0 && "Unsupported topology type.");
        return blurp::TopologyType::TRIANGLES;
    }
}
