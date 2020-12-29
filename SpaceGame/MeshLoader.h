#pragma once
#include <string>
#include <RenderResourceManager.h>
#include <Data.h>
#include <fx/gltf.h>
#include "GLTFUtil.h"
#include <glm/gtc/type_ptr.hpp>

/*
 * A GLTF mesh can contain multiple drawable objects.
 * Multiple DrawData can thus exist for a single mesh due to vertex limits and multiple materials.
 */
struct GLTFMesh
{
    //Vector containing the IDs of all drawables for this mesh.
    std::vector<int> drawableIds;
    std::vector<int> transparentDrawableIds;

    //The transforms that apply to each primitive in this mesh.
    std::vector<glm::mat4> transforms;
};

/*
 * A GLTFScene contains all drawable meshes.
 */
struct GLTFScene
{
    std::vector<blurp::DrawData> drawDatas;
    std::vector<blurp::DrawData> transparentDrawDatas;
    std::vector<blurp::PipelineState> transparentPipelineStates;
    std::vector<blurp::PipelineState> pipelineStates;
    std::vector<GLTFMesh> meshes;
};


struct MeshLoaderSettings
{
    //The GLTF file to load.
    std::string fileName;

    //The folder where the mesh file is located.
    std::string path;

    //If larger than 0, multiple transforms will be inserted in the vertex buffer for more instance drawing.
    std::uint32_t numVertexInstances;

    //Pointer to the actual instances if numVertexInstances is larger than 0.
    glm::mat4* vertexInstances;

};

bool hasEnding(std::string const& fullString, std::string const& ending);

/*
 * Load a mesh from a GLTF file from the given file path + name.
 * This parses the GLTF, creates the mesh and sets up the draw data object.
 * Materials in the GLTF model are also parsed and included in the draw data for simplicity.
 *
 * If multiple meshes are inside the model, then multiple DrawData objects are created.
 *
 * If a_BakeTransforms is true, all nodes in the GLTF scene using a specific mesh will have their transfrom chain embedded with the mesh on the GPU.
 * Note that this means that the instance count for each draw call will be 1, and that a root transform has to be provided for each draw call.
 *
 * If a_ForceRecompimeMeshes is true, all meshes are recompiled and stored.
 * Keep in mind that this only works when a_BakeTransforms is set to true.
 */
GLTFScene LoadMesh(const MeshLoaderSettings& a_Settings, blurp::RenderResourceManager& a_ResourceManager, bool a_BakeTransforms, bool a_ForceRecompileMaterials, bool a_ForceRecompileMeshes);

//Interally resolve a GLTF node.
void ResolveNode(GLTFScene& a_Scene, fx::gltf::Document& a_File, int a_NodeIndex, glm::mat4 a_ParentTransform);

/*
 * Find all transforms affecting a mesh instance and store them in the output vector.
 * This chains together according to the node hierarchy.
 */
void FindTransforms(int a_MeshIndex, fx::gltf::Document& a_File, int a_NodeIndex, glm::mat4 a_ParentTransform, std::vector<glm::mat4>& a_Output);



struct LoadedImageInformation
{
    std::uint8_t* data = nullptr;
    int w = 0;
    int h = 0;
    int channels = 0;
};

//Load texture data from the GLTF file.
LoadedImageInformation LoadTexture(fx::gltf::Document& a_File, int a_TextureId, const std::string& a_Path, int numChannels);

/*
 * Returns true if the array contains non-zero elements.
 */
template<typename T, size_t N>
bool NotEmpty(std::array<T, N> a_Array)
{
    for(int i = 0; i < N; ++i)
    {
        if (a_Array[i] != 0) return true;
    }
    return false;
}

blurp::WrapMode WrapFromGL(int glEnum);

blurp::MagFilterType MagFromGL(int glEnum);

blurp::MinFilterType MinFromGL(int glEnum);

blurp::TopologyType ToBlurp(fx::gltf::Primitive::Mode a_Mode);