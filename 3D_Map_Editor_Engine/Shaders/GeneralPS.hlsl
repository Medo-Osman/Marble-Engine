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
    float   pad; // Light Array needs correct byte offset
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

Texture2D shadowMap         : TEXTURE : register(t3);

// Samplers
SamplerState sampState                  : SAMPLER : register(s0);
SamplerComparisonState shadowSampler    : SAMPLER : register(s1);

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
    float2 shadowUV = input.shadowPosition.xy / input.shadowPosition.w * 0.5f + 0.5f;
    shadowUV.y = 1.0f - shadowUV.y;

    float shadowDepth = input.shadowPosition.z / input.shadowPosition.w;

    float shadowFactor = 0.f;
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
                shadowFactor += shadowMap.SampleCmpLevelZero(shadowSampler, shadowUV, shadowDepth, int2(x, y));
            }
        }
        shadowFactor /= ((sampleRange * 2 + 1) * (sampleRange * 2 + 1));
    }
    
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