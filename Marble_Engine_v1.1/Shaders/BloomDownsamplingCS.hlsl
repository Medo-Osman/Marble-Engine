// Textures
Texture2D<half4> InputTex : register(t0);
Texture2D<half4> LuminanceTex : register(t1);
RWTexture2D<half4> outputTex : register(u0);
RWTexture2D<half4> outputBufferTex : register(u1);

// Sampler
SamplerState sampState : register(s0);

// Constant Buffer
cbuffer DownsampleCB : register(b0)
{
    float4 threshold;   // x = threshold, y = threshold - knee, z = knee * 2, w = 0.25 / knee
    float4 mipData;     // x = 1 / width, y = 1 / height, z = mip level, w = exposure
};

// Functions
half4 boxFilter(half2 uv, half2 stepSize)
{
    half4 direction = half4(0, 0, stepSize.x, stepSize.y);
    
    half4 result = InputTex.SampleLevel(sampState, uv + direction.xy, mipData.z);
    result += InputTex.SampleLevel(sampState, uv + direction.zy, mipData.z);
    result += InputTex.SampleLevel(sampState, uv + direction.xw, mipData.z);
    result += InputTex.SampleLevel(sampState, uv + direction.zw, mipData.z);
    return half4(result.rgb * (1.0 / 4.0), 1.f);
}

half4 box13Filter(half2 uv, half2 stepSize)
{
    half4 A = InputTex.SampleLevel(sampState, uv + stepSize * half2(-1.0, -1.0),  mipData.z);
    half4 B = InputTex.SampleLevel(sampState, uv + stepSize * half2(0.0, -1.0),   mipData.z);
    half4 C = InputTex.SampleLevel(sampState, uv + stepSize * half2(1.0, -1.0),   mipData.z);
    half4 D = InputTex.SampleLevel(sampState, uv + stepSize * half2(-0.5, -0.5),  mipData.z);
    half4 E = InputTex.SampleLevel(sampState, uv + stepSize * half2(0.5, -0.5),   mipData.z);
    half4 F = InputTex.SampleLevel(sampState, uv + stepSize * half2(-1.0, 0.0),   mipData.z);
    half4 G = InputTex.SampleLevel(sampState, uv,                                  mipData.z);
    half4 H = InputTex.SampleLevel(sampState, uv + stepSize * half2(1.0, 0.0),    mipData.z);
    half4 I = InputTex.SampleLevel(sampState, uv + stepSize * half2(-0.5, 0.5),   mipData.z);
    half4 J = InputTex.SampleLevel(sampState, uv + stepSize * half2(0.5, 0.5),    mipData.z);
    half4 K = InputTex.SampleLevel(sampState, uv + stepSize * half2(-1.0, 1.0),   mipData.z);
    half4 L = InputTex.SampleLevel(sampState, uv + stepSize * half2(0.0, 1.0),    mipData.z);
    half4 M = InputTex.SampleLevel(sampState, uv + stepSize * half2(1.0, 1.0),    mipData.z);

    half2 div = (1.0 / 4.0) * half2(0.5, 0.125);

    half4 result = (D + E + I + J) * div.x;
    result += (A + B + G + F) * div.y;
    result += (B + C + H + G) * div.y;
    result += (F + G + L + K) * div.y;
    result += (G + H + M + L) * div.y;

    return result;
}

[numthreads(8, 8, 1)]
void main(uint3 gruoupId : SV_GroupID, uint3 groupThredId : SV_GroupThreadID, uint groupIndex : SV_GroupIndex, uint3 dispatchId : SV_DispatchThreadID)
{
    // Pixel Index
    uint2 outPixelIndex = uint2(dispatchId.x, dispatchId.y);
    half2 inputIndex = outPixelIndex * 2;
    
    // Filter
    // - Box
    //half4 result = boxFilter((inputIndex * mipData.xy) + (mipData.xy * 1.5f), mipData.xy * 1.5f);
    
    // - 13 Tap
    half4 result = box13Filter((inputIndex * mipData.xy) + (mipData.xy * 2.f), mipData.xy * 2.f);
    
    // Threshold Test
    if (mipData.z < 1) // Only test on first mip
    {
        // Apply Exposure
        float autoExpsodure = LuminanceTex.Load(int3(0, 0, 0)).r;
        result *= lerp(mipData.w + 1.f, 1.f, autoExpsodure);
        
        // Curve
        half br = max(max(result.r, result.g), result.b);
        
        half rq = clamp(br - threshold.y, 0.0, threshold.z);
        rq = threshold.w * rq * rq;
        
        result.rgb *= max(rq, br - threshold.x) / max(br, 0.0001);
        
        // Linear
        //half intensityTest = (half) (length(result.rgb) > threshold.x);
        //result.rgb *= intensityTest;
    }
    
    // Output
    outputTex[outPixelIndex] = result;
    outputBufferTex[outPixelIndex] = result;
}