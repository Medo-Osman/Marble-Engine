static const int RADIUS = 7;

Texture2D<float4> inputTex : register(t0);
Texture2D<float4> DepthTexture : register(t1);
Texture2D<float4> NormalRoughnessTexture : register(t2);
RWTexture2D<float4> outputTex : register(u0);

cbuffer blurBuffer : register(b0)
{
    matrix projectionMatrix;
    float4 weights[(RADIUS + 1) / 4]; // Saved as float4 because float packing behaviour
    int radius;
    bool direction;
}
//static float4 weights[2] = { float4(0.227027, 0.1945946, 0.1216216, 0.054054), float4(0.016216, 0, 0, 0) };

float ndcDepthToViewDepth(float depth)
{
    return projectionMatrix[3][2] / (depth - projectionMatrix[2][2]);
}

[numthreads(16, 16, 1)]
void main(uint3 gruoupId : SV_GroupID, uint3 groupThredId : SV_GroupThreadID, uint groupIndex : SV_GroupIndex, uint3 dispatchId : SV_DispatchThreadID)
{
    uint2 pixelIndex = uint2(dispatchId.x, dispatchId.y);
    float4 inputColor;
    // Positive and Negative directions
    int2 direction2 = int2(direction - 1, direction);
    
    float4 result = weights[0 >> 2][0 & 3] * inputTex[mad(0, direction2, pixelIndex)];
    float totalWeight = weights[0 >> 2][0 & 3];
    
    float3 centerNormal = NormalRoughnessTexture[pixelIndex].xyz;
    float centerDepth = ndcDepthToViewDepth(DepthTexture[pixelIndex].r);
    
    float weight = 0;
    
    for (int i = -radius; i <= radius; ++i)
    {
        if (i == 0)
            continue;
        
        uint absOfi = (uint) abs(i);
        
        float3 neighborNormal = NormalRoughnessTexture[mad(i, direction2, pixelIndex)].xyz;
        float neighborDepth = ndcDepthToViewDepth(DepthTexture[mad(i, direction2, pixelIndex)].r);
        
        if (dot(neighborNormal, centerNormal) >= 0.8f && abs(neighborDepth - centerDepth) <= 0.2f)
        {
            weight = weights[absOfi >> 2][absOfi & 3];
            result += weight * inputTex[mad(i, direction2, pixelIndex)];
            totalWeight += weight;
        }
    }
    
    outputTex[pixelIndex] = result / totalWeight;
}