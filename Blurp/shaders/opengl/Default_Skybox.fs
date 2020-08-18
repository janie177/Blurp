#version 460 core

layout(binding = 0) uniform samplerCube skyboxSampler;

layout(location = 1) uniform vec3 colorMix;
layout(location = 2) uniform vec3 colorMultiplier;
layout(location = 3) uniform float alpha;


in vec3 direction;

void main()
{
	gl_FragColor = vec4((colorMultiplier * texture(skyboxSampler, direction).xyz) + colorMix, alpha);
}