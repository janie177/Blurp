#include "MeshFile.h"
#include "Data.h"
#include "lz4.h"
#include "lz4hc.h"
#include "Settings.h"
#include "RenderResourceManager.h"
#include <iostream>
#include <fstream>
#include <filesystem>

namespace blurp
{
    bool CreateMeshFile(const MeshSettings& a_MeshSettings, const std::string& a_Path, const std::string& a_FileName)
    {
        std::vector<char> data;
        data.resize(sizeof(MeshFileHeader));

        //Buffer containing the raw data without compression.
        std::vector<char> uncompressed;

        const size_t indicesStartPos = 0;
        char* start = (char*)a_MeshSettings.indexData;
        char* end =  (char*)a_MeshSettings.indexData + static_cast<size_t>(a_MeshSettings.numIndices * SizeOf(a_MeshSettings.indexDataType));
        uncompressed.insert(uncompressed.end(), start, end);

        const size_t verticesStartPos = uncompressed.size();
        start = (char*)a_MeshSettings.vertexData;
        end = (char*)a_MeshSettings.vertexData + static_cast<size_t>(a_MeshSettings.vertexDataSizeBytes);
        uncompressed.insert(uncompressed.end(), start, end);

        //Create a mesh file header and fill in the data.
        MeshFileHeader header;
        header.settings = a_MeshSettings;
        header.settings.vertexData = reinterpret_cast<void*>(verticesStartPos);
        header.settings.indexData = reinterpret_cast<void*>(indicesStartPos);
        header.version = 1;

        /*
         * Compression using LZ4.
         */
        const int src_size = static_cast<int>(uncompressed.size());
        const int max_dst_size = LZ4_compressBound(src_size);
        char* compressed_data = static_cast<char*>(malloc(static_cast<size_t>(max_dst_size)));
        if (compressed_data == NULL)
        {
            throw std::exception("Could not allocate compression memory!");
        }
        const int compressed_data_size = LZ4_compress_HC(&uncompressed[0], compressed_data, src_size, max_dst_size, LZ4HC_CLEVEL_MAX);
        if (compressed_data_size <= 0)
        {
            throw std::exception("A 0 or negative result from LZ4_compress_default() indicates a failure trying to compress the data. ");
        }

        header.uncompressedSize = src_size;
        header.compressedSize = compressed_data_size;
        *reinterpret_cast<MeshFileHeader*>(&data[0]) = header;

        //Append compressed data.
        data.insert(data.end(), compressed_data, compressed_data + compressed_data_size);

        //Create the path if not exist.
        std::filesystem::create_directories(a_Path);

        //Write to file.
        std::string finalName = a_Path + a_FileName + MESH_FILE_EXTENSION;
        std::ofstream file(finalName, std::ios::out | std::ios::binary);

        if(!file.write(&data[0], data.size()))
        {
            std::cout << "Could not save mesh file." << std::endl;
            return false;
        }
        file.close();

        //Free memory
        free(compressed_data);
   
        return true;
    }

    std::shared_ptr<Mesh> LoadMeshFile(RenderResourceManager& a_ResourceManager, const std::string& a_FileName)
    {
        std::string fileName = a_FileName + MESH_FILE_EXTENSION;
        std::ifstream file(fileName, std::ios::in | std::ios::binary);
        std::vector<char> data;

        if (!file.eof() && !file.fail())
        {
            file.seekg(0, std::ios_base::end);
            auto fileSize = file.tellg();
            data.resize(fileSize);

            file.seekg(0, std::ios_base::beg);
            file.read(&data[0], fileSize);
        }
        else
        {
            throw std::exception("Could not load material file!");
        }

        MeshFileHeader* header = reinterpret_cast<MeshFileHeader*>(&data[0]);

        char* const regen_buffer = static_cast<char*>(malloc(header->uncompressedSize));
        if (regen_buffer == NULL)
        {
            throw std::exception("Failed to allocate memory for *regen_buffer.");
        }

        char* originStart = reinterpret_cast<char*>(&data[0]) + sizeof(MeshFileHeader);

        const int decompressed_size = LZ4_decompress_safe(originStart, regen_buffer, static_cast<int>(header->compressedSize), static_cast<int>(header->uncompressedSize));

        if (decompressed_size < 0)
        {
            throw std::exception("A negative result from LZ4_decompress_safe indicates a failure trying to decompress the data.  See exit code (echo $?) for value returned.");
        }

        if (decompressed_size != header->uncompressedSize)
        {
            throw std::exception("Decompressed data is different from original!");
        }

        MeshSettings settings = header->settings;
        settings.vertexData = (void*)(reinterpret_cast<size_t>(regen_buffer) + reinterpret_cast<size_t>(header->settings.vertexData));
        settings.indexData = (void*)(reinterpret_cast<size_t>(regen_buffer) + reinterpret_cast<size_t>(header->settings.indexData));

        auto mesh = a_ResourceManager.CreateMesh(settings);

        //Free after creating from data.
        free(regen_buffer);

        return mesh;
    }
}
