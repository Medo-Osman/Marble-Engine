// Globals
static const uint MAX_BLOOM_MIP = 6;

// Textures
Texture2D<half4> InputTex : register(t0);
Texture2D<half4> LastResultTex : register(t1);
RWTexture2D<half4> outputTex : register(u0);

// Sampler
SamplerState sampState : register(s0);

// Constant Buffer
cbuffer UpsampleCB : register(b0)
{
    float4 mipData; // x = 1 / width, y = 1 / height, z = mip level, w is not used
};

// Functions
half4 boxFilter(half4 uvDirection)
{   
    half4 result =  InputTex.SampleLevel(sampState, uvDirection.xy, mipData.z);
    result +=       InputTex.SampleLevel(sampState, uvDirection.zy, mipData.z);
    result +=       InputTex.SampleLevel(sampState, uvDirection.xw, mipData.z);
    result +=       InputTex.SampleLevel(sampState, uvDirection.zw, mipData.z);
    return result * (1.0 / 4.0);
}

half4 tentFilter(half4 uvDirection, half2 center)
{
    half4 result =  InputTex.SampleLevel(sampState, center - uvDirection.xy, mipData.z);
    result +=       InputTex.SampleLevel(sampState, center - uvDirection.wy, mipData.z) * 2.0;
    result +=       InputTex.SampleLevel(sampState, center - uvDirection.zy, mipData.z);
    
    result +=       InputTex.SampleLevel(sampState, center + uvDirection.zw, mipData.z) * 2.0;
    result +=       InputTex.SampleLevel(sampState, center                 , mipData.z) * 4.0;
    result +=       InputTex.SampleLevel(sampState, center + uvDirection.xw, mipData.z) * 2.0;
    
    result +=       InputTex.SampleLevel(sampState, center + uvDirection.zy, mipData.z);
    result +=       InputTex.SampleLevel(sampState, center + uvDirection.wy, mipData.z) * 2.0;
    result +=       InputTex.SampleLevel(sampState, center + uvDirection.xy, mipData.z);

    return result * (1.0 / 16.0);
}

[numthreads(16, 16, 1)]
void main(uint3 gruoupId : SV_GroupID, uint3 groupThredId : SV_GroupThreadID, uint groupIndex : SV_GroupIndex, uint3 dispatchId : SV_DispatchThreadID)
{
    // Pixel Position
    int2 outputPixel = uint2(dispatchId.x, dispatchId.y);
    half2 inputIndex = outputPixel * 2;
    
    // Offsetet UV
    half2 uvTL = (inputIndex * (mipData.xy)); // Top-Left / Center
    half2 uvBR = uvTL + (mipData.xy);                  // Bottom-Right
    half2 uvTR = half2(uvTL.x, uvBR.y);               // Top-Right
    half2 uvBL = half2(uvBR.x, uvTL.y);               // Bottom-Left
    
    // Filter
    // - Box
    //half4 uvDirection = half4(uvTL.x, uvTL.y, uvBR.x, uvBR.y);
    //half4 result = boxFilter(uvDirection);
    
    // - Tent
    half4 uvDirection = half4(mipData.x, mipData.y, -mipData.x, -mipData.y);
    half4 result = tentFilter(uvDirection, uvTL + (mipData.xy / 1.5f));
    
    // Get Last Result from Higher Mip
    if (mipData.z < MAX_BLOOM_MIP) // Skips first pass because no lower mip exists
        result += LastResultTex.SampleLevel(sampState, (uvTL), mipData.z + 1);
    
    // Add last and current result to output
    outputTex[outputPixel] = result;
    //outputTex[outputPixel] = InputTex.SampleLevel(sampState, (half2(inputIndex) * (mipData.xy)), mipData.z);;
}