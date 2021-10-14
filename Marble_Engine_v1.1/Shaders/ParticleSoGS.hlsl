#define PT_EMITTER 0
#define PT_FLARE 1
 
struct Particle
{
    float3  initialPosition : POSITION;
    float3  initialVelocity : VELOCITY;
    float2  size            : SIZE;
    float   age             : AGE;
    uint    type            : TYPE;
};

cbuffer cbPerFrame
{
    float3 eyePosition;
    
    // for when the emit position/direction is varying
    float3 emitPosition;
    float3 emitDirection;
    
    float gameTime;
    float deltaTime;
    matrix viewProj;
};

Texture1D randomTexture     : TEXTURE : register(t0);
SamplerState sampState      : SAMPLER : register(s1); // Imgui uses slot 0, use 1 for default

float3 RandUnitVec3(float offset)
{
    // Use game time plus offset to sample random texture.
    float smapleIndex = (gameTime + offset);
    
    // coordinates in [-1,1]
    float3 v = randomTexture.SampleLevel(sampState, smapleIndex, 0).xyz;
    
    // project onto unit sphere
    return normalize(v);
}

float3 RandVec3(float offset)
{
    // Use game time plus offset to sample random texture.
    float smapleIndex = (gameTime + offset);
    
    // coordinates in [-1,1]
    float3 v = randomTexture.SampleLevel(sampState, smapleIndex, 0).xyz;
    
    return v;
}

[maxvertexcount(2)]
void main(point Particle input[1],
                 inout PointStream<Particle> ptStream)
{
    input[0].age += deltaTime;
    
    if (input[0].type == PT_EMITTER)
    {
        // time to emit a new particle?
        if (input[0].age > 0.005f)
        {
            float3 vRandom = RandUnitVec3(0.0f);
            vRandom.x *= 0.5f;
            vRandom.z *= 0.5f;
            
            Particle particle;
            particle.initialPosition = emitPosition.xyz;
            particle.initialVelocity = 4.0f * vRandom;
            particle.size = float2(3.0f, 3.0f);
            particle.age = 0.0f;
            particle.type = PT_FLARE;
            
            ptStream.Append(particle);
            
            // reset the time to emit
            input[0].age = 0.0f;
        }
        
        // always keep emitters
        ptStream.Append(input[0]);
    }
    else
    {
        // Specify conditions to keep particle; this may vary from system to system.
        if (input[0].age <= 1.0f)
            ptStream.Append(input[0]);
    }
}