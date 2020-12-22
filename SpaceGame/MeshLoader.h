#pragma once
#include <string>
#include <RenderResourceManager.h>
#include <Data.h>
#include <fx/gltf.h>
#include "GLTFUtil.h"

struct MeshLoaderSettings
{
    //The GLTF file to load.
    std::string fileName;

    //If larger than 0, multiple transforms will be inserted in the vertex buffer for more instance drawing.
    std::uint32_t numVertexInstances;

    //Pointer to the actual instances if numVertexInstances is larger than 0.
    glm::mat4* vertexInstances;

};

/*
 * Load a mesh from a GLTF file from the given file path + name.
 * This parses the GLTF, creates the mesh and sets up the draw data object.
 * Materials in the GLTF model are also parsed and included in the draw data for simplicity.
 *
 * If multiple meshes are inside the model, then multiple DrawData objects are created.
 */
std::vector<blurp::DrawData> LoadMesh(const MeshLoaderSettings& a_Settings, blurp::RenderResourceManager& a_ResourceManager)
{
    std::vector<blurp::DrawData> output;

    auto file = fx::gltf::LoadFromText((a_Settings.fileName));

    for(size_t meshId = 0; meshId < file.meshes.size(); ++meshId)
    {
        /*
         * A mesh contains many primitives.
         * Primitives exist to separate materials or vertex limits.
         */
        const auto& mesh = file.meshes[meshId];

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
            assert(primitive.indices >= 0 && "Only index buffer meshes are supported currently.");

            BufferInfo indexBuffer = GLTFUtil::ReadBufferData(file, primitive.indices);
            BufferInfo bufferInfo[4];
            blurp::VertexAttribute attribs[4]{blurp::VertexAttribute::POSITION_3D, blurp::VertexAttribute::NORMAL, blurp::VertexAttribute::TANGENT, blurp::VertexAttribute::UV_COORDS};

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

            //Note: Currently tangents are not calculated if absent.
            //If model makers are lazy and not including them for meshes that require them, implement generation.
            size_t offset = 0;

            for(int buffer = 0; buffer < 4; ++buffer)
            {
                if (bufferInfo[buffer].HasData())
                {
                    int floatStride = totalStride / 4;
                    int floatOffset = offset / 4;
                    for (int i = 0; i < bufferInfo[buffer].numElements; ++i)
                    {
                        auto element = bufferInfo[buffer].GetElement<float>(i);

                        //This loop interprets it as float array, so for vec3 it'll add 3 elements and for vec2 only two.
                        for (int q = 0; q < bufferInfo[buffer].dataSize / 4; ++ q)
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
            if(indexBuffer.dataSize == 2)
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
            if(a_Settings.numVertexInstances > 0)
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


            //TODO enable material and dynamic attributes.


            //Add to set.
            output.push_back(drawData);
        }
    }

    return output;
}
