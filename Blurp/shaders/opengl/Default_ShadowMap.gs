#version 460 core
#define MAX_LIGHTS 64

layout (triangles) in;
layout (triangle_strip, max_vertices = 78) out;

//Output for positional only.
#ifdef POSITIONAL
out GEOMETRY_OUT
{
    flat vec4 lightPosition;
    vec4 fragmentPosition;
} outData;
#endif

//Index data.
layout(std140, binding = 2) uniform LightIndices
{
    int numLightIndices;
    ivec4 lightIndices[MAX_LIGHTS];
};

//POS LIGHTS
#ifdef POSITIONAL
struct PosLightData
{
    vec4 lightPosition;
    mat4 transforms[6];
    int shadowMapId;
};

layout(std140, binding = 1) uniform PosLights
{
    PosLightData posLightData[MAX_LIGHTS];
};
#endif

//DIR LIGHTS
#ifdef DIRECTIONAL

layout(std140, binding = 1) uniform DirLights
{
    int numCascades;
    int dirShadowMapId[MAX_LIGHTS];
} dirLights;

struct DirCascade
{
    vec4 depthClipSpace;
    mat4 transform;
};

//Transforms. N for each light, where N is the number of cascades.
layout(std430, binding = 3) buffer DirCascades
{
    DirCascade cascades[];
} dirCascades;

#endif

void main()
{
#ifdef POSITIONAL
    //First loop over every light index and get the corresponding light.
    for(int light = 0; light < numLightIndices; light += 4)
    {
        int numElements = min(numLightIndices - light, 4);
        int vecIndex = light / 4;

        for(int element = 0; element < numElements; ++element)
        {
            int lightIndex = lightIndices[vecIndex][element];
        
            //Get the transform for every cubemap face and transform all three vertices. Then set the layer accordingly.
            for(int faceIndex = 0; faceIndex < 6; ++faceIndex)
            {   
                //Then calculate the layer index for the current light.
                gl_Layer = (6 * int(posLightData[lightIndex].shadowMapId)) + faceIndex;
                for(int i = 0; i < gl_in.length(); ++i)
                {
                    outData.fragmentPosition = gl_in[i].gl_Position;
                    outData.lightPosition = posLightData[lightIndex].lightPosition;
                    gl_Position = posLightData[lightIndex].transforms[faceIndex] * outData.fragmentPosition;
                    EmitVertex();
                }
                EndPrimitive();
            }
        }
    }
#endif

#ifdef DIRECTIONAL
    for(int light = 0; light < numLightIndices; light += 4)
    {
        int numElements = min(numLightIndices - light, 4);
        int vecIndex = light / 4;

        for(int element = 0; element < numElements; ++element)
        {
            int lightIndex = lightIndices[vecIndex][element];
        
            //Calculate the layer based on the current cascade.
            int cascadeBase = (dirLights.numCascades * dirLights.dirShadowMapId[lightIndex]);

            //Loop over every cascade because some triangles are in multiple cascades. 
            for(int cascadeIndex = 0; cascadeIndex < dirLights.numCascades; ++cascadeIndex)
            {
                int cascade = cascadeBase + cascadeIndex;

                //Loop over every vertex and output it to this cascade.
                for(int i = 0; i < gl_in.length(); ++i)
                {
                    gl_Layer = cascade;
                    gl_Position = dirCascades.cascades[cascade].transform * gl_in[i].gl_Position;
                    EmitVertex();
                }
                EndPrimitive();
            }            
        }
    }
#endif
}  