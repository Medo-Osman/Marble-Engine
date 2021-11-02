// Globals
static const float PI = 3.14159265359;
static const float LIGHT_SCATTERING_G = -0.5f;
static const int VOLUME_RAY_STEPS = 60;
static const float2 CULL_UV_OFFSET = float2(0.1f, 0.1f);

static const float ditherPattern[4][4] = 
    {
        { 0.0f, 0.5f, 0.125f, 0.625f},
        { 0.75f, 0.22f, 0.875f, 0.375f},
        { 0.1875f, 0.6875f, 0.0625f, 0.5625},
        { 0.9375f, 0.4375f, 0.8125f, 0.3125}
    };


// Vertex
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

cbuffer shadowBuffer : register(b2)
{
    matrix shadowViewMatrix;
    matrix shadowProjectionMatrix;
};

cbuffer sunData : register(b5)
{
    float3 sunDirection;
    float sunIntensity;
    float3 sunColor;
};

// Textures
Texture2D DepthTexture : register(t0);
Texture2D ShadowMap : register(t6);

// Samplers
SamplerState sampState : register(s1); // Imgui uses slot 0, use 1 for default

// Functions

// Mie scaterring approximated with Henyey-Greenstein phase function.
float ComputeScattering(float lightDotView)
{
    float result = 1.0f - LIGHT_SCATTERING_G * LIGHT_SCATTERING_G;
    result /= (4.0f * PI * pow(1.0f + LIGHT_SCATTERING_G * LIGHT_SCATTERING_G - (2.0f * LIGHT_SCATTERING_G) * lightDotView, 1.5f));
    return result;
}

float3 getWorldPos(float2 texCoord)
{
    float z = DepthTexture.Sample(sampState, texCoord).r;
    float x = texCoord.x * 2 - 1;
    float y = (1 - texCoord.y) * 2 - 1;
    float4 ndcPosition = float4(x, y, z, 1.f);
    float4 viewPosition = mul(ndcPosition, projInverseMatrix);
    viewPosition /= viewPosition.w;
    return mul(viewPosition, viewInverseMatrix).xyz;
}

// PS Main
float4 main(PS_IN input, bool isFrontFace : SV_IsFrontFace) : SV_TARGET
{
    float3 WorldPos = getWorldPos(input.TexCoord);
    
    // Ray
    float3 startPosition = cameraPosition.xyz;
    
    float3 rayVector = WorldPos - startPosition;
    float rayLength = length(rayVector);
    float3 rayDirection = rayVector / rayLength;

    // Step
    float stepLength = rayLength / VOLUME_RAY_STEPS;
    float3 step = rayDirection * stepLength;
    float ditherValue = ditherPattern[input.Position.x % 4][input.Position.y % 4];
    
    startPosition += step * ditherValue;
    float3 currentPosition = startPosition;
        
    // Scattering contribution
    float3 accumFog = (float3) 0;
    float3 scatteringForEveryStep = ComputeScattering(dot(rayDirection, sunDirection)) * sunColor * sunIntensity;
    
    // Loop Helpers
    float shadowMapValue = 0.f;
    float4 shadowPos = (float4) 0;
    float2 shadowUV = (float2) 0;
    
    // Ray Marching
    for (int i = 0; i < VOLUME_RAY_STEPS; i++)
    {
        shadowPos = mul(float4(currentPosition, 1.0f), shadowViewMatrix);
        shadowPos = mul(shadowPos, shadowProjectionMatrix);
        shadowPos /= shadowPos.w;
        
        shadowUV = shadowPos.xy * 0.5f + 0.5f;
        shadowUV.y = 1.0f - shadowUV.y;
        
        if ((shadowUV.x > 1.f + CULL_UV_OFFSET.x || shadowUV.x < -CULL_UV_OFFSET.x) || // Outside of shadowmap, break loop
                (shadowUV.y > 1.f + CULL_UV_OFFSET.y || shadowUV.y < -CULL_UV_OFFSET.y))
        {
            uint stepsLeft = (step * (VOLUME_RAY_STEPS - i));
            currentPosition += stepsLeft;
            //accumFog += scatteringForEveryStep * stepsLeft;
            break;
        }
        else
        {
            shadowMapValue = ShadowMap.Sample(sampState, shadowUV).r;
        
            if (shadowMapValue > shadowPos.z)
                accumFog += scatteringForEveryStep;
        
            currentPosition += step;
        }
    }
    // Accumilation
    accumFog /= VOLUME_RAY_STEPS;
        
    return float4(accumFog, 1.f);
}