#version 460 core

#define MAX_MATERIAL_BATCH_SIZE 512

//LIGHT TEST REMOVE THIS LATER

	vec3 light_pos_world = vec3(-5, 10, -5);
	vec3 light_color = vec3(1, 1, 1);
	vec3 ambient_light = vec3(0.02, 0.02, 0.02);
//END OF LIGHT TEST

in VERTEX_OUT
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
#if defined(MAT_BATCH_DEFINE) && defined(VA_MATERIALID_DEF)

	//The amount of textures currently active in the material batch, representing stride.
	layout(location = 6) uniform int numActiveBatchTextures;

	//The texture array containing all material textures.
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
	vec3 viewDirection = normalize(inData.camPos - inData.fragPos);

	//Texture coordinates that can be updated by the parallax mapping.
	#ifdef VA_UVCOORD_DEF
	vec2 texCoords = inData.uv;
	#endif

	#if defined(MAT_EMISSIVE_CONSTANT_DEFINE) || defined(MAT_EMISSIVE_TEXTURE_DEFINE)
	vec4 emissiveModifier = vec4(0, 0, 0, 0);
	#endif

	#if defined(MAT_OCCLUSION_TEXTURE_DEFINE)
	float aoModifier = 1.0;
	#endif

	#if defined(MAT_ALPHA_TEXTURE_DEFINE) || defined(MAT_ALPHA_CONSTANT_DEFINE)
	float alphaModifier = 1.0;
	#endif

	vec3 lPos = light_pos_world;

	//Normals are used, but no normalmapping is enabled.
	#if defined(VA_NORMAL_DEF) && !(defined(VA_TANGENT_DEF) && defined(MAT_NORMAL_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF))
	vec3 surfaceNormal = inData.normal;
	#endif

	//Metallic is used.
	#if defined(MAT_METALLIC_TEXTURE_DEFINE) || defined(MAT_METALLIC_CONSTANT_DEFINE)
	float metallic = 0;
	#endif

	//Roughness is used.
	#if defined(MAT_ROUGHNESS_TEXTURE_DEFINE) || defined(MAT_ROUGHNESS_CONSTANT_DEFINE)
	float roughness = 0;
	#endif

	//SINGLE MATERIAL
	#ifdef MAT_SINGLE_DEFINE

			//AmbientOcclusion/Height. Requires normalmapping to be active too.
		#if (defined(MAT_OCCLUSION_TEXTURE_DEFINE) || defined(MAT_HEIGHT_TEXTURE_DEFINE))
			#if defined(MAT_HEIGHT_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF) && defined(VA_NORMAL_DEF) && defined(VA_TANGENT_DEF) && defined(MAT_NORMAL_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF)
				//Take multiple samples from different layers, each oriented along the view direction.
				const float numLayers = 10;
				float layerDepth = 1.0 / numLayers;
				float currentLayerDepth = 0.0;
				vec2 P = viewDirection.xy * 0.6; 
				vec2 deltaTexCoords = P / numLayers;
				vec2  currentTexCoords     = texCoords;
				float currentDepthMapValue = 1.0 - texture(occlusionheightTexture, currentTexCoords).g;
				while(currentLayerDepth < currentDepthMapValue)
				{
					currentTexCoords -= deltaTexCoords;
					currentDepthMapValue = 1.0 - texture(occlusionheightTexture, currentTexCoords).g;  
					currentLayerDepth += layerDepth;  
				}

				//Interpolate between the layers to find the best matching depth.
				vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
				float afterDepth  = currentDepthMapValue - currentLayerDepth;
				float beforeDepth = (1.0 - texture(occlusionheightTexture, prevTexCoords).g) - currentLayerDepth + layerDepth;
				float weight = afterDepth / (afterDepth - beforeDepth);
				vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

				texCoords = finalTexCoords;
			#endif

				//Retrieve again because texCoords may have changed.
			#ifdef MAT_OCCLUSION_TEXTURE_DEFINE
				aoModifier = texture2D(occlusionheightTexture, texCoords).r;
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

		#endif

		//Emissive constant
		#ifdef MAT_EMISSIVE_CONSTANT_DEFINE
			emissiveModifier = vec4(emissiveConstant, 1.0);
		#endif
		//Emissive texture
		#if defined(MAT_EMISSIVE_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF)
			emissiveModifier = texture2D(emissiveTexture, texCoords);
		#endif


		//MetalRoughnessAlpha
		#if (defined(MAT_METALLIC_TEXTURE_DEFINE) || defined(MAT_ROUGHNESS_TEXTURE_DEFINE) || defined(MAT_ALPHA_TEXTURE_DEFINE)) && defined(VA_UVCOORD_DEF)
			vec4 mra = texture2D(metalroughalphaTexture, texCoords);

			#ifdef MAT_METALLIC_TEXTURE_DEFINE
				metallic = mra.r;
			#endif
			#ifdef MAT_ROUGHNESS_TEXTURE_DEFINE
				roughness = mra.g;
			#endif
			#ifdef MAT_ALPHA_TEXTURE_DEFINE
				alphaModifier = alphaConstant;
			#endif

			
		#endif
		//Metal constant
		#ifdef MAT_METALLIC_CONSTANT_DEFINE
			metallic = metallicConstant;
		#endif
		//Roughness constant
		#ifdef MAT_ROUGHNESS_CONSTANT_DEFINE
			roughness = roughnessConstant;
		#endif
		//Alpha constant
		#ifdef MAT_ALPHA_CONSTANT_DEFINE
			alphaModifier = alphaConstant;
		#endif

	//END SINGLE MATERIAL
	#endif


	//MATERIAL BATCH
	#if defined(MAT_BATCH_DEFINE) && defined(VA_MATERIALID_DEF)
	int texLayerOffset = 0;

					//AmbientOcclusion/Height. Requires normalmapping to be active too.
		#if (defined(MAT_OCCLUSION_TEXTURE_DEFINE) || defined(MAT_HEIGHT_TEXTURE_DEFINE))
			#if defined(MAT_HEIGHT_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF) && defined(VA_NORMAL_DEF) && defined(VA_TANGENT_DEF) && defined(MAT_NORMAL_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF)
				//Take multiple samples from different layers, each oriented along the view direction.
				const float numLayers = 10;
				float layerDepth = 1.0 / numLayers;
				float currentLayerDepth = 0.0;
				vec2 P = viewDirection.xy * 0.6; 
				vec2 deltaTexCoords = P / numLayers;
				vec2  currentTexCoords     = texCoords;
				float currentDepthMapValue = 1.0 - texture(materialArray, vec3(currentTexCoords, (numActiveBatchTextures * inData.materialID) + texLayerOffset)).g;
				while(currentLayerDepth < currentDepthMapValue)
				{
					currentTexCoords -= deltaTexCoords;
					currentDepthMapValue = 1.0 - texture(materialArray, vec3(currentTexCoords, inData.materialID + texLayerOffset)).g;  
					currentLayerDepth += layerDepth;  
				}

				//Interpolate between the layers to find the best matching depth.
				vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
				float afterDepth  = currentDepthMapValue - currentLayerDepth;
				float beforeDepth = (1.0 - texture(materialArray, vec3(prevTexCoords, (numActiveBatchTextures * inData.materialID) + texLayerOffset)).g) - currentLayerDepth + layerDepth;
				float weight = afterDepth / (afterDepth - beforeDepth);
				vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

				texCoords = finalTexCoords;
			#endif

				//Retrieve again because texCoords may have changed.
			#ifdef MAT_OCCLUSION_TEXTURE_DEFINE
				aoModifier = texture(materialArray, vec3(texCoords, (numActiveBatchTextures * inData.materialID) + texLayerOffset)).r;
			#endif

			//Increment the offset into the texture buffer.
			++texLayerOffset;
		#endif


		//Diffuse constant
		#ifdef MAT_DIFFUSE_CONSTANT_DEFINE

			outColor = constAttribData[inData.materialID].diffuse;
		#endif

		//Diffuse texture
		#if defined(MAT_DIFFUSE_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF)
			outColor = texture(materialArray, vec3(texCoords, (numActiveBatchTextures * inData.materialID) + texLayerOffset));
			++texLayerOffset;

		#endif

		//Normal texture
		#if defined(VA_NORMAL_DEF) && defined(VA_TANGENT_DEF) && defined(MAT_NORMAL_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF)
			vec3 surfaceNormal = texture(materialArray, vec3(texCoords, (numActiveBatchTextures * inData.materialID) + texLayerOffset)).rgb;
			surfaceNormal = surfaceNormal * 2.0 - 1.0;   
			surfaceNormal = normalize(surfaceNormal);

			//TODO remove this whith actual lighting
			lPos = inData.tbn * light_pos_world;
			//END TODO

			++texLayerOffset;
		#endif

		//Emissive constant
		#ifdef MAT_EMISSIVE_CONSTANT_DEFINE
			emissiveModifier = constAttribData[inData.materialID].emissive;
		#endif
		//Emissive texture
		#if defined(MAT_EMISSIVE_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF)
			emissiveModifier = texture(materialArray, vec3(texCoords, (numActiveBatchTextures * inData.materialID) + texLayerOffset));
			++texLayerOffset;
		#endif


		//MetalRoughnessAlpha
		#if (defined(MAT_METALLIC_TEXTURE_DEFINE) || defined(MAT_ROUGHNESS_TEXTURE_DEFINE) || defined(MAT_ALPHA_TEXTURE_DEFINE)) && defined(VA_UVCOORD_DEF)
			vec4 mra = texture(materialArray, vec3(texCoords, (numActiveBatchTextures * inData.materialID) + texLayerOffset));

			#ifdef MAT_METALLIC_TEXTURE_DEFINE
				metallic = mra.r;
			#endif
			#ifdef MAT_ROUGHNESS_TEXTURE_DEFINE
				roughness = mra.g;
			#endif
			#ifdef MAT_ALPHA_TEXTURE_DEFINE
				alphaModifier = alphaConstant;
			#endif

			++texLayerOffset;
		#endif
		//Metal constant
		#ifdef MAT_METALLIC_CONSTANT_DEFINE
			metallic = constAttribData[inData.materialID].metallic;
		#endif
		//Roughness constant
		#ifdef MAT_ROUGHNESS_CONSTANT_DEFINE
			roughness = constAttribData[inData.materialID].roughness;;
		#endif
		//Alpha constant
		#ifdef MAT_ALPHA_CONSTANT_DEFINE
			alphaModifier = constAttribData[inData.materialID].alpha;;
		#endif

	//END MATERIAL BATCH
	#endif

	
	//Modify diffuse with vertex color.
	#ifdef VA_COLOR_DEF
		outColor *= vec4(inData.color, 1.0);
	#endif

	//Calculate light if a normal is present.
	#if defined(VA_NORMAL_DEF)
		vec3 lightDir = normalize(inData.fragPos.xyz - lPos);

		float intensity = max(dot(surfaceNormal, -lightDir), 0.0);
		outColor = vec4((intensity * outColor.rgb) + (outColor.rgb * ambient_light), 1.0);
	#endif


	//Apply AO, alpha and emissive.

	#if defined(MAT_OCCLUSION_TEXTURE_DEFINE)
	outColor *= aoModifier;
	#endif

	#if defined(MAT_EMISSIVE_CONSTANT_DEFINE) || defined(MAT_EMISSIVE_TEXTURE_DEFINE)
	outColor += emissiveModifier;
	#endif
	
	#if defined(MAT_ALPHA_TEXTURE_DEFINE) || defined(MAT_ALPHA_CONSTANT_DEFINE)
	outColor.a = alphaModifier;
	#endif

	//Set the final color as the fragment shader output.
	gl_FragColor = outColor;
}