#version 460 core
layout(location = 0) in vec3 aPosition;

layout(location = 0) uniform mat4 pvMatrix;

out vec3 direction;

void main()
{
	direction = aPosition;	
	gl_Position = (pvMatrix * vec4(aPosition, 1.0)).xyww;
}