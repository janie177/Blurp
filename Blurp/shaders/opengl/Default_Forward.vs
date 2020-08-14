#version 460 core

layout(location = 0) in vec2 aPos2D;
layout(location = 1) in vec3 aPos;
layout(location = 2) in vec2 aUv;
layout(location = 3) in vec3 aNormal;
layout(location = 4) in vec3 aColor;
layout(location = 5) in vec3 aTangent;
layout(location = 6) in vec3 aBiTangent;
layout(location = 7) in vec3 aBoneIndices;
layout(location = 8) in vec3 aBoneWeights;
layout(location = 9) in mat4 aMatrix;
layout(location = 13) in unsigned short aMaterialID;
layout(location = 14) in mat4 aITMatrix;


//INSTANCE DATA
#if defined(INSTANCE_DATA_M) || defined(INSTANCE_DATA_IM)
    struct InstanceData
    {
    #ifdef INSTANCE_DATA_M
        mat4 modelMatrix;           //ModelMatrix to transform to screen space.
    #endif

    #ifdef INSTANCE_DATA_IM
        mat4 normalMatrix;          //Normal to camera space.
    #endif
    };

    layout (std430, binding = 0) buffer Instances
    {
        InstanceData data[];
    } aInstances;

//END INSTANCE DATA
#endif

//Uniforms that are always required.
layout(location = 0) uniform int numInstances;

//CAMERA DATA
layout(std140, binding = 1) uniform CameraMatrices
{
    mat4 viewProjection;
    vec4 cameraPosition;
};


//Output
out VERTEX_OUT
{
    //Vertex position in world space without the projection applied.
    vec4 fragPos;

    #ifdef VA_COLOR_DEF
    vec3 color;
    #endif

    //Texture coordinates are used.
    #ifdef VA_UVCOORD_DEF
    vec2 uv;
    #endif

    //If normalmapping is enabled and a normal and tangent are provided. Bitangent is optional in the calculation (cross product possible).
    #if defined(VA_NORMAL_DEF) && defined(VA_TANGENT_DEF) && defined(MAT_NORMAL_TEXTURE_DEFINE)
    mat3x3 tbn;

    //Normal in light space.
    #elif defined(VA_NORMAL_DEF)
    vec3 normal;
    #endif

} outData;


void main()
{    
//TRANSFORM MATRIX
#if defined(VA_MATRIX_DEF) && defined(INSTANCE_DATA_M)
    mat4 transform = aInstances.data[gl_InstanceID / numInstances].modelMatrix * aMatrix;

    //NORMAL MATRIX
    #if defined(VA_ITMATRIX_DEF) && defined(INSTANCE_DATA_IM)
        mat4 normalMatrix = aInstances.data[gl_InstanceID / numInstances].normalMatrix * aITMatrix;
    #elif defined(VA_ITMATRIX_DEF)
        mat4 normalMatrix = aInstances.data[gl_InstanceID / numInstances].modelMatrix * aITMatrix;
    #elif defined(INSTANCE_DATA_IM)
        mat4 normalMatrix = aInstances.data[gl_InstanceID / numInstances].normalMatrix * aMatrix;
    #else
        mat4 normalMatrix = transform;
    #endif

    //Only use the vertex attribute matrix.
#elif defined(VA_MATRIX_DEF)
    mat4 transform = aMatrix;

    //NORMAL MATRIX
    #if defined(VA_ITMATRIX_DEF)
        mat4 normalMatrix = aITMatrix;
    #else
        mat4 normalMatrix = transform;
    #endif

    //Only use the uploaded matrix.
#elif defined(INSTANCE_DATA_M)
    mat4 transform = aInstances.data[gl_InstanceID / numInstances].modelMatrix;
    
    //NORMAL MATRIX
    #if defined(INSTANCE_DATA_IM)
        mat4 normalMatrix = aInstances.data[gl_InstanceID / numInstances].normalMatrix;
    #else
        mat4 normalMatrix = transform;
    #endif

    //No transforms are used, just use the identity.
#else
    mat4 transform = mat4(1.0);
    mat4 normalMatrix = transform;
#endif
//END OF MATRIX. transform = model to world. normalMatrix is defined for normal to world space.

    //Normalmapping is active.
#if defined(VA_NORMAL_DEF) && defined(VA_TANGENT_DEF) && defined(MAT_NORMAL_TEXTURE_DEFINE)
    vec3 norm = normalize(vec3(model * vec4(aNormal, 0.0)));
    vec3 tang = normalize(vec3(normalMatrix * vec4(aTangent, 0.0)));
    
    //Calculate bitangent if not provided.
    #if defined(VA_BITANGENT_DEF)
        vec3 biTang = normalize(vec3(normalMatrix * vec4(aBiTangent, 0.0)));
    #else
        vec3 biTang = cross(norm, tang);
    #endif
    outData.tbn = mat3x3(tang, biTang, norm)

    //Regular normals are active.
#elif defined(VA_NORMAL_DEF)
    outData.normal = normalize(vec3( normalMatrix * vec4(aNormal, 0.0)));
#endif
    
#ifdef VA_COLOR_DEF
    outData.color = aColor;
#endif

#ifdef VA_UVCOORD_DEF
    outData.uv = aUv;
#endif  


//WORLD SPACE POSITION FOR LIGHT CALCULATIONS
    outData.fragPos = transform * vec4(aPos, 1.0);       //Vertex position in projected space.

//PROJECTED SPACE.
    gl_Position = viewProjection * outData.fragPos;
}