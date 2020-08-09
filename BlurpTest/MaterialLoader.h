#pragma once
#include <Material.h>
#include <RenderResourceManager.h>

struct MaterialData
{
    std::string path;

    std::string diffuseTextureName;
    std::string emissiveTextureName;
    std::string normalTextureName;
    std::string aoTextureName;
    std::string heightTextureName;
    std::string metallicTextureName;
    std::string roughnessTextureName;
    std::string alphaTextureName;
};

/*
 * Load a material from the given texture paths.
 * This automatically compresses them in the right format.
 */
std::shared_ptr<blurp::Material> LoadMaterial(blurp::RenderResourceManager& a_Manager, const MaterialData& a_Data);