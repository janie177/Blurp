#version 460 core

#define MAX_MATERIAL_BATCH_SIZE 512

//LIGHT TEST REMOVE THIS LATER

	vec3 light_pos_world = vec3(-5, 10, -5);
	vec3 light_color = vec3(1, 1, 1);
	vec3 ambient_light = vec3(0.05, 0.05, 0.05);
//END OF LIGHT TEST


in VERTEX_OUT
{
    //Vertex position in world space without the projection applied.
    vec3 fragPos;

    //Camera position.
    vec3 camPos;

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
	vec4 outColor = vec4(1.0, 1.0, 1.0, 1.0);
	vec2 texCoords = inData.uv;
	vec3 viewDirection = normalize(inData.camPos - inData.fragPos);

	vec3 lPos = light_pos_world;

	//SINGLE MATERIAL
	#ifdef MAT_SINGLE_DEFINE

			//AmbientOcclusion/Height. Requires normalmapping to be active too.
		#if (defined(MAT_OCCLUSION_TEXTURE_DEFINE) || defined(MAT_HEIGHT_TEXTURE_DEFINE))
			vec4 oh = texture2D(occlusionheightTexture, texCoords);

			#if defined(MAT_HEIGHT_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF) && defined(VA_NORMAL_DEF) && defined(VA_TANGENT_DEF) && defined(MAT_NORMAL_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF)
				//float height = float( ((((int(round(oh.g * 255))) << 8) + int(round(oh.b * 255))) / 65535.0)) * 10.0;

				const float numLayers = 10;
				float layerDepth = 1.0 / numLayers;
				float currentLayerDepth = 0.0;
				vec2 P = viewDirection.xy * 0.6; 
				vec2 deltaTexCoords = P / numLayers;
				vec2  currentTexCoords     = texCoords;
				float currentDepthMapValue = 1.0 - texture(occlusionheightTexture, currentTexCoords).g;
				while(currentLayerDepth < currentDepthMapValue)
				{
					// shift texture coordinates along direction of P
					currentTexCoords -= deltaTexCoords;
					// get depthmap value at current texture coordinates
					currentDepthMapValue = 1.0 - texture(occlusionheightTexture, currentTexCoords).g;  
					// get depth of next layer
					currentLayerDepth += layerDepth;  
				}

				// get texture coordinates before collision (reverse operations)
				vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

				// get depth after and before collision for linear interpolation
				float afterDepth  = currentDepthMapValue - currentLayerDepth;
				float beforeDepth = (1.0 - texture(occlusionheightTexture, prevTexCoords).g) - currentLayerDepth + layerDepth;
 
				// interpolation of texture coordinates
				float weight = afterDepth / (afterDepth - beforeDepth);
				vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

				texCoords = finalTexCoords;
			#endif

				//Retrieve again because texCoords may have changed.
			#ifdef MAT_OCCLUSION_TEXTURE_DEFINE
				float ao = texture2D(occlusionheightTexture, texCoords).r;
			#endif
		#endif


		//Diffuse constant
		#ifdef MAT_DIFFUSE_CONSTANT_DEFINE
			outColor = vec4(diffuseConstant, 1.0);
		#endif

		//Diffuse texture
		#if defined(MAT_DIFFUSE_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF)
			outColor = texture2D(diffuseTexture, texCoords);
		#endif

		//Normal texture
		#if defined(VA_NORMAL_DEF) && defined(VA_TANGENT_DEF) && defined(MAT_NORMAL_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF)
			vec3 surfaceNormal = texture2D(normalTexture, texCoords).rgb;
			surfaceNormal = surfaceNormal * 2.0 - 1.0;   
			surfaceNormal = normalize(surfaceNormal);

			//TODO remove this whith actual lighting
			lPos = inData.tbn * light_pos_world;
			//END TODO

		//Regular normal
		#elif defined(VA_NORMAL_DEF)
			vec3 surfaceNormal = inData.normal;
		#endif

		//Emissive constant
		#ifdef MAT_EMISSIVE_CONSTANT_DEFINE
			outColor *= vec4(emissiveConstant, 1.0);
		#endif
		//Emissive texture
		#if defined(MAT_EMISSIVE_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF)
			outColor *= texture2D(emissiveTexture, texCoords);
		#endif


		//MetalRoughnessAlpha
		#if (defined(MAT_METALLIC_TEXTURE_DEFINE) || defined(MAT_ROUGHNESS_TEXTURE_DEFINE) || defined(MAT_ALPHA_TEXTURE_DEFINE)) && defined(VA_UVCOORD_DEF)
			vec4 mra = texture2D(metalroughalphaTexture, texCoords);

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

	//END SINGLE MATERIAL
	#endif


	//MATERIAL BATCH
	#ifdef MAT_BATCH_DEFINE



	//END MATERIAL BATCH
	#endif

	
	//Modify diffuse with vertex color.
	#ifdef VA_COLOR_DEF
		outColor *= vec4(inData.color, 1.0);
	#endif

	//Calculate light if a normal is present.
	#if defined(VA_NORMAL_DEF)
		vec3 lightDir = normalize(inData.fragPos.xyz - lPos);

		float intensity = max(dot(surfaceNormal, lightDir), 0.0);
		outColor = vec4(max(intensity * outColor.xyz, ambient_light), outColor.a);
	#endif

	gl_FragColor = outColor;
}