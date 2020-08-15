#pragma once
#include <BlurpEngine.h>
#include <Mesh.h>


class Sphere
{
public:
	/*
	 * Load the sphere.
	 */
	static std::shared_ptr<blurp::Mesh> Load(blurp::BlurpEngine& a_Engine, float a_Radius, int a_XComponents = 64, int a_YComponents = 64);


    static inline glm::vec3 calculateTangent(
        std::vector<glm::vec3>& vertices,
        std::vector<glm::vec2>& uvs,
        std::vector<glm::vec3>& normals
    )
    {
        // Shortcuts for vertices
        glm::vec3& v0 = vertices[0];
        glm::vec3& v1 = vertices[1];
        glm::vec3& v2 = vertices[2];

        // Shortcuts for UVs
        glm::vec2& uv0 = uvs[0];
        glm::vec2& uv1 = uvs[1];
        glm::vec2& uv2 = uvs[2];

        // Edges of the triangle : position delta
        glm::vec3 deltaPos1 = v1 - v0;
        glm::vec3 deltaPos2 = v2 - v0;

        // UV delta
        glm::vec2 deltaUV1 = uv1 - uv0;
        glm::vec2 deltaUV2 = uv2 - uv0;

        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;

        return tangent;
    }
};