// Globals
#define LIGHT_CAP 40

#define POINT_LIGHT 0
#define SPOT_LIGHT 1
#define DIRECTIONAL_LIGHT 2
static const float PI = 3.14159265359;
static const float MAX_REFLECTION_LOD = 6.0;
static const float EPSILON = 0.000001f;

// - Fog
static const float FOG_DENSITIY = 0.004;
static const float HEIGHT_FACTOR = 0.05f;

// Pixel
struct PS_IN
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

// Constant Buffers
cbuffer CameraCB : register(b0)
{
    float4 cameraPosition;
    matrix viewInverseMatrix;
    matrix projInverseMatrix;
};

struct Light
{
    float4 position;
    float3 direction;
    float intensity;
    float3 color;
    float range;
    float2 spotAngles;
    int type;
    bool enabled;
};
cbuffer lightBuffer : register(b1)
{
    Light lights[LIGHT_CAP];
    uint nrOfLights;
    float enviormentDiffContribution;
    float enviormentSpecContribution;
    bool volumetricSunScattering;
    bool fog;
    bool procederualSky;
};

cbuffer shadowBuffer : register(b2)
{
    matrix shadowViewMatrix;
    matrix shadowProjectionMatrix;
};

cbuffer SkyLightDataCB : register(b5) // Used for Sun and Moon
{
    float3 skyLightDirection;
    float skyLightIntensity;
    float3 skyLightColor;
    int moonOrSun; // 0 = Moon, 1 = Sun
    bool skyLightCastingShadow;
    float3 ambientColor; // used when procederualSky is on
};

// Textures
Texture2D AlbedoMetallicTexture : register(t0);
Texture2D NormalRoughnessTexture : register(t1);
Texture2D EmissiveShadowMaskTexture : register(t2);
Texture2D AmbientOcclusionTexture : register(t3);
Texture2D DepthTexture : register(t4);
Texture2D VolumetricSunTexture : register(t5);

TextureCube IrradianceMap : register(t6);
TextureCube SpecularIBLMap : register(t7);

// Sampler
SamplerState sampState : register(s1); // Imgui uses slot 0, use 1 for default

// Functions
float Pow5(float x)
{
    float xSq = x * x;
    return xSq * xSq * x;
}
float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * Pow5(1.0 - cosTheta);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    float invRoughness = 1.0 - roughness;
    return F0 + (max(float3(invRoughness, invRoughness, invRoughness), F0) - F0) * Pow5(1.0 - cosTheta);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.f);
    float NdotH2 = NdotH * NdotH;
	
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = max(0.000001f, PI * denom * denom);
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.f);
    float NdotL = max(dot(N, L), 0.f);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

float3 lightCommon(float3 N, float3 H, float3 V, float NDotV, float3 L, float3 F0, float roughness, float metallic, float3 radiance, float3 albedo)
{
    // cook-torrance brdf
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    float3 F = fresnelSchlick(max(dot(H, V), 0.f), F0);
        
    float3 kS = F;
    float3 kD = float3(1.f, 1.f, 1.f) - kS;
    kD *= 1.f - metallic;
        
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(NDotV, 0.f) * max(dot(N, L), 0.f);
    float3 specular = numerator / max(denominator, 0.001f);
        
    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.f);
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

float3 getUppsampledVolumetricScattering(float2 texCoord)
{
    int2 screenCoordinates = (int2)(texCoord * float2(1584.f, 861.f));
    int2 downscaledCoordinates = screenCoordinates * .5f;
    float upSampledDepth = DepthTexture.Load(int3(screenCoordinates, 0)).x;

    float3 color = 0.0f.xxx;
    float totalWeight = 0.0f;

    // Select the closest downscaled pixels.

    int xOffset = screenCoordinates.x % 2 == 0 ? -1 : 1;
    int yOffset = screenCoordinates.y % 2 == 0 ? -1 : 1;

    int2 offsets[] =
    {
        int2(0, 0),
        int2(0, yOffset),
        int2(xOffset, 0),
        int2(xOffset, yOffset)
    };

    for (int i = 0; i < 4; i++)
    {
        float3 downscaledColor = VolumetricSunTexture.Load(int3(downscaledCoordinates + offsets[i], 0)).rgb;

        float downscaledDepth = DepthTexture.Load(int3(downscaledCoordinates + offsets[i], 1)).r;

        float currentWeight = 1.0f;
        currentWeight *= max(0.0f, 1.0f - (0.05f) * abs(downscaledDepth - upSampledDepth));

        color += downscaledColor * currentWeight;
        totalWeight += currentWeight;

    }
    
    return color / (totalWeight + EPSILON);
}

// Main
float4 main(PS_IN input) : SV_TARGET
{
    // G-Buffer Textures
    float4 albedoMetallic = AlbedoMetallicTexture.Sample(sampState, input.TexCoord);
    float4 normalRoughness = NormalRoughnessTexture.Sample(sampState, input.TexCoord);
    float4 emissiveShadowMask = EmissiveShadowMaskTexture.Sample(sampState, input.TexCoord);
	
    // Position
    float z = DepthTexture.Sample(sampState, input.TexCoord).r;
    float x = input.TexCoord.x * 2 - 1;
    float y = (1 - input.TexCoord.y) * 2 - 1;
    float4 ndcPosition = float4(x, y, z, 1.f);
    float4 viewPosition = mul(ndcPosition, projInverseMatrix);
    viewPosition /= viewPosition.w;
    float3 worldPosition = mul(viewPosition, viewInverseMatrix).xyz;
    
    // Albedo
    float3 albedo = albedoMetallic.rgb;
	// Normal
    float3 normal = normalize(normalRoughness.xyz);
    // Metallic
    float metallic = albedoMetallic.a;
	// Rough
    float roughness = normalRoughness.a;
    // Emissive
    float3 emissive = emissiveShadowMask.rgb;
    // Ambient Occlusion
    float ambientOcclusion = AmbientOcclusionTexture.Sample(sampState, input.TexCoord).r;
    
    float3 finalColor;
    float3 fogColor = float3(1.f, 0.9f, 1.f);
    
    //if (length(emissive) == 0.f)
    //{
        float3 N = normalize(normal);
        float3 V = normalize(cameraPosition.xyz - worldPosition);
        float3 R = reflect(-V, N);
        float NDotV = dot(N, V);

        // Lighting Setup
        float3 F0 = float3(0.04f, 0.04f, 0.04f);
        F0 = lerp(F0, albedo, metallic);
        uint i;
        float3 Lo = (float3) 0;
        
        // Sun/Moon Light
        float3 direction = normalize(skyLightDirection) * (float)(moonOrSun * 2 - 1); // Flip if Moon
                    
        float3 L = normalize(-direction);
        float3 H = normalize(V + L);
        float3 radiance = skyLightColor * skyLightIntensity;
        
        float3 sunMoonLightContribution = lightCommon(N, H, V, NDotV, L, F0, roughness, metallic, radiance, albedo);
        fogColor = skyLightColor;
        
        if (skyLightCastingShadow)
        {
            sunMoonLightContribution *= emissiveShadowMask.a;
        }
        Lo += sunMoonLightContribution;
        
        // Scene Lights
        for (i = 0; i < nrOfLights; ++i)
        {
            L = lights[i].position.xyz - worldPosition;
            radiance = lights[i].color.xyz * lights[i].intensity;
            
            switch (lights[i].type)
            {
                case DIRECTIONAL_LIGHT:
                {
                    direction = lights[i].direction.xyz;
                    L = normalize(-direction);
                    H = normalize(V + L);
                    
                    Lo += lightCommon(N, H, V, NDotV, L, F0, roughness, metallic, radiance, albedo);
                }
                break;
                case POINT_LIGHT:
                {
                    float3 lightDir = normalize(L);
                    H = normalize(V + lightDir);
                    
                    float lightDistSq = dot(L, L);
                    float invLightDist = rsqrt(lightDistSq);
                    
                    float radiusSq = lights[i].range * lights[i].range;
                    float distanceFalloff = radiusSq * (invLightDist * invLightDist);
                    float attenuation = max(0, distanceFalloff - rsqrt(distanceFalloff));
                    
                    Lo += lightCommon(N, H, V, NDotV, lightDir, F0, roughness, metallic, radiance, albedo) * attenuation;
                }
                break;
                case SPOT_LIGHT:
                {
                    float3 lightDir = normalize(L);
                    H = normalize(V + lightDir);
                    direction = normalize(lights[i].direction.xyz);
                    
                    float lightDistSq = dot(L, L);
                    float invLightDist = rsqrt(lightDistSq);
                    
                    float radiusSq = lights[i].range * lights[i].range;
                    float distanceFalloff = radiusSq * (invLightDist * invLightDist);
                    float attenuation = max(0, distanceFalloff - rsqrt(distanceFalloff));
                    
                    //float minCos = cos((lights[i].spotAngle + 1.f) * 0.5f);
                    //float maxCos = cos(lights[i].spotAngle);
                    //float maxCos = (minCos + 1.f) / 2.f;
                    //float cosAngle = dot(direction, -L);
                    //float spotAttenuation = smoothstep(minCos, maxCos, cosAngle);
                    
                    float coneFalloff = dot(-lightDir, direction);
                    float spotAttenuation = saturate((coneFalloff - lights[i].spotAngles.y) * lights[i].spotAngles.x);
                    
                    Lo += lightCommon(N, H, V, NDotV, lightDir, F0, roughness, metallic, radiance, albedo) * spotAttenuation * attenuation;
                }
                break;
            }
        }
        
        float3 F = FresnelSchlickRoughness(max(NDotV, 0.0), F0, roughness);
        float3 kS = F;
        float3 kD = 1.0 - kS;
        kD *= 1.0 - metallic;

        // Diffuse IBL
        float3 diffuse = (float3)0;
        float lod = roughness * MAX_REFLECTION_LOD;
        if (enviormentDiffContribution > 0.f)
        {
            float3 irradiance;
            if (procederualSky)
                irradiance = ambientColor;
            else
                irradiance = IrradianceMap.Sample(sampState, N).rgb;
            diffuse = irradiance * albedo * enviormentDiffContribution;
            
        }

        // Specular IBL
        float3 specular = (float3)0;
        if (enviormentSpecContribution > 0.f)
        {
            float3 prefilteredColor;
            if (procederualSky)
                prefilteredColor = ambientColor;
            else 
                prefilteredColor = SpecularIBLMap.SampleLevel(sampState, R, lod).rgb;
    
            const float4 c0 = float4(-1, -0.0275, -0.572, 0.022);
            const float4 c1 = float4(1, 0.0425, 1.04, -0.04);
            float4 r = roughness * c0 + c1;
            float a004 = min(r.x * r.x, exp2(-5.28 * NDotV)) * r.x + r.y;
            float2 envBRDF = float2(-1.04, 1.04) * a004 + r.zw;
            specular = prefilteredColor * (F * envBRDF.x + envBRDF.y) * enviormentSpecContribution;
        }
        
        float3 ambient = (kD * diffuse + specular);

        finalColor = (ambient + Lo) * ambientOcclusion;
    //}
    //else
        finalColor += emissive;
    
    // Volumetric Sun Scattering
    if (volumetricSunScattering)
        finalColor += VolumetricSunTexture.Sample(sampState, input.TexCoord).xyz;
        //finalColor += getUppsampledVolumetricScattering(input.TexCoord);

    // Fog
    if (fog)
    {
        float3 fogOrigin = cameraPosition.xyz;
        float3 fogDirection = normalize(worldPosition - fogOrigin);
        float fogDepth = distance(worldPosition, fogOrigin);
        
        float fogFactor =   HEIGHT_FACTOR * exp(-fogOrigin.y * FOG_DENSITIY) *
                            (1.f - exp(-fogDepth * fogDirection.y * FOG_DENSITIY)) / fogDirection.y;
    
        finalColor = lerp(finalColor, fogColor, saturate(fogFactor));
    }
    
    return float4(finalColor, 1.f);
}