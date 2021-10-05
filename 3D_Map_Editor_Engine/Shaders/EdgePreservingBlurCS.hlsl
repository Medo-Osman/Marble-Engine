static const int MAX_BLUR_RADIUS = 15;

Texture2D<float4> InputTex : register(t0);
Texture2D<float4> DepthTexture : register(t1);
Texture2D<float4> NormalRoughnessTexture : register(t2);
RWTexture2D<float4> outputTex : register(u0);

cbuffer blurBuffer : register(b0)
{
    matrix projectionMatrix;
    int radius;
    int direction;
    float2 pad;
    float4 weights[MAX_BLUR_RADIUS / 4]; // Saved as float4 because float packing behaviour
}

float ndcDepthToViewDepth(float depth)
{
    return projectionMatrix[3][2] / (depth - projectionMatrix[2][2]);
}

[numthreads(16, 16, 1)]
void main(uint3 gruoupId : SV_GroupID, uint3 groupThredId : SV_GroupThreadID, uint groupIndex : SV_GroupIndex, uint3 dispatchId : SV_DispatchThreadID)
{
    uint2 pixelIndex = uint2(dispatchId.x, dispatchId.y);
    float blurWeights[12] =
    {
        weights[0].x, weights[0].y, weights[0].z, weights[0].w,
        weights[1].x, weights[1].y, weights[1].z, weights[1].w,
        weights[2].x, weights[2].y, weights[2].z, weights[2].w,
    };
    // Positive and Negative directions
    int2 direction2 = int2(1 - direction, direction);
    
    float4 result = blurWeights[radius] * InputTex[pixelIndex];
    float totalWeight = blurWeights[radius];
    
    float3 centerNormal = NormalRoughnessTexture[pixelIndex].xyz;
    float centerDepth = ndcDepthToViewDepth(DepthTexture[pixelIndex].r);
    
    float weight = 0;
    
    for (int i = -radius; i <= radius; ++i)
    {
        if (i == 0)
            continue;
        
        float3 neighborNormal = NormalRoughnessTexture[mad(i, direction2, pixelIndex)].xyz;
        float neighborDepth = ndcDepthToViewDepth(DepthTexture[mad(i, direction2, pixelIndex)].r);
        
        if (dot(neighborNormal, centerNormal) >= 0.8f)
        {
            weight = blurWeights[i + radius];
            result += weight * InputTex[mad(i, direction2, pixelIndex)];
            totalWeight += weight;
        }
    }
    
    outputTex[pixelIndex] = result / totalWeight;
    
    //int uBlurSize = 4;
    //float2 texelSize = 1.0 / float2(1424, 771);
    //float result = 0.0;
    //float2 hlim = float2(float(-uBlurSize) * 0.5 + 0.5);
    //for (int i = 0; i < uBlurSize; ++i)
    //{
    //    for (int j = 0; j < uBlurSize; ++j)
    //    {
    //        float2 offset = (hlim + float2(float(x), float(y))) * texelSize;
    //        result += inputTex[inputTex, pixelIndex + offset].r;
    //    }
    //}
 
    //float fResult = result / float(uBlurSize * uBlurSize);
    
    //outputTex[pixelIndex] = result / totalWeight;
}