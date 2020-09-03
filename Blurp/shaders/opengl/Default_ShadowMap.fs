#version 460 core

layout(location = 1) uniform float farPlane;

#ifdef POSITIONAL
in GEOMETRY_OUT
{
    flat vec4 lightPosition;
    vec4 fragmentPosition;
} inData;
#endif

void main()
{
#ifdef POSITIONAL
	float len = length(inData.lightPosition.xyz - inData.fragmentPosition.xyz);

	//Len is never bigger than the far plane because it's along the Z axis.
	//This means that the frag depth is stored as a float between 0 and 1.
	//By doing this, I can calculate the shadow without further matrix transforms (6 per cubemap) in the forward shader.
	//Dividing objects distance from the light by the far plane + their direction is enough to look them up and compare.
	gl_FragDepth = len / farPlane;
#endif

	//For dir lights this is empty.
}