#pragma once
#include <RenderResourceManager.h>

struct CubeMapSettings
{
    std::string path;

    std::string up;
    std::string down;
    std::string left;
    std::string right;
    std::string front;
    std::string back;
};

std::shared_ptr<blurp::Texture> LoadCubeMap(blurp::RenderResourceManager& a_Manager, const CubeMapSettings& a_Settings);