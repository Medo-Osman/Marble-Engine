struct Particle
{
    float3 position : POSITION;
    float3 velocity : VELOCITY;
    float2 size     : SIZE;
    float  rotation : ROTATION;
    float  age      : AGE;
    uint   type     : TYPE;
    uint   maxId    : MAXID;
};

struct VS_OUT
{
    float3  position    : POSITION;
    float   rotation    : ROTATION;
    float2  size        : SIZE;
    float4  color       : COLOR;
    uint    type        : TYPE;
    uint    id          : ID;
};

cbuffer ParticleStyleCB : register(b2)
{
    float3 colorBegin;
    float colorBias;
    float3 colorEnd;
    float intensity;
    float scaleVariationMax;
    float rotationVariationMax;
    float lifetime;
    bool useNoise;
    float3 emitDirection;
    float emitInterval;
};

VS_OUT main(Particle input, uint vertexID : SV_VERTEXID)
{
    VS_OUT output;
    
    float age = input.age;
    
    output.position = input.position;
    output.rotation = input.rotation;
    
    // fade color with time
    float opacity = 1.0f - smoothstep(0.0f, 1.0f, age / lifetime);
    output.color = float4(lerp(colorEnd, colorBegin, saturate(opacity - colorBias)), opacity);
    
    output.size = input.size;
    output.type = input.type;
    output.id = input.maxId;
    
    return output;
}