// Globals
static const uint NR_OF_BINS = 256;
static const float EPSILON = 0.00001f;

// Textures
RWByteAddressBuffer LuminanceHistogramTex : register(u0);
RWTexture2D<float> LuminanceOutputTex : register(u1);

// Constant Buffer
cbuffer AveragingCB : register(b0)
{
    uint pixelCount;
    float minLogLuminance;
    float logLuminanceRange;
    float deltaTime;
    float tau;
};

// Shared Memory
groupshared float histogramShared[NR_OF_BINS];

// Main
[numthreads(16, 16, 1)]
void main(uint groupIndex : SV_GroupIndex)
{
    float countForThisBin = (float) LuminanceHistogramTex.Load(groupIndex * 4);
    histogramShared[groupIndex] = countForThisBin * (float) groupIndex;
    
    GroupMemoryBarrierWithGroupSync();
    
    [unroll]
    for (uint histogramSampleIndex = (NR_OF_BINS >> 1); histogramSampleIndex > 0; histogramSampleIndex >>= 1)
    {
        if (groupIndex < histogramSampleIndex)
            histogramShared[groupIndex] += histogramShared[groupIndex + histogramSampleIndex];

        GroupMemoryBarrierWithGroupSync();
    }
    
    if (groupIndex == 0)
    {
        float weightedLogAverage = (histogramShared[0].x / max((float) pixelCount - countForThisBin, 1.0)) - 1.0;
        float weightedAverageLuminance = exp2(((weightedLogAverage / 254.0) * logLuminanceRange) + minLogLuminance);
        float luminanceLastFrame = LuminanceOutputTex[uint2(0, 0)];
        float adaptedLuminance = luminanceLastFrame + (weightedAverageLuminance - luminanceLastFrame) * (1 - exp(-deltaTime * tau));
        LuminanceOutputTex[uint2(0, 0)] = adaptedLuminance;
    }
}