// Globals
static float occlusionRadius = 0.5f;
static float occlusionFadeStart = 0.2f;
static float occlusionFadeEnd = 2.0f;
static float surfaceEpsilon = 0.1f;
static const int SAMPLE_COUNT = 14;
static float nearZ = 0.1f;
static float farZ = 1000.f;

struct PS_IN
{
    float4 Position : SV_POSITION;
    float4 PositionV : POSITION;
    float2 TexCoord : TEXCOORD1;
};

cbuffer cameraBuffer : register(b0)
{
    matrix viewToTexMatrix; 
    matrix projectionMatrix;
    matrix viewMatrix;
};

cbuffer HBAOData : register(b1)
{
    float4 sampleDirections[SAMPLE_COUNT];
    float2 ditherScale;
};

Texture2D DepthTexture : register(t0);
Texture2D NormalRoughnessTexture : register(t1);
Texture2D<float3> RandomTexture : register(t2);

SamplerState depthNormalSampler: register(s2);
SamplerState randomSampler : register(s3);

float ndcDepthToViewDepth(float depth)
{
    return projectionMatrix[3][2] / (depth - projectionMatrix[2][2]);
}

float occlusionFunction(float distZ)
{
    float occlusion = 0.f;
    if (distZ > surfaceEpsilon)
    {
        float fadeLength = occlusionFadeEnd - occlusionFadeStart;
        occlusion = saturate((occlusionFadeEnd - distZ) / fadeLength);
    }
    return occlusion;
}

float4 main(PS_IN input) : SV_TARGET
{
    // Smaple Normal
    float3 n = normalize(NormalRoughnessTexture.SampleLevel(depthNormalSampler, input.TexCoord, 0.0f).xyz);
    n = mul(n, (float3x3) viewMatrix); // Convert from World to View
    
    // Sample Depth texcture to get Position
    float pz = ndcDepthToViewDepth(DepthTexture.SampleLevel(depthNormalSampler, input.TexCoord, 0.0f).r);
    float3 p = (pz / input.PositionV.z) * input.PositionV;
    
    // Get Random Vector
    //float3 randVec = RandomTexture.SampleLevel(randomSampler, input.TexCoord * 4.0, 0).rgb * 2.f - 1.0f; // Random Vector
    float3 randVec = RandomTexture.SampleLevel(randomSampler, input.TexCoord * ditherScale, 0).rgb; // Dither Texture
    
    float occlusionSum = 0.0f;
    
    [unroll]
    for (int i = 0; i < SAMPLE_COUNT; ++i)
    {
        float3 offset = reflect(sampleDirections[i].xyz, randVec);
    
        float flip = sign(dot(offset, n));
        
        float3 q = p + flip * occlusionRadius * offset;
        
        float4 projQ = mul(float4(q, 1.0f), viewToTexMatrix);
        projQ /= projQ.w;

        float rz = ndcDepthToViewDepth(DepthTexture.SampleLevel(depthNormalSampler, projQ.xy, 0.0f).r);
        float3 r = (rz / q.z) * q;
        
        float distZ = p.z - r.z;
        float dp = max(dot(n, normalize(r - p)), 0.0f);
        float occlusion = dp * occlusionFunction(distZ);
        
        occlusionSum += occlusion;
    }
    
    occlusionSum /= SAMPLE_COUNT;
    
    float access = 1.0f - occlusionSum;
    float ao = saturate(pow(access, 2.f));
    return float4(ao, ao, ao, 1.f);
}