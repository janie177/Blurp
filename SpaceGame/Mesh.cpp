#include "Mesh.h"
#include "MeshLoader.h"

Mesh::Mesh() : m_GenerateShadow(false)
{

}

bool Mesh::Load(const std::string& a_Path, const std::string& a_FileName, blurp::RenderResourceManager& a_ResourceManager, bool a_GenerateShadow)
{
	MeshLoaderSettings settings;
	settings.path = a_Path;
	settings.fileName = a_FileName;
	settings.vertexInstances = nullptr;
	settings.numVertexInstances = 0;
	m_Scene = LoadMesh(settings, a_ResourceManager, true, false, false);
	m_GenerateShadow = a_GenerateShadow;
	return true;
}

bool Mesh::GeneratesShadow() const
{
	return m_GenerateShadow;
}

std::vector<blurp::DrawData>& Mesh::GetDrawDatas()
{
	return m_Scene.drawDatas;
}

std::vector<blurp::DrawData>& Mesh::GetTransparentDrawDatas()
{
	return m_Scene.transparentDrawDatas;
}
