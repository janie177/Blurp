#version 460 core

#define MAX_MATERIAL_BATCH_SIZE 512
#define MAX_LIGHTS 580

//Functions for PBR shading.
vec3 LightReflected(vec3 toLightDir, vec3 toCameraDir, vec3 surfaceNormal, vec3 f0, vec3 lightColor, float lightDistance, float metallic, float roughness, vec3 diffuse);
float DistributionGGX(vec3 surfaceNormal, vec3 h, float roughness);
float GeometrySchlickGGX(float sNormalToCamDot, float roughness);
float GeometrySmith(vec3 surfaceNormal, vec3 toCameraDir, vec3 toLightDir, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 f0);

in VERTEX_OUT
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
	float fragDepth;
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
#ifdef USE_DIR_SHADOWS_DEFINE
	layout(binding = 6) uniform sampler2DArrayShadow shadowSampler2D;

	//Struct containing the shadow transform and the vector which contains the clip space depth for the far plane of each cascade.
	struct DirCascade
	{
		vec4 depthClipSpace;
		mat4 transform;
	};

	//Unbound array containing the transforms to light space for each directional light for each cascade.
	layout(std430, binding = 4) buffer DirCascades
    {
        DirCascade cascades[];
    };

#endif

#ifdef USE_POS_SHADOWS_DEFINE
	layout(binding = 7) uniform samplerCubeArrayShadow shadowSamplerCube;
#endif

    struct DirectionalLightData
    {
        vec4 colorIntensity;
        vec4 directionShadowMapIndex;
    };

    struct SpotLightData
    {
        vec4 colorIntensity;
        vec4 directionAngle;
        vec4 positionShadowMapIndex;
    };

    struct PointLightData
    {
        vec4 colorIntensity;
        vec4 positionShadowMapIndex;
    };

	//Pointlights and shadow point lights.
	layout(std430, binding = 5) buffer PointLightDataBuffer
    {
        PointLightData pointLightData[];
    };

	//Same as above but for spot lights and shadow spot lights.
	layout(std430, binding = 6) buffer SpotLightDataBuffer
    {
        SpotLightData spotLightData[];
    };

	//Buffer contains all directional light data. First regular, then shadow.
    layout(std430, binding = 7) buffer DirectionalLightDataBuffer
    {
        DirectionalLightData dirLightData[];
    };
//END LIGHT DATA


void main()
{
	vec4 diffuse = vec4(1.0, 1.0, 1.0, 1.0);
	vec3 toCameraDir = normalize(inData.camPos - inData.fragPos);

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

	//Alpha modifier default.
	float alphaModifier = 1.0;

	//Normals are used, but no normalmapping is enabled.
	#if defined(VA_NORMAL_DEF) && !(defined(VA_TANGENT_DEF) && defined(MAT_NORMAL_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF))
	vec3 surfaceNormal = inData.normal;
	#endif

	//default PBR properties.
	float metallic = 0.0;
	float roughness = 1.0;

	//SINGLE MATERIAL
	#ifdef MAT_SINGLE_DEFINE

			//AmbientOcclusion/Height. Requires normalmapping to be active too.
		#if (defined(MAT_OCCLUSION_TEXTURE_DEFINE) || defined(MAT_HEIGHT_TEXTURE_DEFINE))
			#if defined(MAT_HEIGHT_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF) && defined(VA_NORMAL_DEF) && defined(VA_TANGENT_DEF) && defined(MAT_NORMAL_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF)
				//Take multiple samples from different layers, each oriented along the view direction.
				const float numLayers = 50;
				float layerDepth = 1.0 / numLayers;
				float currentLayerDepth = 0.0;
				vec2 P = toCameraDir.xy * 0.2;	//SCALE 
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
			diffuse = vec4(diffuseConstant, 1.0);
		#endif

		//Diffuse texture
		#if defined(MAT_DIFFUSE_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF)
			diffuse = texture2D(diffuseTexture, texCoords);
		#endif

		//Normal texture
		#if defined(VA_NORMAL_DEF) && defined(VA_TANGENT_DEF) && defined(MAT_NORMAL_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF)
			vec3 surfaceNormal = texture2D(normalTexture, texCoords).rgb;
			//Invert the G channel in OpenGL.
			surfaceNormal = surfaceNormal * 2.0 - 1.0;
			surfaceNormal.g *= -1.0;
			surfaceNormal = normalize(surfaceNormal);

			//Transform from tangent to world space.
			surfaceNormal = inData.tbn * surfaceNormal;
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
				vec2 P = toCameraDir.xy * 0.6; 
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

			diffuse = constAttribData[inData.materialID].diffuse;
		#endif

		//Diffuse texture
		#if defined(MAT_DIFFUSE_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF)
			diffuse = texture(materialArray, vec3(texCoords, (numActiveBatchTextures * inData.materialID) + texLayerOffset));
			++texLayerOffset;

		#endif

		//Normal texture
		#if defined(VA_NORMAL_DEF) && defined(VA_TANGENT_DEF) && defined(MAT_NORMAL_TEXTURE_DEFINE) && defined(VA_UVCOORD_DEF)
			vec3 surfaceNormal = texture(materialArray, vec3(texCoords, (numActiveBatchTextures * inData.materialID) + texLayerOffset)).rgb;
			surfaceNormal = surfaceNormal * 2.0 - 1.0;   
			surfaceNormal.g *= -1.0;	//OpenGL inverts the g channel. G = +y and textures are -y format.
			surfaceNormal = normalize(surfaceNormal);

			//Transform from tangent to world space.
			surfaceNormal = inData.tbn * surfaceNormal;

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
		diffuse *= vec4(inData.color, 1.0);
	#endif



//Light calculations

	//Diffuse can always be applied with ambient light.
	vec3 diffuseVec3 = diffuse.xyz;
	vec3 f0 = vec3(0.04); 
    f0 = mix(f0, diffuseVec3, metallic);

	//Total reflected light
    vec3 reflectedLight = vec3(0.0);
    vec3 ambientColor = inData.ambientLight * diffuseVec3;

#if defined(VA_NORMAL_DEF)
    //Point lights.
    for(int i = 0; i < inData.numLights.x; ++i)
    {		
		PointLightData data = pointLightData[i];

		//PBR shading
		vec3 lightColor = data.colorIntensity.xyz * data.colorIntensity.w;
		vec3 toLightDir = data.positionShadowMapIndex.xyz - inData.fragPos.xyz;
        float lightDistance = length(toLightDir);
        toLightDir /= lightDistance;
        reflectedLight += LightReflected(toLightDir, toCameraDir, surfaceNormal, f0, lightColor, lightDistance, metallic, roughness, diffuseVec3);
    }

#ifdef USE_POS_SHADOWS_DEFINE
	//Point lights with shadowmaps.
	for(int i = 0; i < inData.numShadows.x; ++i)
    {
		PointLightData data = pointLightData[int(inData.numLights.x) + i];

		//PBR shading
		vec3 lightColor = data.colorIntensity.xyz * data.colorIntensity.w;
		vec3 toLightDir = data.positionShadowMapIndex.xyz - inData.fragPos.xyz;
        float lightDistance = length(toLightDir);
        toLightDir /= lightDistance;

		//Index into shadow map and check for shadow.
		vec4 shadowCoord = vec4(-toLightDir, data.positionShadowMapIndex.w);
		float visibility = texture(shadowSamplerCube, shadowCoord, lightDistance / inData.farPlane);

		//If nothing is in front of the light for the frag coord, do the lighting.
		if(visibility > 0.0)
		{
			reflectedLight += LightReflected(toLightDir, toCameraDir, surfaceNormal, f0, lightColor, lightDistance, metallic, roughness, diffuseVec3);
		}
    }
	//ENDIF SHADOWS
#endif

	//Spot lights.
    for(int i = 0; i < inData.numLights.y; ++i)
    {
		SpotLightData data = spotLightData[i];

		//PBR shading
		vec3 spotLightDirection = data.directionAngle.xyz;
		vec3 lightColor = data.colorIntensity.xyz * data.colorIntensity.w;
		vec3 toLightDir = data.positionShadowMapIndex.xyz - inData.fragPos.xyz;
        float lightDistance = length(toLightDir);
        toLightDir /= lightDistance;
        

		//Ensure that the position is within the angle.
		float angle = acos(dot(-toLightDir, spotLightDirection));
		if (angle > data.directionAngle.w)
		{
			continue;
		}

		reflectedLight += LightReflected(toLightDir, toCameraDir, surfaceNormal, f0, lightColor, lightDistance, metallic, roughness, diffuseVec3);
    }

#ifdef USE_POS_SHADOWS_DEFINE
	//Spot lights with shadows.
	for(int i = 0; i < inData.numShadows.y; ++i)
    {
		SpotLightData data = spotLightData[int(inData.numLights.y) + i];

		//PBR shading
		vec3 spotLightDirection = data.directionAngle.xyz;
		vec3 lightColor = data.colorIntensity.xyz * data.colorIntensity.w;
		vec3 toLightDir = data.positionShadowMapIndex.xyz - inData.fragPos.xyz;
		vec3 lightDir = -toLightDir;
        float lightDistance = length(toLightDir);
        toLightDir /= lightDistance;
        

		//Ensure that the position is within the angle.
		float angle = acos(dot(-toLightDir, spotLightDirection));
		if (angle > data.directionAngle.w)
		{
			continue;
		}

		//Index into shadow map and check for shadow.
		vec4 shadowCoord = vec4(lightDir, data.positionShadowMapIndex.w);
		float visibility = texture(shadowSamplerCube, shadowCoord, lightDistance / inData.farPlane);

		//If nothing is in front of the light for the frag coord, do the lighting.
		if(visibility > 0.0)
		{
			reflectedLight += LightReflected(toLightDir, toCameraDir, surfaceNormal, f0, lightColor, lightDistance, metallic, roughness, diffuseVec3);
		}
    }
	//ENDIF SHADOWS
#endif

    //Directional lights.
    for(int i = 0; i < inData.numLights.z; ++i)
    {
		DirectionalLightData data = dirLightData[i];

		//PBR shading
		vec3 lightColor = data.colorIntensity.xyz * data.colorIntensity.w;
		vec3 toLightDir = -data.directionShadowMapIndex.xyz;
        float lightDistance = 1.0;
        reflectedLight += LightReflected(toLightDir, toCameraDir, surfaceNormal, f0, lightColor, lightDistance, metallic, roughness, diffuseVec3);
    }

#ifdef USE_DIR_SHADOWS_DEFINE
    //Directional lights with shadows.
    for(int i = 0; i < inData.numShadows.z; ++i)
    {
		DirectionalLightData data = dirLightData[int(inData.numLights.z) + i];

		//PBR shading
		vec3 lightColor = data.colorIntensity.xyz * data.colorIntensity.w;
		vec3 toLightDir = -data.directionShadowMapIndex.xyz;
        float lightDistance = 1.0;

		int shadowMapIndex = int(data.directionShadowMapIndex.w);
		int numShadowCascades = int(inData.numShadowCascades);

		//Calculate the index of the current cascade in the shadow array based on distance from the camera.
		int cascadeBase = int(shadowMapIndex * numShadowCascades);

		//Set to visible so that areas outside the frustum are always lit up correctly.
		float visibility = 1.0;

		//Calculate the cascade offset based on the clip depth. Will be -1 if not in frustum.
		int cascadeOffset = -1;
		for(int q = 0; q < numShadowCascades; ++q)
		{
			if(inData.fragDepth <= cascades[q].depthClipSpace.z)
			{
				cascadeOffset = q;
				break;
			}
		}

		//Inside the cascades, so do the shadow check.
		if(cascadeOffset != -1)
		{
			int cascadeIndex = cascadeBase + cascadeOffset;

			mat4 shadowMatrix = cascades[cascadeIndex].transform;

			//Calculate where in the light frustum the current pixel is.
			vec4 fragLightSpace = shadowMatrix * vec4(inData.fragPos, 1.0);

			//Convert this light space position to 2D texture coordinates by dividing with the projection and then mapping {-1, 1} to {0, 1}.
			//Because this is in light space, X and Y are equal to the UV coordinates on the lights plane. Z is the depth.
			vec3 fragLightCoords = ((fragLightSpace.xyz / fragLightSpace.w) * 0.5) + 0.5;

			vec4 shadowCoords;						
			shadowCoords.xy = fragLightCoords.xy;					//UV coordinates.
			shadowCoords.z = cascadeIndex;							//Depth slice.
			shadowCoords.w = fragLightCoords.z;						//Depth comparison. In the lights frustum, Z is equal to the depth.

			visibility = texture(shadowSampler2D, shadowCoords); 
		}

		if(visibility > 0.0)
		{
			reflectedLight += LightReflected(toLightDir, toCameraDir, surfaceNormal, f0, lightColor, lightDistance, metallic, roughness, diffuseVec3);
		}
    }
	//ENDIF SHADOWS
#endif

#endif
	//Apply AO, alpha and emissive.
	#if defined(MAT_OCCLUSION_TEXTURE_DEFINE)
	ambientColor *= aoModifier;
	#endif

	vec3 outColor = ambientColor + reflectedLight;

	#if defined(MAT_EMISSIVE_CONSTANT_DEFINE) || defined(MAT_EMISSIVE_TEXTURE_DEFINE)
	outColor += emissiveModifier.xyz;
	#endif

	//Reinhardt tonemapping.
	outColor = outColor / (outColor + vec3(1.0));
    outColor = pow(outColor, vec3(1.0/2.2)); 

	//Set output with alpha component.
	gl_FragColor = vec4(outColor, alphaModifier);
}



//PBR function implementations
vec3 LightReflected(vec3 toLightDir, vec3 toCameraDir, vec3 surfaceNormal, vec3 f0, vec3 lightColor, float lightDistance, float metallic, float roughness, vec3 diffuse)
{
	    // calculate per-light radiance
        
        vec3 h = normalize(toCameraDir + toLightDir);
        float attenuation = 1.0 / (lightDistance * lightDistance);
        vec3 radiance = lightColor * attenuation;        
        
        // cook-torrance brdf
        float ndf = DistributionGGX(surfaceNormal, h, roughness);        
        float g = GeometrySmith(surfaceNormal, toCameraDir, toLightDir, roughness);      
        vec3 f = FresnelSchlick(max(dot(h, toCameraDir), 0.0), f0);       
        
        vec3 kS = f;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	  
        
        vec3 numerator = ndf * g * f;
        float denominator = 4.0 * max(dot(surfaceNormal, toCameraDir), 0.0) * max(dot(surfaceNormal, toLightDir), 0.0);
        vec3 specular = numerator / max(denominator, 0.001);  
            
        // add to outgoing radiance Lo
        float nDotL = max(dot(surfaceNormal, toLightDir), 0.0);                
        return vec3(kD * diffuse / 3.1415926536 + specular) * radiance * nDotL; 
}

float DistributionGGX(vec3 surfaceNormal, vec3 h, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(surfaceNormal, h), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.1415926536 * denom * denom;
	
    return num / denom;
}


float GeometrySchlickGGX(float sNormalToCamDot, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = sNormalToCamDot;
    float denom = sNormalToCamDot * (1.0 - k) + k;
	
    return num / denom;
}


float GeometrySmith(vec3 surfaceNormal, vec3 toCameraDir, vec3 toLightDir, float roughness)
{
    float NdotV = max(dot(surfaceNormal, toCameraDir), 0.0);
    float NdotL = max(dot(surfaceNormal, toLightDir), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}


vec3 FresnelSchlick(float cosTheta, vec3 f0)
{
    return f0 + (1.0 - f0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}