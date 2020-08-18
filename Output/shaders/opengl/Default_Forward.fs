#version 460 core

#define MAX_MATERIAL_BATCH_SIZE 512
#define MAX_LIGHTS 580

in VERTEX_OUT
{
    //Vertex position in world space without the projection applied.
    vec3 fragPos;

    //Camera position.
    vec3 camPos;

    //Point, spot and directional lights.
    flat vec3 numLights;

    //Point spot and directional lights with shadows.
    flat vec3 numShadows;

    //The ambient light of the scene.
    flat vec3 ambientLight;

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


//LIGHT DATA
	
	//Shadow sampler arrays.
	layout(binding = 6) uniform sampler2DArrayShadow shadowSampler2D;
	layout(binding = 7) uniform samplerCubeArrayShadow shadowSamplerCube;

    struct LightData
    {
        vec4 v1;
        vec4 v2;
        vec4 v3;
        mat4 m1;
    };

    layout(std140, binding = 4) uniform LightDataBuffer
    {
        LightData lightData[MAX_LIGHTS];
    };
//END LIGHT DATA


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
				vec2 P = viewDirection.xy * 0.6;	//SCALE 
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



//Light calculations
	//Diffuse can always be applied.
	vec3 totalDiffuseLight = inData.ambientLight;

#if defined(VA_NORMAL_DEF)
    int offset = 0;

    //Point lights.
    for(int i = 0; i < inData.numLights.x; ++i)
    {
		LightData data = lightData[offset];

		//Calculate the light position either in tangent or world space.
        #if defined(VA_NORMAL_DEF) && defined(VA_TANGENT_DEF) && defined(MAT_NORMAL_TEXTURE_DEFINE)
			vec3 lightPos = inData.tbn * data.v2.xyz;
        #else
			vec3 lightPos = data.v2.xyz;
        #endif

		vec3 lightColor = data.v1.xyz;
		float intensity = data.v1.w;
		//Get the light direction, distance and normalize.
		vec3 lightDir = lightPos - inData.fragPos.xyz;
		float length2 = dot(lightDir, lightDir);
		lightDir /= sqrt(length2);
		//Intensity is equal to the incoming angle multiplied by the inverse square law.
		intensity = max(dot(surfaceNormal, lightDir), 0.0) * (intensity / length2);
		//Append to total light.
        totalDiffuseLight += (intensity * lightColor);

		//Increment offset into light buffer.
        ++offset;
    }

	//Point lights with shadowmaps.
	for(int i = 0; i < inData.numShadows.x; ++i)
    {
		LightData data = lightData[offset];

		//Calculate the light position either in tangent or world space.
        #if defined(VA_NORMAL_DEF) && defined(VA_TANGENT_DEF) && defined(MAT_NORMAL_TEXTURE_DEFINE)
			vec3 lightPos = inData.tbn * data.v2.xyz;
        #else
			vec3 lightPos = data.v2.xyz;
        #endif

		vec3 lightColor = data.v1.xyz;
		float intensity = data.v1.w;
		//Get the light direction, distance and normalize.
		vec3 lightDir = lightPos - inData.fragPos.xyz;
		float length2 = dot(lightDir, lightDir);
		lightDir /= sqrt(length2);
		//Intensity is equal to the incoming angle multiplied by the inverse square law.
		intensity = max(dot(surfaceNormal, lightDir), 0.0) * (intensity / length2);
		//Append to total light.
        totalDiffuseLight += (intensity * lightColor);


		//TODO index into shadow map and check for shadow with the light matrix.


		//Increment offset into light buffer.
        ++offset;
    }


	//Spot lights.
    for(int i = 0; i < inData.numLights.y; ++i)
    {
		LightData data = lightData[offset];

		//Calculate the light position either in tangent or world space.
        #if defined(VA_NORMAL_DEF) && defined(VA_TANGENT_DEF) && defined(MAT_NORMAL_TEXTURE_DEFINE)
			vec3 lightPos = inData.tbn * data.v2.xyz;
			vec3 spotLightDirection = inData.tbn * data.v3.xyz;
        #else
			vec3 lightPos = data.v2.xyz;
			vec3 spotLightDirection = data.v3.xyz;
        #endif

		vec3 lightColor = data.v1.xyz;
		float intensity = data.v1.w;
		//Get the light direction, distance and normalize.
		vec3 lightDir = lightPos - inData.fragPos.xyz;
		float length2 = dot(lightDir, lightDir);
		lightDir /= sqrt(length2);

		//Ensure that the position is within the angle.
		float angle = acos(-dot(lightDir, spotLightDirection));
		if (angle > data.v3.w)	//v3.w contains the angle of the spotlight.
		{
			//Angle doesn't fall in the cone of the spotlight, so just don't do anything.
			++offset;
			continue;
		}

		//Intensity is equal to the incoming angle multiplied by the inverse square law.
		intensity = max(dot(surfaceNormal, lightDir), 0.0) * (intensity / length2);
		//Append to total light.
        totalDiffuseLight += (intensity * lightColor);

		//Increment offset into light buffer.
        ++offset;
    }

	//Spot lights with shadows.
	for(int i = 0; i < inData.numShadows.y; ++i)
    {
		LightData data = lightData[offset];

		//Calculate the light position either in tangent or world space.
        #if defined(VA_NORMAL_DEF) && defined(VA_TANGENT_DEF) && defined(MAT_NORMAL_TEXTURE_DEFINE)
			vec3 lightPos = inData.tbn * data.v2.xyz;
			vec3 spotLightDirection = inData.tbn * data.v3.xyz;
        #else
			vec3 lightPos = data.v2.xyz;
			vec3 spotLightDirection = data.v3.xyz;
        #endif

		vec3 lightColor = data.v1.xyz;
		float intensity = data.v1.w;
		//Get the light direction, distance and normalize.
		vec3 lightDir = lightPos - inData.fragPos.xyz;
		float length2 = dot(lightDir, lightDir);
		lightDir /= sqrt(length2);

		//Ensure that the position is within the angle.
		float angle = acos(-dot(lightDir, spotLightDirection));
		if (angle > data.v3.w)	//v3.w contains the angle of the spotlight.
		{
			//Angle doesn't fall in the cone of the spotlight, so just don't do anything.
			++offset;
			continue;
		}

		//Intensity is equal to the incoming angle multiplied by the inverse square law.
		intensity = max(dot(surfaceNormal, lightDir), 0.0) * (intensity / length2);
		//Append to total light.
        totalDiffuseLight += (intensity * lightColor);



		//TODO check shadowmap with index and then use the matrix to transform to shadow space.



		//Increment offset into light buffer.
        ++offset;
    }

    //Directional lights.
    for(int i = 0; i < inData.numLights.z; ++i)
    {
		LightData data = lightData[offset];

		//Retrieve the light direction and transform it if we're in tangent space.
        #if defined(VA_NORMAL_DEF) && defined(VA_TANGENT_DEF) && defined(MAT_NORMAL_TEXTURE_DEFINE)
			vec3 lightDir = inData.tbn * data.v2.xyz;
        #else
			vec3 lightDir = data.v2.xyz;
        #endif

		vec3 lightColor = data.v1.xyz;
		float intensity = data.v1.w;

		//Intensity for dir lights has no dropoff because there's no position.
		intensity = max(-dot(surfaceNormal, lightDir), 0.0);

		//Append to total light.
        totalDiffuseLight += (intensity * lightColor);

		//Increment offset into light buffer.
        ++offset;
    }

    //Directional lights with shadows.
    for(int i = 0; i < inData.numShadows.z; ++i)
    {
		LightData data = lightData[offset];

		//Retrieve the light direction and transform it if we're in tangent space.
        #if defined(VA_NORMAL_DEF) && defined(VA_TANGENT_DEF) && defined(MAT_NORMAL_TEXTURE_DEFINE)
			vec3 lightDir = inData.tbn * data.v2.xyz;
        #else
			vec3 lightDir = data.v2.xyz;
        #endif

		vec3 lightColor = data.v1.xyz;
		float intensity = data.v1.w;

		//Intensity for dir lights has no dropoff because there's no position.
		intensity = max(-dot(surfaceNormal, lightDir), 0.0);


		//TODO compare shadows with shadow matrix and index into shadow texture.


		//Append to total light.
        totalDiffuseLight += (intensity * lightColor);

		//Increment offset into light buffer.
        ++offset;
    }
#endif

	//Set the output color RGB channels to be within 0 and 1, with all light taken into account.
	//If normals are not active, this will only contain ambient lights.
	outColor = vec4(clamp(totalDiffuseLight * outColor.rgb, 0.0, 1.0), 1.0);

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