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
    float4  matEmissive;
    float4  matAmbient;
    float4  matDiffuse;
    float4  matSpecular;
    float   matShininess;
    
    bool    diffTextureExist;
    bool    specTextureExist;
    bool    normTextureExist;
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
Texture2D diffuseTexture    : TEXTURE : register(t0);
Texture2D specularTexture   : TEXTURE : register(t1);
Texture2D normalTexture     : TEXTURE : register(t2);

//Texture2D shadowMap         : TEXTURE : register(t6);

// Samplers
SamplerState            sampState           : SAMPLER : register(s1); // Imgui uses slot 0, use 1 for default
SamplerComparisonState  shadowSampler       : SAMPLER : register(s2);

// Light Calculation Components
float4 doSpecular(float3 color, float3 posToCameraVector, float3 direction, float3 normal)
{
    float3 reflectVector = normalize(reflect(-direction, normal));
    float RdotV = max(dot(reflectVector, posToCameraVector), 0);
 
    return float4(color * pow(RdotV, matShininess), 1.f);
}
float4 doDiffuse(float3 color, float3 direction, float3 normal)
{
    float intensity = max(dot(normal, direction), 0);
    return float4(color * intensity, 1.f);
}

// Light Type Calculation
ColorValues directionalLightCalc(Light dLight, float3 posToCameraVector, float3 normal)
{
    ColorValues finalValues;
    float3 direction = normalize(dLight.direction.xyz);
    finalValues.diffuse = doDiffuse(dLight.color.rgb, -direction, normal);
    finalValues.specular = doSpecular(dLight.color.rgb, posToCameraVector, -direction, normal);
 
    return finalValues;
}
ColorValues pointLightCalc(float3 position, Light pLight, float3 pixelToCameraVector, float3 normal)
{
    ColorValues finalValues;
    finalValues.diffuse = float4(0.f, 0.f, 0.f, 1.f);
    finalValues.specular = float4(0.f, 0.f, 0.f, 1.f);
   
    // Attenuation
    float distanceToLight = distance(pLight.position.xyz, position);
    [flatten]
    if (distanceToLight > pLight.range)
        return finalValues;
    
    //float attFactor = 1 / (pLight.attenuation.x + pLight.attenuation.y * distanceToLight + pLight.attenuation.z * pow(distanceToLight, 2));
    float attFactor = 1.0f - smoothstep(pLight.range * 0.75f, pLight.range, distanceToLight);
    
    // Diffuse
    float3 posToLightVector = normalize(pLight.position.xyz - position);
    finalValues.diffuse = doDiffuse(pLight.color.rgb, posToLightVector, normal) * attFactor;
    
    // Specular, Phong model
    finalValues.specular = doSpecular(pLight.color.rgb, pixelToCameraVector, posToLightVector, normal) * attFactor;
    
    return finalValues;
}

static const uint PCSS_SampleCount = 32;
static const float PCSS_SampleRadius = 1.0f;
static const float2 PCSS_Samples[PCSS_SampleCount] =
{
    float2(0.06407013, 0.05409927),
	float2(0.7366577, 0.5789394),
	float2(-0.6270542, -0.5320278),
	float2(-0.4096107, 0.8411095),
	float2(0.6849564, -0.4990818),
	float2(-0.874181, -0.04579735),
	float2(0.9989998, 0.0009880066),
	float2(-0.004920578, -0.9151649),
	float2(0.1805763, 0.9747483),
	float2(-0.2138451, 0.2635818),
	float2(0.109845, 0.3884785),
	float2(0.06876755, -0.3581074),
	float2(0.374073, -0.7661266),
	float2(0.3079132, -0.1216763),
	float2(-0.3794335, -0.8271583),
	float2(-0.203878, -0.07715034),
	float2(0.5912697, 0.1469799),
	float2(-0.88069, 0.3031784),
	float2(0.5040108, 0.8283722),
	float2(-0.5844124, 0.5494877),
	float2(0.6017799, -0.1726654),
	float2(-0.5554981, 0.1559997),
	float2(-0.3016369, -0.3900928),
	float2(-0.5550632, -0.1723762),
	float2(0.925029, 0.2995041),
	float2(-0.2473137, 0.5538505),
	float2(0.9183037, -0.2862392),
	float2(0.2469421, 0.6718712),
	float2(0.3916397, -0.4328209),
	float2(-0.03576927, -0.6220032),
	float2(-0.04661255, 0.7995201),
	float2(0.4402924, 0.3640312),
};

inline float Random(float2 co)
{
    return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);
}

inline float2 PCSS_Rotate(float2 pos, float2 rotationTrig)
{
    return float2(pos.x * rotationTrig.x - pos.y * rotationTrig.y, pos.y * rotationTrig.x + pos.x * rotationTrig.y);
}

// Input parameters are:
// tex - Input shadow depth map
// state - Sampler state for shadow depth map
// projCoord - holds projection UV coordinates, and depth for receiver (~further compared against shadow depth map)
// searchUV - input size for blocker search
// rotationTrig - input parameter for random rotation of kernel samples
inline float2 PCSS_BlockerDistance(Texture2D<float4> tex, SamplerState state, float3 projCoord, float searchUV, float2 rotationTrig)
{
	// Perform N samples with pre-defined offset and random rotation, scale by input search size
    int blockers = 0;
    float avgBlockerDistance = 0.0f;
    for (int i = 0; i < (int) PCSS_SampleCount; i++)
    {
		// Calculate sample offset (technically anything can be used here - standard NxN kernel, random samples with scale, etc.)
        float2 offset = PCSS_Samples[i] * searchUV;
        offset = PCSS_Rotate(offset, rotationTrig);

		// Compare given sample depth with receiver depth, if it puts receiver into shadow, this sample is a blocker
        float z = tex.SampleLevel(state, projCoord.xy + offset, 0.0f).x;
        if (z < projCoord.z)
        {
            blockers++;
            avgBlockerDistance += z;
        }
    }

	// Calculate average blocker depth
    avgBlockerDistance /= blockers;

	// To solve cases where there are no blockers - we output 2 values - average blocker depth and no. of blockers
    return float2(avgBlockerDistance, (float) blockers);
}

inline float PCSS_PCFFilter(Texture2D<float4> tex, SamplerState state, float3 projCoord, float filterRadiusUV, float penumbra, float2 rotationTrig, float2 grad)
{
    float sum = 0.0f;
    for (int i = 0; i < (int) PCSS_SampleCount; i++)
    {
        float2 offset = PCSS_Samples[i] * filterRadiusUV;
        offset = PCSS_Rotate(offset, rotationTrig);

		/*float4 tmp = tex.Gather(state, projCoord.xy + offset);
		tmp.x = tmp.x < projCoord.z ? 0.0f : 1.0f;
		tmp.y = tmp.y < projCoord.z ? 0.0f : 1.0f;
		tmp.z = tmp.z < projCoord.z ? 0.0f : 1.0f;
		tmp.w = tmp.w < projCoord.z ? 0.0f : 1.0f;
		sum += lerp(lerp(tmp.w, tmp.z, grad.x), lerp(tmp.x, tmp.y, grad.x), grad.y);*/
        sum += tex.SampleLevel(state, projCoord.xy + offset, 0.0f).x < projCoord.z ? 0.0f : 1.0f;
    }
    sum /= (float) PCSS_SampleCount;
    return sum;
}

float ShadowMapPCSS(Texture2D<float4> tex, SamplerState state, float3 projCoord, float resolution, float pixelSize, float lightSize)
{
    float2 uv = projCoord.xy;
    float depth = projCoord.z;
    float zAwareDepth = depth;

    float rotationAngle = Random(projCoord.xy) * 3.1415926;
    float2 rotationTrig = float2(cos(rotationAngle), sin(rotationAngle));

    float searchSize = lightSize / resolution * 100.0 * saturate(zAwareDepth - .02) / zAwareDepth;
    float2 blockerInfo = PCSS_BlockerDistance(tex, state, projCoord, searchSize, rotationTrig);

    if (blockerInfo.y < 1.0)
    {
        return 1.0f;
    }
    else
    {
        float penumbra = max(zAwareDepth - blockerInfo.x, 0.0);
        float filterRadiusUV = penumbra * lightSize / resolution * 100.0;

        float2 grad = frac(projCoord.xy * resolution + 0.5f);

        float shadow = PCSS_PCFFilter(tex, state, projCoord, filterRadiusUV, penumbra, rotationTrig, grad);

        return shadow;
    }
}

// PS Main
float4 main(PS_IN input) : SV_TARGET
{
    float4 emissiveColor = matEmissive;
    float4 ambientColor = matAmbient;
    float4 diffuseColor = 0;
    float4 specularColor = 0;
    
    // Position to Camera
    float3 posToCameraVector = normalize(camPosition.xyz - input.wPosition.xyz);
    
    // Normal Map Calculation
    [flatten]
    if (normTextureExist)
    {
        float3 normalTex = normalTexture.Sample(sampState, input.texCoord).xyz;
        normalTex = normalize(normalTex);
        normalTex.x = normalTex.x * 2.f - 1.f;
        normalTex.y = -normalTex.y * 2.f + 1.f;
        normalTex.z = normalTex.z;
        
        float3x3 TBNMatrix = float3x3(input.tangent, input.biTangent, input.normal);
        input.normal = normalize(mul(normalTex, TBNMatrix));
    }
    
    // Shadow Mapping
    float shadowFactor = 1.f;
    //float shadowFactor = ShadowMapPCSS(shadowMap, sampState, input.shadowPosition.xyz, 3072.f, 1.f, 10.f);
    //float2 shadowUV = input.shadowPosition.xy / input.shadowPosition.w * 0.5f + 0.5f;
    //shadowUV.y = 1.0f - shadowUV.y;

    //float shadowDepth = input.shadowPosition.z / input.shadowPosition.w;

    //float shadowFactor = 0.f;
    //[flatten]
    //if (shadowDepth < 0.f || shadowDepth > 1.f) // if the pixels depth is beyond the shadow map, skip shadow sampling(Pixel is lit by light)
    //{
    //    shadowFactor = 1.f;
    //}
    //else
    //{
    //    const int sampleRange = 1;
    //    [unroll]
    //    for (int x = -sampleRange; x <= sampleRange; x++)
    //    {
    //        [unroll]
    //        for (int y = -sampleRange; y <= sampleRange; y++)
    //        {
    //            shadowFactor += shadowMap.SampleCmpLevelZero(shadowSampler, shadowUV, shadowDepth, int2(x, y));
    //        }
    //    }
    //    shadowFactor /= ((sampleRange * 2 + 1) * (sampleRange * 2 + 1));
    //}
    
    // Light Calculation
    ColorValues finalLightValues;
    finalLightValues.diffuse = float4(0.f, 0.f, 0.f, 1.f);
    finalLightValues.specular = float4(0.f, 0.f, 0.f, 1.f);
    
    for (int i = 0; i < nrOfLights; ++i)
    {
        ColorValues currentLightValues;
        switch (lights[i].type)
        {
            case DIRECTIONAL_LIGHT:
            {
                currentLightValues = directionalLightCalc(lights[i], posToCameraVector, input.normal);
                
                currentLightValues.diffuse = currentLightValues.diffuse * shadowFactor;
                currentLightValues.specular = currentLightValues.specular * shadowFactor;
            }
            break;
            case POINT_LIGHT:
            {
                currentLightValues = pointLightCalc(input.wPosition, lights[i], posToCameraVector, input.normal);
            }
            break;
            case SPOT_LIGHT:
            {
                //currentLightValues = spotLightCalc(lights[i], pixelToCameraVector, input.normal);
            }
            break;
        }
        finalLightValues.diffuse += currentLightValues.diffuse;
        finalLightValues.specular += currentLightValues.specular;
    }
    
    // Diffuse Color
    diffuseColor = matDiffuse;
    //diffuseColor *= 1 + diffuseTexture.Sample(sampState, input.texCoord) * diffTextureExist + diffTextureExist * -1;
    if (diffTextureExist)
    {
        ambientColor *= diffuseTexture.Sample(sampState, input.texCoord);
        diffuseColor *= diffuseTexture.Sample(sampState, input.texCoord);
    }

    diffuseColor *= float4(finalLightValues.diffuse.xyz, 1.f); // Add Light Diffuse Result
    
    // Specular Color
    specularColor = matSpecular;
    if (specTextureExist)
        specularColor *= specularTexture.Sample(sampState, input.texCoord);
    
    specularColor *= finalLightValues.specular; // Add Light Specular Result
    
    // Final Pixel Color
    return float4((ambientColor + emissiveColor + diffuseColor + specularColor).rgb, diffuseColor.a);
}