#pragma once
#include <vector>
#include <Data.h>
#include "MeshLoader.h"

namespace blurp {
    class RenderResourceManager;
}

class Mesh
{
public:
    Mesh();

    bool Load(const std::string& a_Path, const std::string& a_FileName, blurp::RenderResourceManager& a_ResourceManager, bool a_GenerateShadow);

    bool GeneratesShadow() const;

    std::vector<blurp::DrawData>& GetDrawDatas();
    std::vector<blurp::DrawData>& GetTransparentDrawDatas();

private:
    GLTFScene m_Scene;
    bool m_GenerateShadow;
};
