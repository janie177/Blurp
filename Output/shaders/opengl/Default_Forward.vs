#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aPos2D;
layout(location = 2) in vec2 aUv;
layout(location = 3) in vec3 aNormal;
layout(location = 4) in vec3 aColor;
layout(location = 5) in vec3 aTangent;
layout(location = 6) in vec3 aBoneIndices;
layout(location = 7) in vec3 aBoneWeights;
layout(location = 8) in mat4 aMatrix;


struct InstanceData
{
    mat4 transform;
};

layout (std140, binding = 0) buffer Instances
{
    InstanceData data[];
} aInstances;

layout(location = 0) uniform int numInstances;

out vec3 color;

void main()
{
    vec4 vPos = vec4(aPos, 1.0);

#ifdef VA_MATRIX_DEF
    vPos = aMatrix * vPos;
#endif

    vPos = aInstances.data[gl_InstanceID / numInstances].transform * vPos;

    color = aColor; 
    gl_Position = vPos;
}