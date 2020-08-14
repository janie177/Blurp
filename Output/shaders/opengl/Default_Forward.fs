#version 460 core

#define MAX_MATERIAL_BATCH_SIZE 512

in VERTEX_OUT
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
    mat4 tbn;

    //Normal in light space.
    #elif defined(VA_NORMAL_DEF)
    vec3 normal;
    #endif
} inData;

//Samplers for single texture.
layout(binding = 0) uniform sampler2D diffuseTexture;
layout(binding = 1) uniform sampler2D normalTexture;
layout(binding = 2) uniform sampler2D emissiveTexture;
layout(binding = 3) uniform sampler2D metalroughalphaTexture;
layout(binding = 4) uniform sampler2D occlusionheightTexture;

//Samplers containing constant values for the materials.
layout(location = 1) uniform vec3 diffuseConstant;
layout(location = 2) uniform vec3 emissiveConstant;
layout(location = 3) uniform float metallicConstant;
layout(location = 4) uniform float roughnessConstant;
layout(location = 5) uniform float alphaConstant;


//Material batch
#ifdef MAT_BATCH_DEFINE

layout(binding = 5) uniform sampler2DArray materialArray;

//Struct that contains the correct layout for all enabled constant material attributes.
struct ConstMatAttribs
{
#ifdef MAT_DIFFUSE_CONSTANT_DEFINE
	vec4 diffuse;
#endif
#ifdef MAT_EMISSIVE_CONSTANT_DEFINE
	vec4 emissive;
#endif
#ifdef MAT_METALLIC_CONSTANT_DEFINE
	float metallic;
#endif
#ifdef MAT_ROUGHNESS_CONSTANT_DEFINE
	float roughness;
#endif
#ifdef MAT_ALPHA_CONSTANT_DEFINE
	float alpha;
#endif
};

//Uniform block containing batched material info.
layout (std140, binding = 2) uniform constData
{ 
	ConstMatAttribs constAttribData[MAX_MATERIAL_BATCH_SIZE];
};

//MAT_BATCH_DEFINE
#endif 


void main()
{
	vec4 outColor = vec4(0.0, 1.0, 1.0, 1.0);

	//SINGLE MATERIAL
	#ifdef MAT_SINGLE_DEFINE

		//Diffuse constant
		#ifdef MAT_DIFFUSE_CONSTANT_DEFINE
			//outColor = vec4(diffuseConstant, 1.0);
		#endif

		//Diffuse texture
		#if defined(MAT_DIFFUSE_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF)
			outColor = texture2D(diffuseTexture, inData.uv);
		#endif



		//Normal texture
		#if defined(VA_NORMAL_DEF) && defined(VA_TANGENT_DEF) && defined(MAT_NORMAL_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF)
			outColor = texture2D(normalTexture, inData.uv);

		//Regular normal
		#elif defined(VA_NORMAL_DEF)

		#endif



		//Emissive constant
		#ifdef MAT_EMISSIVE_CONSTANT_DEFINE
			outColor *= vec4(emissiveConstant, 1.0);
		#endif
		//Emissive texture
		#if defined(MAT_EMISSIVE_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF)
			outColor *= texture2D(emissiveTexture, inData.uv);
		#endif


		//MetalRoughnessAlpha
		#if (defined(MAT_METALLIC_TEXTURE_DEFINE) || defined(MAT_ROUGHNESS_TEXTURE_DEFINE) || defined(MAT_ALPHA_TEXTURE_DEFINE)) && defined(VA_UVCOORD_DEF)
			vec4 mra = texture2D(metalroughalphaTexture, inData.uv);

			#ifdef MAT_METALLIC_TEXTURE_DEFINE
				float metal = mra.r;
			#endif
			#ifdef MAT_ROUGHNESS_TEXTURE_DEFINE
				float rough = mra.g;
			#endif
			#ifdef MAT_ALPHA_TEXTURE_DEFINE
				outColor.a = alphaConstant;
			#endif

			
		#endif
		//Metal constant
		#ifdef MAT_METALLIC_CONSTANT_DEFINE
			float metal = metallicConstant;
		#endif
		//Roughness constant
		#ifdef MAT_ROUGHNESS_CONSTANT_DEFINE
			float rough = roughnessConstant;
		#endif
		//Alpha constant
		#ifdef MAT_ALPHA_CONSTANT_DEFINE
			outColor.a = alphaConstant;
		#endif

		//AmbientOcclusion/Height
		#if (defined(MAT_OCCLUSION_TEXTURE_DEFINE) || defined(MAT_HEIGHT_TEXTURE_DEFINE)) && defined(VA_UVCOORD_DEF)
			vec4 oh = texture2D(metalroughalphaTexture, inData.uv);

			#ifdef MAT_OCCLUSION_TEXTURE_DEFINE
				float ao = oh.r;
			#endif
			#ifdef MAT_HEIGH_TEXTURE_DEFINE
				short height = (((short)oh.g) << 8) + oh.b;
			#endif
		#endif

	//END SINGLE MATERIAL
	#endif


	//MATERIAL BATCH
	#ifdef MAT_BATCH_DEFINE



	//END MATERIAL BATCH
	#endif

	
	//Vertex color attribute
	#ifdef VA_COLOR_DEF
		outColor *= vec4(inData.color, 1.0);
	#endif

	gl_FragColor = outColor;
}