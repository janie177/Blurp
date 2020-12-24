
vec3 LightReflected(vec3 toLightDir, vec3 toCameraDir, vec3 surfaceNormal, vec3 f0, vec3 lightColor, float lightDistance, float metallic, float roughness);
float DistributionGGX(vec3 surfaceNormal, vec3 h, float roughness);
float GeometrySchlickGGX(float sNormalToCamDot, float roughness);
float GeometrySmith(vec3 surfaceNormal, vec3 toCameraDir, vec3 toLightDir, float roughness);
vec3 FresnelSchlick(float cosTheta, vec3 f0);

void main()
{
    
    
    vec3 f0 = vec3(0.04); 
    f0 = mix(f0, diffuse, metallic);

    vec3 toCameraDir = normalize(camPos - worldPos);
    vec3 surfaceNormal = ???;

    vec3 reflectedLight(0.0);
    for(light : lights)
    {
        vec3 toLightDir = lightPos - WorldPos;
        float lightDistance = length(toLightDir);
        toLightDir /= lightDistance;

        reflectedLight += LightReflected(toLightDir, toCameraDir, surfaceNormal, f0, lightColor, lightDistance, metallic, roughness);
	}

    //Add together final color.
    vec3 ambientColor = ambientLight * diffuse * ao;
    vec3 color = ambientColor + reflectedLight;
	
    //Tonemapping which should be moved to another render pass. This is reinhardt.
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));  
   
    gl_FragColor = vec4(color, 1.0);
}

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