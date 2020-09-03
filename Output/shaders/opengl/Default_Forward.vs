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


//STATIC DATAT: Always the same for all draw calls in this shader for a single frame.
layout(std140, binding = 1) uniform StaticData
{
    mat4 viewProjection;            //PV matrix.
    vec4 cameraPositionFarPlane;    //Camera position in world space. W is the far plane value.
    vec4 numLightsNumCascades;      //Number of lights. X = point, Y = spot, Z = directional.   W = number of directional shadow cascades.
    vec4 numShadowsCascadeDistance; //Number of shadow lights. X = point, Y = spot, Z = directional.    W = Distance per directional shadow cascade.
    vec4 ambientLight;              //Total ambient light count.
};

//Uv modifiers
#if defined(VA_UVMODIFIERID_DEF) && defined(VA_UVCOORD_DEF) && defined(DYNAMIC_UVMODIFIER_DEFINE)
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

    //The far plane of the projection matrix.
    float farPlane;
    //Point, spot and directional lights.
    flat vec3 numLights;
    //Point spot and directional lights with shadows.
    flat vec3 numShadows;
    //The ambient light of the scene.
    flat vec3 ambientLight;

#ifdef USE_DIR_SHADOWS_DEFINE
    //Directional shadow cascading.
    flat float numShadowCascades;
    flat float shadowCascadeDistance;
#endif

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
#if defined(VA_MATRIX_DEF) && defined(DYNAMIC_TRANSFORMMATRIX_DEFINE)
    mat4 transform = aInstances.data[gl_InstanceID / numInstances].modelMatrix * aMatrix;

    //NORMAL MATRIX
    #if defined(VA_ITMATRIX_DEF) && defined(DYNAMIC_NORMALMATRIX_DEFINE)
        mat3 normalMatrix = mat3(aInstances.data[gl_InstanceID / numInstances].normalMatrix * aITMatrix);
    #elif defined(VA_ITMATRIX_DEF)
        mat3 normalMatrix = mat3(aInstances.data[gl_InstanceID / numInstances].modelMatrix * aITMatrix);
    #elif defined(DYNAMIC_NORMALMATRIX_DEFINE)
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
#elif defined(DYNAMIC_TRANSFORMMATRIX_DEFINE)
    mat4 transform = aInstances.data[gl_InstanceID / numInstances].modelMatrix;
    
    //NORMAL MATRIX
    #if defined(DYNAMIC_NORMALMATRIX_DEFINE)
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
    outData.tbn = mat3x3(tang, biTang, norm);

    //Regular normals are active.
#elif defined(VA_NORMAL_DEF)
    outData.normal = normalize(normalMatrix * aNormal);
#endif
    
#ifdef VA_COLOR_DEF
    outData.color = aColor;
#endif

#ifdef VA_UVCOORD_DEF
    #if defined(VA_UVMODIFIERID_DEF) && defined(DYNAMIC_UVMODIFIER_DEFINE)
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

    //The world space position of the fragment used in light calculations.
    outData.fragPos =  vec3(transform * vec4(aPos, 1.0));

    //Pass the camera position in world space.
    outData.camPos = cameraPositionFarPlane.xyz;

    //The far plane is stored with the camera to save space.
    outData.farPlane = cameraPositionFarPlane.w;

    //Calculate the projected space.
    gl_Position = viewProjection * vec4(outData.fragPos, 1.0);

    //Pass on light information.
    outData.numLights = numLightsNumCascades.xyz;
    outData.numShadows = numShadowsCascadeDistance.xyz;
    outData.ambientLight = ambientLight.xyz;
    
#ifdef USE_DIR_SHADOWS_DEFINE
    //If directional shadows are used, set the cascade information here.
    outData.numShadowCascades = numLightsNumCascades.w;
    outData.shadowCascadeDistance = numShadowsCascadeDistance.w;
#endif
}