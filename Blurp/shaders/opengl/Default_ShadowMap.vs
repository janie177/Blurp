#version 460 core

#define MAX_LIGHTS 580

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
#if defined(DYNAMIC_NORMALMATRIX_DEFINE) || defined(DYNAMIC_TRANSFORMMATRIX_DEFINE)
    struct InstanceData
    {
    #ifdef DYNAMIC_TRANSFORMMATRIX_DEFINE
        mat4 modelMatrix;           //ModelMatrix to transform to screen space.
    #endif

    #ifdef DYNAMIC_NORMALMATRIX_DEFINE
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

void main()
{    
//TRANSFORM MATRIX
#if defined(VA_MATRIX_DEF) && defined(DYNAMIC_TRANSFORMMATRIX_DEFINE)
    mat4 transform = aInstances.data[gl_InstanceID / numInstances].modelMatrix * aMatrix;

    //Only use the vertex attribute matrix.
#elif defined(VA_MATRIX_DEF)
    mat4 transform = aMatrix;

    //Only use the uploaded matrix.
#elif defined(DYNAMIC_TRANSFORMMATRIX_DEFINE)
    mat4 transform = aInstances.data[gl_InstanceID / numInstances].modelMatrix;

    //No transforms are used, just use the identity.
#else
    mat4 transform = mat4(1.0);
#endif

    //The world space position of the fragment.
    gl_Position = transform * vec4(aPos, 1.0);
}