// Globals

// Pixel
struct PS_IN
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

// Constant Buffer
cbuffer TonemapCB : register(b0)
{
    // Exposure should be expressed as a relative expsure value (-2, -1, 0, +1, +2 )
    float Exposure;
    
    // Gamma
    float Gamma;
    
    // ACES Filmic parameters
    // See: https://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting/142
    float ACESss; // Shoulder strength
    float ACESls; // Linear strength
    float ACESla; // Linear angle
    float ACESts; // Toe strength
    float ACEStn; // Toe Numerator
    float ACEStd; // Toe denominator
    // Note E/F = Toe angle.
    float LinearWhite;
};

// Textures
Texture2D HDRTexture : register(t0);
Texture2D BloomHDRTexture : register(t1);
Texture2D LuminanceTexture : register(t2); // Size: 1 x 1

// Sampler
SamplerState sampState : register(s1); // Imgui uses slot 0, use 1 for default

// Functions
float3 ACESFilmic(float3 x, float A, float B, float C, float D, float E, float F)
{
    // From: https://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting/142
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - (E / F);
}

// Main
float4 main(PS_IN input) : SV_TARGET
{
    float3 HDR = mad(0.5f, BloomHDRTexture.Sample(sampState, input.TexCoord), HDRTexture.Sample(sampState, input.TexCoord)).rgb;;
    
    // Exposure + (AE * 2 - 1.f)
    HDR *= exp2(Exposure - (((LuminanceTexture.Load(int3(0, 0, 0)).r * 2.f) - 1.f) * 5.f));
    //HDR *= exp2(lerp(Exposure, LuminanceTexture.Load(int3(0, 0, 0)).r, 0.5f));
    //HDR *= exp2(LuminanceTexture.Load(int3(0, 0, 0)).r);
    //HDR *= exp2(Exposure / LuminanceTexture.Load(int3(0, 0, 0)).r);
    
    float3 SDR = ACESFilmic(HDR, ACESss, ACESls, ACESla, ACESts, ACEStn, ACEStd) /
                 ACESFilmic(LinearWhite, ACESss, ACESls, ACESla, ACESts, ACEStn, ACEStd);
    
    return float4(pow(abs(SDR), 1.0f / Gamma), 1);
}