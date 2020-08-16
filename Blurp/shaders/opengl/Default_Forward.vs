#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUv;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec3 aColor;
layout(location = 4) in vec3 aTangent;
layout(location = 5) in vec3 aBiTangent;
layout(location = 6) in vec3 aBoneIndices;
layout(location = 7) in vec3 aBoneWeights;
layout(location = 8) in float aMaterialID;
layout(location = 9) in float aUvModifierID;
layout(location = 10) in mat4 aMatrix;
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

//Uv modifiers
#if defined(VA_UVMODIFIERID_DEF) && defined(VA_UVCOORD_DEF)
    struct UvModifier
    {
        vec2 multiply;
        vec2 add;
    };

    layout (std430, binding = 3) buffer UvModifiers
    {
        UvModifier data[];
    } uvModifiers;
#endif

//Output
out VERTEX_OUT
{
    //Vertex position in world space without the projection applied.
    vec3 fragPos;

    //Camera position.
    vec3 camPos;

    //Material ID in the material batch.
	#ifdef VA_MATERIALID_DEF
	flat int materialID;
	#endif

    //Vertex color modifier.
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
        mat3 normalMatrix = mat3(aInstances.data[gl_InstanceID / numInstances].normalMatrix * aITMatrix);
    #elif defined(VA_ITMATRIX_DEF)
        mat3 normalMatrix = mat3(aInstances.data[gl_InstanceID / numInstances].modelMatrix * aITMatrix);
    #elif defined(INSTANCE_DATA_IM)
        mat3 normalMatrix = mat3(aInstances.data[gl_InstanceID / numInstances].normalMatrix * aMatrix);
    #else
        mat3 normalMatrix = mat3(transform);
    #endif

    //Only use the vertex attribute matrix.
#elif defined(VA_MATRIX_DEF)
    mat4 transform = aMatrix;

    //NORMAL MATRIX
    #if defined(VA_ITMATRIX_DEF)
        mat3 normalMatrix = mat3(aITMatrix);
    #else
        mat3 normalMatrix = mat3(transform);
    #endif

    //Only use the uploaded matrix.
#elif defined(INSTANCE_DATA_M)
    mat4 transform = aInstances.data[gl_InstanceID / numInstances].modelMatrix;
    
    //NORMAL MATRIX
    #if defined(INSTANCE_DATA_IM)
        mat3 normalMatrix = mat3(aInstances.data[gl_InstanceID / numInstances].normalMatrix);
    #else
        mat3 normalMatrix = mat3(transform);
    #endif

    //No transforms are used, just use the identity.
#else
    mat4 transform = mat4(1.0);
    mat3 normalMatrix = mat3(transform);
#endif
//END OF MATRIX. transform = model to world. normalMatrix is defined for normal to world space.

    //Normalmapping is active.
#if defined(VA_NORMAL_DEF) && defined(VA_TANGENT_DEF) && defined(MAT_NORMAL_TEXTURE_DEFINE)
    vec3 norm = normalize(normalMatrix * aNormal);
    vec3 tang = normalize(normalMatrix * aTangent);
    
    //Calculate bitangent if not provided.
    #if defined(VA_BITANGENT_DEF)
        vec3 biTang = normalize(normalMatrix * aBiTangent);
    #else
        vec3 biTang = cross(norm, tang);
    #endif
    outData.tbn = transpose(mat3x3(tang, biTang, norm));

    //Regular normals are active.
#elif defined(VA_NORMAL_DEF)
    outData.normal = normalize(normalMatrix * aNormal);
#endif
    
#ifdef VA_COLOR_DEF
    outData.color = aColor;
#endif

#ifdef VA_UVCOORD_DEF
    #if defined(VA_UVMODIFIERID_DEF)
        //Transform the UV coordinate with the data provided.
        UvModifier modifier = uvModifiers.data[int(aUvModifierID)];
        outData.uv = (aUv * modifier.multiply) + modifier.add;
    #else
        //No modifiers so just pass the original one.
        outData.uv = aUv;
    #endif
#endif  

#ifdef VA_MATERIALID_DEF
    outData.materialID = int(aMaterialID);
#endif


    //WORLD SPACE POSITION FOR LIGHT CALCULATIONS
    outData.fragPos =  vec3(transform * vec4(aPos, 1.0));

    //Pass the camera position in world space.
    outData.camPos = cameraPosition.xyz;

    //PROJECTED SPACE. Do this before converting to tangent space.
    gl_Position = viewProjection * vec4(outData.fragPos, 1.0);

    //Convert to tangent space if active.
#if defined(VA_NORMAL_DEF) && defined(VA_TANGENT_DEF) && defined(MAT_NORMAL_TEXTURE_DEFINE)
    outData.camPos = outData.tbn * outData.camPos;
    outData.fragPos = outData.tbn * outData.fragPos;
#endif
}