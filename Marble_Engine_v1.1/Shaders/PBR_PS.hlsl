// Globals
#define LIGHT_CAP 20

#define POINT_LIGHT 0
#define SPOT_LIGHT 1
#define DIRECTIONAL_LIGHT 2

// Vertex
struct PS_IN
{
    float4 position         : SV_POSITION;
    float3 wPosition        : POSITION;
    float4 shadowPosition   : POSITION1;
    float3 normal           : NORMAL;
    float3 tangent          : TANGENT;
    float3 biTangent        : BITANGENT;
    float2 texCoord         : TEXCOORD;
};

// Constant Buffers
cbuffer materialBuffer : register(b0)
{
    float3 matAlbedo;
    float matMetallic;
    float matRoughness;
    float matEmissiveStrength;
    int materialTextured;
};

struct Light
{
    float4  position;
    float4  direction;
    float4  color;
    float   spotAngle;
    float3  attenuation;
    float   range;
    int     type;
    bool    enabled;
    bool    isCastingShadow;
};

cbuffer lightBuffer : register(b1)
{
    Light lights[LIGHT_CAP];
    uint nrOfLights;
};

struct ColorValues
{
    float4 diffuse;
    float4 specular;
};

cbuffer cameraBuffer : register(b2)
{
    float4 camPosition;
};

cbuffer shadowBuffer : register(b3)
{
    matrix shadowTextureMatrix;
};

// Textures
Texture2D AlbedoTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D MetallicTexture : register(t2);
Texture2D RoughnessTexture : register(t3);
Texture2D EmissiveTexture : register(t4);
Texture2D AmbientOcclusionTexture : register(t5);

Texture2D ShadowMap : TEXTURE : register(t6);
TextureCube IrradianceMap : register(t7);
TextureCube SpecularIBLMap : register(t8);


// Samplers
SamplerState            sampState       : SAMPLER : register(s1); // Imgui uses slot 0, use 1 for default
SamplerComparisonState  shadowSampler   : SAMPLER : register(s2);

// Const Variables
static const float PI = 3.14159265359;

// Helper Functions
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
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
	
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
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
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

float3 computeNormal(PS_IN input)
{
    float3 normalTex = normalize(NormalTexture.Sample(sampState, input.texCoord).xyz * 2 - 1);
    
    float3 normal = input.normal;
    float3 tangent = normalize(input.tangent);
    float3 bitangent = normalize(input.biTangent);
    float3x3 TBNMatrix = float3x3(tangent, bitangent, normal);
    
    return normalize(mul(normalTex, TBNMatrix));
}

// PS Main
float4 main(PS_IN input) : SV_TARGET
{
    // Shadow Mapping
    float2 shadowUV = input.shadowPosition.xy / input.shadowPosition.w * 0.5f + 0.5f;
    shadowUV.y = 1.0f - shadowUV.y;

    float shadowDepth = input.shadowPosition.z / input.shadowPosition.w;

    float shadowFactor = 0;
    [flatten]
    if (shadowDepth < 0.f || shadowDepth > 1.f) // if the pixels depth is beyond the shadow map, skip shadow sampling(Pixel is lit by light)
    {
        shadowFactor = 1.f;
    }
    else
    {
        const int sampleRange = 1;
        [unroll]
        for (int x = -sampleRange; x <= sampleRange; x++)
        {
            [unroll]
            for (int y = -sampleRange; y <= sampleRange; y++)
            {
                shadowFactor += ShadowMap.SampleCmpLevelZero(shadowSampler, shadowUV, shadowDepth, int2(x, y));
            }
        }
        shadowFactor /= ((sampleRange * 2 + 1) * (sampleRange * 2 + 1));
    }
    
    // Albedo
    float3 albedo = matAlbedo;
	// Normal
    float3 Normal = normalize(input.normal.xyz);
    //Metallic
    float metallic = matMetallic;
	//Rough
    float roughness = matRoughness;
    // Ambient Occlusion
    float ao = 1.0;
    // Emissive
    float3 emissive = albedo * matEmissiveStrength;
    
    if (materialTextured)
    {
        albedo *= AlbedoTexture.Sample(sampState, input.texCoord).rgb;
        Normal = computeNormal(input);
        roughness = RoughnessTexture.Sample(sampState, input.texCoord).r;
        metallic = MetallicTexture.Sample(sampState, input.texCoord).r;
        emissive = albedo * matEmissiveStrength;
        ao = AmbientOcclusionTexture.Sample(sampState, input.texCoord).r;
    }
    
    float3 WorldPos = input.wPosition;
    float3 N = normalize(Normal);
    float3 V = normalize(camPosition.xyz - WorldPos);
    float3 R = reflect(-V, N);
    float NDotV = dot(N, V);

    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);
    
    // reflectance equation
    uint i;
    float3 Lo = float3(0.0, 0.0, 0.0);
    for (i = 0; i < nrOfLights; ++i)
    {
        switch (lights[i].type)
        {
            case DIRECTIONAL_LIGHT:
            {
                float3 direction = normalize(lights[i].direction.xyz);
                float intensity = max(dot(N, direction), 0);
                float3 radiance = lights[i].color.xyz; /* * lights[i].Intensity * */;
                
                // cook-torrance brdf
                float NDF = DistributionGGX(N, intensity, roughness);
                float G = GeometrySmith(N, V, -direction, roughness);
                float3 F = fresnelSchlick(max(dot(intensity, V), 0.0), F0);
        
                float3 kS = F;
                float3 kD = float3(1.f, 1.f, 1.f) - kS;
                kD *= 1.0 - metallic;
        
                float3 numerator = NDF * G * F;
                float denominator = 4.0 * max(NDotV, 0.0) * max(dot(N, -direction), 0.0);
                float3 specular = numerator / max(denominator, 0.001);
                specular = float3(0, 0, 0);
        
                // add to outgoing radiance Lo
                float NdotL = max(dot(N, -direction), 0.0);
                float3 directionalLightContribution = (1 * albedo / PI + specular) * radiance * NdotL;
                if (lights[i].isCastingShadow)
                    directionalLightContribution *= shadowFactor;
                Lo += directionalLightContribution;
            }
            break;
            case POINT_LIGHT:
            {
                // calculate per-light radiance
                float3 L = normalize(lights[i].position.xyz - WorldPos);
                float3 H = normalize(V + L);
                float distance = length(lights[i].position.xyz - WorldPos);
        
                float attFactor = lights[i].range.x;
                //float attenuation = 1.0f / (1.0f + attFactor * distance * distance);
                float attenuation = 1.0f - smoothstep(attFactor * 0.75f, attFactor, distance);
        
                //attenuation = distanceFalloff;
                float3 radiance = lights[i].color.xyz * 10 * attenuation;
        
                // cook-torrance brdf
                float NDF   = DistributionGGX(N, H, roughness);
                float G     = GeometrySmith(N, V, L, roughness);
                float3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
                float3 kS = F;
                float3 kD = float3(1.f, 1.f, 1.f) - kS;
                kD *= 1.0 - metallic;
        
                float3 numerator = NDF * G * F;
                float denominator = 4.0 * max(NDotV, 0.0) * max(dot(N, L), 0.0);
                float3 specular = numerator / max(denominator, 0.001);
                
                // add to outgoing radiance Lo
                float NdotL = max(dot(N, L), 0.0);
                Lo += (kD * albedo / PI + specular) * radiance * NdotL;
            }
            break;
            case SPOT_LIGHT:
            {
                //float3 L = normalize(lights[i].position.xyz - WorldPos);
                //float3 H = normalize(V + L);
                //float distance = length(lights[i].position.xyz - WorldPos);
        
                //float attenuation = 1.0f / (1.0f + lights[i].attenuation.x * distance * distance);
                
                //float minCos = cos(lights[i].spotAngle);
                //float maxCos = (minCos + 1.0f) / 2.0f;
                //float cosAngle = dot(lights[i].direction.xyz, -L);
                //float spotIntensity = smoothstep(minCos, maxCos, cosAngle);
        
                //float3 radiance = lights[i].color.xyz * /*lights[i].Intensity * */spotIntensity * attenuation; // * (coneFalloff * distanceFalloff);

                //// cook-torrance brdf
                //float NDF = DistributionGGX(N, H, roughness);
                //float G = GeometrySmith(N, V, L, roughness);
                //float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
                //float3 kS = F;
                //float3 kD = float3(1.0, 1.0, 1.0) - kS;
                //kD *= 1.0 - metallic;
        
                //float3 numerator = NDF * G * F;
                //float denominator = 4.0 * max(NDotV, 0.0) * max(dot(N, L), 0.0);
                //float3 specular = numerator / max(denominator, 0.001);
        
                //// add to outgoing radiance Lo
                //float NdotL = max(dot(N, L), 0.0);
                //Lo += (kD * albedo / PI + specular) * radiance * NdotL;
            }
            break;
        }
    }
    
    float3 F = FresnelSchlickRoughness(max(NDotV, 0.0), F0, roughness);
    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    // Diffuse IBL
    const float MAX_REFLECTION_LOD = 6.0;
    float lod = roughness * MAX_REFLECTION_LOD;
    float3 irradiance = IrradianceMap.Sample(sampState, N).rgb;
    //irradiance = float3(1,1,1);
    float3 diffuse = irradiance * albedo;

    // Specular IBL
    float3 prefilteredColor = SpecularIBLMap.SampleLevel(sampState, R, lod).rgb;
    prefilteredColor *= max(1 - roughness, 0.001);
    //prefilteredColor = float3(0,0,0);
    float2 envBRDF; // = BRDFMap.Sample(LinearRepeatSampler, float2(max(NDotV, 0.0f), roughness)).rg;
    
    const float4 c0 = float4(-1, -0.0275, -0.572, 0.022);
    const float4 c1 = float4(1, 0.0425, 1.04, -0.04);
    float4 r = roughness * c0 + c1;
    float a004 = min(r.x * r.x, exp2(-5.28 * NDotV)) * r.x + r.y;
    envBRDF = float2(-1.04, 1.04) * a004 + r.zw;
    
    float3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
    float3 ambient = ((kD * diffuse) + specular) * ao;
    
    // Final Pixel Color
    //return float4(albedo, 1.f);
    //return float4(1, 0, 0, 1.f);
    return float4(ambient + Lo, 1.f);
}