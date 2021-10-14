// Globals
static const uint NR_OF_BINS = 256;
static const float EPSILON = 0.00001f;

// Textures
Texture2D InputTex : register(t0);
RWByteAddressBuffer LuminanceHistogramTex : register(u0);
RWTexture2D<float4> LuminanceHistogramImageTex : register(u1);

// Constant Buffer
cbuffer HistogramCB : register(b0)
{
    uint texWidth;
    uint texHeight;
    float minLogLuminance;
    float oneOverLogLuminanceRange;
};

// Shared Memory
groupshared uint histogramShared[NR_OF_BINS];

// Functions
float GetLuminance(float3 color)
{
    return dot(color, float3(0.2127f, 0.7152f, 0.0722f));
}
uint HDRToHistogramBin(float3 color)
{
    float luminance = GetLuminance(color);
    
    if (luminance < EPSILON)
        return 0;
    
    float logLuminance = saturate((log2(luminance) - minLogLuminance) * oneOverLogLuminanceRange);
    return (uint) (logLuminance * 254.0f + 1.0f);
}

// Main
[numthreads(16, 16, 1)]
void main(uint groupIndex : SV_GroupIndex, uint3 dispatchId : SV_DispatchThreadID)
{
    // Reset
    histogramShared[groupIndex] = 0;
    
    // Wait for other thread
    GroupMemoryBarrierWithGroupSync();
    
    if (dispatchId.x < texWidth && dispatchId.y < texHeight)
    {
        float3 hdrColor = InputTex.Load(int3(dispatchId.xy, 0)).rgb;
        uint binIndex = HDRToHistogramBin(hdrColor);
        InterlockedAdd(histogramShared[binIndex], 1);
    }
    
    // Wait for other thread
    GroupMemoryBarrierWithGroupSync();
    
    uint temp = 0;
    LuminanceHistogramTex.InterlockedAdd(groupIndex * 4, histogramShared[groupIndex], temp);
    //InterlockedAdd(LuminanceHistogramImageTex[groupIndex * 4].x, histogramShared[groupIndex]);
}