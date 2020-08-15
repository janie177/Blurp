#include "Sphere.h"
#include <RenderResourceManager.h>

using namespace blurp;


std::shared_ptr<blurp::Mesh> Sphere::Load(BlurpEngine& a_Engine, float a_Radius, int a_XComponents, int a_YComponents)
{
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> uv;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> tangents;
    std::vector<std::uint16_t> indices;

    const float PI = acos(-1);

    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / a_Radius;    // normal
    float s, t;                                     // texCoord

    float sectorStep = 2 * PI / a_XComponents;
    float stackStep = PI / a_YComponents;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= a_YComponents; ++i)
    {
        stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = a_Radius * cosf(stackAngle);             // r * cos(u)
        z = a_Radius * sinf(stackAngle);              // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for (int j = 0; j <= a_XComponents; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            positions.push_back({ x, y, z });

            // normalized vertex normal
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            normals.push_back({ nx, ny, nz });

            // vertex tex coord between [0, 1]
            s = (float)j / a_XComponents;
            t = (float)i / a_YComponents;
            uv.push_back({ s, t });
        }
    }

    // indices
    //  k1--k1+1
    //  |  / |
    //  | /  |
    //  k2--k2+1
    unsigned int k1, k2;
    for (int i = 0; i < a_YComponents; ++i)
    {
        k1 = i * (a_XComponents + 1);     // beginning of current stack
        k2 = k1 + a_XComponents + 1;      // beginning of next stack

        for (int j = 0; j < a_XComponents; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding 1st and last stacks
            if (i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (a_YComponents - 1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    tangents.resize(positions.size());

    for(int i = 0; i < indices.size(); i+=3)
    {
        auto i1 = indices[i];
        auto i2 = indices[i + 1];
        auto i3 = indices[i + 2];

        std::vector<glm::vec3> p;
        std::vector<glm::vec2> u;
        std::vector<glm::vec3> n;

        p.emplace_back(positions[i1]);
        p.emplace_back(positions[i2]);
        p.emplace_back(positions[i3]);

        u.emplace_back(uv[i1]);
        u.emplace_back(uv[i2]);
        u.emplace_back(uv[i3]);

        n.emplace_back(normals[i1]);
        n.emplace_back(normals[i2]);
        n.emplace_back(normals[i3]);

        auto tangent = calculateTangent(
            p, u, n
        );

        tangents[i1] = tangent;
        tangents[i2] = tangent;
        tangents[i3] = tangent;
    }

    std::vector<float> data;
    for (unsigned int i = 0; i < positions.size(); ++i)
    {
        data.push_back(positions[i].x);
        data.push_back(positions[i].y);
        data.push_back(positions[i].z);
        if (normals.size() > 0)
        {
            data.push_back(normals[i].x);
            data.push_back(normals[i].y);
            data.push_back(normals[i].z);
        }
        if (uv.size() > 0)
        {
            data.push_back(uv[i].x);
            data.push_back(uv[i].y);
        }
        if(tangents.size() > 0)
        {
            data.push_back(tangents[i].x);
            data.push_back(tangents[i].y);
            data.push_back(tangents[i].z);
        }
    }

    //Create the mesh that the material is used with.
    MeshSettings meshSettings;
    meshSettings.indexData = &indices[0];
    meshSettings.vertexData = &data[0];
    meshSettings.indexDataType = DataType::USHORT;
    meshSettings.usage = MemoryUsage::GPU;
    meshSettings.access = AccessMode::READ_ONLY;
    meshSettings.vertexDataSizeBytes = sizeof(float) * data.size();
    meshSettings.numIndices = indices.size();

    //Enabled attributes for the mesh.
    meshSettings.vertexSettings.EnableAttribute(VertexAttribute::POSITION_3D, 0, 44, 0);
    meshSettings.vertexSettings.EnableAttribute(VertexAttribute::NORMAL, 12, 44, 0);
    meshSettings.vertexSettings.EnableAttribute(VertexAttribute::UV_COORDS, 24, 44, 0);
    meshSettings.vertexSettings.EnableAttribute(VertexAttribute::TANGENT, 32, 44, 0);

    //Load the mesh.
    return a_Engine.GetResourceManager().CreateMesh(meshSettings);
}
