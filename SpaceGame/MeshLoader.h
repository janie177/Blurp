#pragma once
#include <string>
#include <RenderResourceManager.h>
#include <Data.h>
#include <fx/gltf.h>
#include "GLTFUtil.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

/*
 * A GLTF mesh can contain multiple drawable objects.
 * Multiple DrawData can thus exist for a single mesh due to vertex limits and multiple materials.
 */
struct GLTFMesh
{
    //Vector containing the IDs of all drawables for this mesh.
    std::vector<int> drawableIds;

    //The transforms that apply to each primitive in this mesh.
    std::vector<glm::mat4> transforms;
};

/*
 * A GLTFScene contains all drawable meshes.
 */
struct GLTFScene
{
    std::vector<blurp::DrawData> drawDatas;
    std::vector<GLTFMesh> meshes;
};


struct MeshLoaderSettings
{
    //The GLTF file to load.
    std::string fileName;

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
 */
GLTFScene LoadMesh(const MeshLoaderSettings& a_Settings, blurp::RenderResourceManager& a_ResourceManager);

//Interally resolve a GLTF node.
void ResolveNode(GLTFScene& a_Scene, fx::gltf::Document& a_File, int a_NodeIndex, glm::mat4 a_ParentTransform);