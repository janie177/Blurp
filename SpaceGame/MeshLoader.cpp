#include "MeshLoader.h"
#include <stb_image.h>
#include <MaterialFile.h>
#include <filesystem>

bool hasEnding(std::string const& fullString, std::string const& ending)
{
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending));
    }
    else {
        return false;
    }
}

GLTFScene LoadMesh(const MeshLoaderSettings& a_Settings, blurp::RenderResourceManager& a_ResourceManager, bool a_RecompileMaterials)
{
    fx::gltf::Document file;
    GLTFScene output;

    std::string fileNameLowerCase = a_Settings.fileName;
    std::for_each(fileNameLowerCase.begin(), fileNameLowerCase.end(), [](char& c) {
        c = ::tolower(c);
    });

    if (hasEnding(fileNameLowerCase, ".gltf"))
    {
        try
        {
            file = fx::gltf::LoadFromText((a_Settings.path + a_Settings.fileName));
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
            file = fx::gltf::LoadFromBinary((a_Settings.path + a_Settings.fileName));
        }
        catch (std::exception e)
        {
            std::cout << e.what() << std::endl;
            return output;
        }
    }

    //TODO:
    //- Sponza won't load because of a fx-gltf error. I think it's the mesh being crooked but gotta check just in case.
    //  The Sponza mesh from the GLTF 2 repo does work according to fx-gltf, but I can't download that one rn for some reason.

    //Keep track of the materials that are reused.
    std::vector<std::shared_ptr<blurp::Material>> materials;

    for(auto& material : file.materials)
    {
        const std::string materialFileName = a_Settings.fileName + "_Material_" + std::to_string(materials.size());
        const std::string materialFileFullPath = a_Settings.path + materialFileName;

        //If the file already exits, load it and continue. This prevents regenerating every time.
        if(!a_RecompileMaterials && std::filesystem::exists(materialFileFullPath))
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
            materialInfo.mask.EnableAttribute(blurp::MaterialAttribute::ALPHA_TEXTURE);
            imagePtrs[0] = LoadTexture(file, material.pbrMetallicRoughness.baseColorTexture.index, a_Settings.path);

            auto samplerId = file.textures[material.pbrMetallicRoughness.baseColorTexture.index].sampler;
            if (samplerId > -1)
            {
                auto& sampler = file.samplers[samplerId];
                materialInfo.settings.diffuse.minFilter = MinFromGL(static_cast<std::uint16_t>(sampler.minFilter));
                materialInfo.settings.diffuse.magFilter = MagFromGL(static_cast<std::uint16_t>(sampler.magFilter));
                materialInfo.settings.diffuse.wrapMode = WrapFromGL(static_cast<std::uint16_t>(sampler.wrapS));
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
            imagePtrs[1] = LoadTexture(file, material.normalTexture.index, a_Settings.path);

            auto samplerId = file.textures[material.normalTexture.index].sampler;
            if (samplerId > -1)
            {
                auto& sampler = file.samplers[samplerId];
                materialInfo.settings.normal.minFilter = MinFromGL(static_cast<std::uint16_t>(sampler.minFilter));
                materialInfo.settings.normal.magFilter = MagFromGL(static_cast<std::uint16_t>(sampler.magFilter));
                materialInfo.settings.normal.wrapMode = WrapFromGL(static_cast<std::uint16_t>(sampler.wrapS));
            }
        }

        //Emissive Texture
        if (!material.emissiveTexture.empty())
        {
            materialInfo.mask.EnableAttribute(blurp::MaterialAttribute::EMISSIVE_TEXTURE);
            imagePtrs[2] = LoadTexture(file, material.emissiveTexture.index, a_Settings.path);

            auto samplerId = file.textures[material.emissiveTexture.index].sampler;
            if (samplerId > -1)
            {
                auto& sampler = file.samplers[samplerId];
                materialInfo.settings.emissive.minFilter = MinFromGL(static_cast<std::uint16_t>(sampler.minFilter));
                materialInfo.settings.emissive.magFilter = MagFromGL(static_cast<std::uint16_t>(sampler.magFilter));
                materialInfo.settings.emissive.wrapMode = WrapFromGL(static_cast<std::uint16_t>(sampler.wrapS));
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
            imagePtrs[3] = LoadTexture(file, material.pbrMetallicRoughness.metallicRoughnessTexture.index, a_Settings.path);

            auto samplerId = file.textures[material.pbrMetallicRoughness.metallicRoughnessTexture.index].sampler;
            if (samplerId > -1)
            {
                auto& sampler = file.samplers[samplerId];
                materialInfo.settings.metalRoughAlpha.minFilter = MinFromGL(static_cast<std::uint16_t>(sampler.minFilter));
                materialInfo.settings.metalRoughAlpha.magFilter = MagFromGL(static_cast<std::uint16_t>(sampler.magFilter));
                materialInfo.settings.metalRoughAlpha.wrapMode = WrapFromGL(static_cast<std::uint16_t>(sampler.wrapS));
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
            imagePtrs[4] = LoadTexture(file, material.pbrMetallicRoughness.metallicRoughnessTexture.index, a_Settings.path);

            auto samplerId = file.textures[material.occlusionTexture.index].sampler;
            if (samplerId > -1)
            {
                auto& sampler = file.samplers[samplerId];
                materialInfo.settings.ambientOcclusionHeight.minFilter = MinFromGL(static_cast<std::uint16_t>(sampler.minFilter));
                materialInfo.settings.ambientOcclusionHeight.magFilter = MagFromGL(static_cast<std::uint16_t>(sampler.magFilter));
                materialInfo.settings.ambientOcclusionHeight.wrapMode = WrapFromGL(static_cast<std::uint16_t>(sampler.wrapS));
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
            alpha.reserve(size);

            for (int i = 0; i < srcSize; i += data.channels)
            {
                metal.push_back(data.data[i + 0]);
                roughness.push_back(data.data[i + 1]);
                alpha.push_back(0);
            }

            //Add alpha component as well if present
            if(imagePtrs[0].data != nullptr)
            {
                for(int i = 0; i < data.w * data.h; ++i)
                {
                    auto& alphaData = imagePtrs[0];
                    alpha[i] = alphaData.data[alphaData.channels * i + 3];
                }
                materialInfo.alpha.data = &alpha[0];
            }

            materialInfo.metallic.data = &metal[0];
            materialInfo.roughness.data = &roughness[0];

            materialInfo.settings.metalRoughAlpha.textureType = blurp::TextureType::TEXTURE_2D;
            materialInfo.settings.metalRoughAlpha.dataType = blurp::DataType::UBYTE;
            materialInfo.settings.metalRoughAlpha.pixelFormat = blurp::PixelFormat::RGB;
            materialInfo.settings.metalRoughAlpha.dimensions = { data.w, data.h, 1 };
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
            materialInfo.settings.ambientOcclusionHeight.dimensions = { data.w, data.h, 1 };
        }

        //Clean up STB.
        for(auto& img : imagePtrs)
        {
            if(img.data != nullptr)
            {
                stbi_image_free(img.data);
            }
        }

        //Create the material at the right index.
        bool saved = blurp::CreateMaterialFile(materialInfo, a_Settings.path, materialFileName);

        assert(saved && "Could not export material for some reason.");

        auto blurpMat = blurp::LoadMaterial(a_ResourceManager, materialFileFullPath);
        materials.push_back(blurpMat);
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
        int startId = output.drawDatas.size();

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

            assert(primitive.mode == fx::gltf::Primitive::Mode::Triangles && "Triangle list is the only supported format now.");

            BufferInfo bufferInfo[4];
            blurp::VertexAttribute attribs[4]{ blurp::VertexAttribute::POSITION_3D, blurp::VertexAttribute::NORMAL, blurp::VertexAttribute::TANGENT, blurp::VertexAttribute::UV_COORDS };

            //Buffer that will contain all vertex info.
            std::vector<float> data;
            size_t totalSize = 0;
            size_t totalStride = 0;

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

            //Reserve enough memory. Divide by four because the total size is measured in bytes, and a float is four bytes.
            data.resize(totalSize / 4);

            BufferInfo indexBuffer;

            //Generate indices if not given.
            std::vector<int> srcIndices;
            if (primitive.indices < 0)
            {
                assert(bufferInfo[0].numElements > 0 && "Positions are required to generate missing indices.");
                for(int i = 0; i < bufferInfo[0].numElements; ++i)
                {
                    srcIndices.push_back(i);
                }

                indexBuffer.dataSize = sizeof(int);
                indexBuffer.numElements = srcIndices.size();
                indexBuffer.totalSize = indexBuffer.numElements * indexBuffer.dataSize;
                indexBuffer.data = reinterpret_cast<std::uint8_t*>(&srcIndices[0]);
            }
            else
            {
                indexBuffer = GLTFUtil::ReadBufferData(file, primitive.indices);
            }


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

            std::vector<char> indices;
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
                for (std::uint32_t i = 0; i < indexBuffer.numElements; ++i)
                {
                    std::uint32_t* asInt = reinterpret_cast<std::uint32_t*>(&indices[0]);

                    for (std::uint32_t i = 0; i < indexBuffer.numElements; ++i)
                    {
                        auto element = indexBuffer.GetElement<std::uint32_t>(i);
                        asInt[i] = *element;
                    }
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

            //Setup the rest of the blurpMesh object.
            blurpMesh.indexData = &indices[0];
            blurpMesh.indexDataType = indexBuffer.dataSize == 2 ? blurp::DataType::USHORT : blurp::DataType::UINT;
            blurpMesh.numIndices = indexBuffer.numElements;

            blurpMesh.vertexData = &data[0];
            blurpMesh.access = blurp::AccessMode::READ_ONLY;
            blurpMesh.usage = blurp::MemoryUsage::GPU;
            blurpMesh.vertexDataSizeBytes = data.size() * sizeof(float);

            //Compile into mesh on the GPU and then store a reference.
            drawData.mesh = a_ResourceManager.CreateMesh(blurpMesh);

            //Enable transformations through dynamic matrix.
            drawData.attributes.EnableAttribute(blurp::DrawAttribute::TRANSFORMATION_MATRIX);

            /*
             * Load material for this draw data object.
             */
            if(primitive.material > 0)
            {
                drawData.materialData.material = materials[primitive.material];
                drawData.attributes.EnableAttribute(blurp::DrawAttribute::MATERIAL_SINGLE);
            }


            //Add to set.
            output.drawDatas.push_back(drawData);
            drawableIds.push_back(startId + primitiveId);
        }

        //Add the indices of the primitives for this mesh.
        output.meshes.push_back(GLTFMesh{ drawableIds });
    }

    //Iterate over the scene nodes in the scenes to add the correct transformations and instance count to each drawable.
    for(int sceneId = 0; sceneId < file.scenes.size(); ++sceneId)
    {
        auto& scene = file.scenes[sceneId];

        for(auto& rootNodeId : scene.nodes)
        {
            auto& rootNode = file.nodes[rootNodeId];
            glm::mat4 rootTransform = glm::make_mat4(&rootNode.matrix[0]);
            ResolveNode(output, file, rootNodeId, rootTransform);
        }
    }

    return output;
}

void ResolveNode(GLTFScene& a_Scene, fx::gltf::Document& a_File, int a_NodeIndex, glm::mat4 a_ParentTransform)
{
    auto& node = a_File.nodes[a_NodeIndex];
    const glm::mat4 transform = glm::make_mat4(&node.matrix[0]);
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

LoadedImageInformation LoadTexture(fx::gltf::Document& a_File, int a_TextureId, const std::string& a_Path)
{
    int imageId = a_File.textures[a_TextureId].source;
    auto& image = a_File.images[imageId];

    std::uint8_t* imgData = nullptr;
    int w = 0;
    int h = 0;
    int channels = 0;


    ImageData data(a_File, a_TextureId, a_Path);
    auto info = data.Info();
    if (info.IsBinary())
    {
        //Load from raw
        imgData = stbi_load_from_memory(info.BinaryData, info.BinarySize, &w, &h, &channels, 0);
    }
    else
    {
        //Load from file.
        imgData = stbi_load(info.FileName.c_str(), &w, &h, &channels, 0);
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
