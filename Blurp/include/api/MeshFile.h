#pragma once
#include "Settings.h"

#define MESH_FILE_EXTENSION ".blurpmesh"

namespace blurp
{
    class RenderResourceManager;

    struct MeshFileHeader
    {
        MeshFileHeader() : version(1), compressedSize(0), uncompressedSize(0){}

        //File version number for backwards compatibility.
        std::uint16_t version;

        //Compression information.
        long long int compressedSize;
        long long int uncompressedSize;

        //The meshes actual settings. The pointers are replaced with offsets into the data buffer.
        MeshSettings settings;
    };

    /*
     * Create a mesh file using the provided mesh information.
     */
    bool CreateMeshFile(const MeshSettings& a_MeshSettings, const std::string& a_Path, const std::string& a_FileName);

    /*
     * Load an existing mesh file into memory.
     */
    std::shared_ptr<Mesh> LoadMeshFile(RenderResourceManager& a_ResourceManager, const std::string& a_FileName);
}
