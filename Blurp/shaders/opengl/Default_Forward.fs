#version 460 core

#define MAX_MATERIAL_BATCH_SIZE 512

in vec3 color;

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
layout (std140, location = 0) uniform constData
{ 
  ConstMatAttribs constAttribData[MAX_MATERIAL_BATCH_SIZE];
};

//MAT_BATCH_DEFINE
#endif 


void main()
{
	vec4 outColor = vec4(1.0, 1.0, 1.0, 1.0);

	//SINGLE MATERIAL
	#ifdef MAT_SINGLE_DEFINE

		#ifdef MAT_DIFFUSE_CONSTANT_DEFINE
			outColor = vec4(diffuseConstant, 1.0);
		#endif

		#ifdef MAT_DIFFUSE_TEXTURE_DEFINE
			outColor = texture2D(diffuseTexture, );
		#endif


	//END SINGLE MATERIAL
	#endif


	//MATERIAL BATCH
	#ifdef MAT_BATCH_DEFINE



	//END MATERIAL BATCH
	#endif

	gl_FragColor = vec4(color, 1.0);
}