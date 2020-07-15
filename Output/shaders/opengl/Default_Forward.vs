#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;


struct InstanceData
{
    mat4 transform;
};

layout (std140, binding = 0) buffer Instances
{
    InstanceData data[];
} aInstances;

out vec3 color;

void main()
{
    gl_Position = aInstances.data[gl_InstanceID].transform * vec4(aPos, 1.0);
    color = aColor;
}