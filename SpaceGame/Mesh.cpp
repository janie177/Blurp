#include "Mesh.h"
#include "MeshLoader.h"

Mesh::Mesh()
{

}

bool Mesh::Load(const std::string& a_Path, const std::string& a_FileName, blurp::RenderResourceManager& a_ResourceManager)
{
	MeshLoaderSettings settings;
	settings.path = a_Path;
	settings.fileName = a_FileName;
	settings.vertexInstances = nullptr;
	settings.numVertexInstances = 0;
	m_Scene = LoadMesh(settings, a_ResourceManager, true, false, false);
	return true;
}

std::vector<blurp::DrawData>& Mesh::GetDrawDatas()
{
	return m_Scene.drawDatas;
}

std::vector<blurp::DrawData>& Mesh::GetTransparentDrawDatas()
{
	return m_Scene.transparentDrawDatas;
}
