// Vertex
struct PS_IN
{
    float4 position : SV_Position;
    float4 color    : COLOR;
    float2 texCoord : TEXCOORD;
    uint   id       : ID;
};

// Constant Buffer
cbuffer ParticleCB : register(b0)
{
    matrix viewMatrix;
    matrix projMatrix;
    float3 cameraPosition;
    float gameTime;
    float3 emitPosition;
    float deltaTime;
    float maxParticles;
};

cbuffer ParticleStyleCB : register(b1)
{
    float3 colorBegin;
    float colorBias;
    float3 colorEnd;
    float colorIntensity;
    float scaleVariationMax;
    float rotationVariationMax;
    float lifetime;
    bool useNoise;
    float3 emitDirection;
    float emitInterval;
    bool randomizePosition;
    float3 randomizePosBounds;
    bool randomizeDirection;
    bool dieOnCollition;
    bool fadeInAndOut;
    uint idInterval;
};

// Textures
Texture2D DiffuseTexture : TEXTURE : register(t0);
Texture2D NoiseTexture   : TEXTURE : register(t1);

// Samplers
SamplerState sampState   : SAMPLER : register(s1); // Imgui uses slot 0, use 1 for default

// Functions
float getNoise(float2 uv)
{
    return NoiseTexture.Sample(sampState, uv).x;
}

// PS Main
float4 main(PS_IN input) : SV_TARGET
{
    // Texture Color
    float4 color = DiffuseTexture.Sample(sampState, input.texCoord);
    
    // Noise Samples
    float noise = 1.f;
    if (useNoise)
    {
        float2 scrollDirection = (float2(.1f, -0.4f)) * gameTime * (1.f / input.id); // + (maxParticles / input.id);
    
        noise = getNoise(input.texCoord + scrollDirection);
        float noiseHalfTiling = getNoise((input.texCoord * 0.5f) + scrollDirection);
        float noiseDoubleTiling = getNoise((input.texCoord * 2.0f) + scrollDirection);
        noise *= noiseHalfTiling * noiseDoubleTiling * 2.f;
    }
    
    // Straight Alpha Blend
    float texAlpha = color.a;
    color *= texAlpha * colorIntensity;
    
    // Add Noise to Alpha
    texAlpha = saturate((texAlpha * noise) * 2.f);
    
    // Multiply Alpha
    color *= input.color * (texAlpha * 4.f);
    color.a = input.color.a * texAlpha;
    
    return color;
}