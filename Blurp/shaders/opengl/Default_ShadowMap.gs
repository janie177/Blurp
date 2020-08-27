#version 460 core
#define MAX_LIGHTS 64

layout (triangles) in;
layout (triangle_strip, max_vertices = 78) out;


out GEOMETRY_OUT
{
    flat vec4 lightPosition;
    vec4 fragmentPosition;
} outData;

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

layout(std140, binding = 2) uniform LightIndices
{
    int numLightIndices;
    ivec4 lightIndices[MAX_LIGHTS];
};

void main()
{
//    for(int faceIndex = 0; faceIndex < 6; ++faceIndex)
//    {   
//        //Then calculate the layer index for the current light.
//        gl_Layer = faceIndex;
//        for(int i = 0; i < gl_in.length(); ++i)
//        {
//            vec4 pos = posLightData[0].transforms[faceIndex] * gl_in[i].gl_Position;
//            gl_Position = pos;
//            EmitVertex();
//        }    
//        EndPrimitive();
//    }

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
}  