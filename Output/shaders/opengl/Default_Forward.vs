#version 460 core

#ifdef VA_POS3D_DEF
layout(location = VA_POS3D_LOCATION_DEF) in vec3 aPos;
#endif

#ifdef VA_UVCOORD_DEF
layout(location = VA_UVCOORD_LOCATION_DEF) in vec2 aUv;
#endif

#ifdef VA_NORMAL_DEF
layout(location = VA_NORMAL_LOCATION_DEF) in vec3 aNormal;
#endif

#ifdef VA_COLOR_DEF
layout(location = VA_COLOR_LOCATION_DEF) in vec3 aColor;
#endif

#ifdef VA_TANGENT_DEF
layout(location = VA_TANGENT_LOCATION_DEF) in vec3 aTangent;
#endif

#ifdef VA_BITANGENT_DEF
layout(location = VA_BITANGENT_LOCATION_DEF) in vec3 aBiTangent;
#endif

#ifdef VA_BONEINDEX_DEF
layout(location = VA_BONEINDEX_LOCATION_DEF) in vec3 aBoneIndices;
#endif

#ifdef VA_BONEWEIGHT_DEF
layout(location = VA_BONEWEIGHT_LOCATION_DEF) in vec3 aBoneWeights;
#endif

#ifdef VA_MATERIALID_DEF
layout(location = VA_MATERIALID_LOCATION_DEF) in float aMaterialID;
#endif

#ifdef VA_UVMODIFIERID_DEF
layout(location = VA_UVMODIFIERID_LOCATION_DEF) in float aUvModifierID;
#endif

#ifdef VA_MATRIX_DEF
layout(location = VA_MATRIX_LOCATION_DEF) in mat4 aMatrix;
#endif

#ifdef VA_ITMATRIX_DEF
layout(location = VA_ITMATRIX_LOCATION_DEF) in mat4 aITMatrix;
#endif


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
    vec4 fragPos = transform * vec4(aPos, 1.0);

    //Pass the camera position in world space.
    outData.camPos = cameraPosition.xyz;

    //PROJECTED SPACE. Do this before converting to tangent space.
    gl_Position = viewProjection * fragPos;

    //Convert to tangent space if active.
#if defined(VA_NORMAL_DEF) && defined(VA_TANGENT_DEF) && defined(MAT_NORMAL_TEXTURE_DEFINE)
    outData.camPos = outData.tbn * outData.camPos;
    fragPos = vec4(outData.tbn * fragPos.xyz, fragPos.w);
#endif

    outData.fragPos = fragPos.xyz;
}