#define PT_EMITTER 0
#define PT_PARTICLE 1
#define COLLISION_THICKNESS 0.5f
#define COLLISION_OFFSET 0.2f
#define RESTITUTION 0.9f

struct Particle
{
    float3  position : POSITION;
    float3  velocity : VELOCITY;
    float2  size     : SIZE;
    float   rotation : ROTATION;
    float   age      : AGE;
    uint    type     : TYPE;
    uint    id       : MAXID;
};

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

Texture1D RandomTexture : register(t0);
Texture2D DepthTexture : register(t1);
Texture2D NormalTexture : register(t2); // Contains World Normals in XYZ and ROUGNESS in alpha(not used)

SamplerState sampState : register(s0);

float ndcDepthToViewDepth(float depth)
{
    return projMatrix[3][2] / (depth - projMatrix[2][2]);
}

float3 RandVec3(float offset)
{
    // Use game time plus offset to sample random texture.
    float sampleIndex = (gameTime + offset);
    
    // coordinates in [-1,1]
    float3 v = RandomTexture.SampleLevel(sampState, sampleIndex, 0).xyz;
    
    return v;
}

[maxvertexcount(2)]
void main(point Particle input[1], inout PointStream<Particle> particleStream)
{
    input[0].age += deltaTime;
    
    if (input[0].type == PT_EMITTER)
    {   
        // time to emit a new particle?
        if (input[0].age > emitInterval)
        {   
            float3 vRand = RandVec3(0.f);
            float rand = vRand.x;
            vRand = normalize(vRand);
            
            vRand.x *= 0.5f;
            vRand.z *= 0.5f;
            
            Particle particle;
            
            // ID
            input[0].id++; // Increment Emitter maxID
            input[0].id = input[0].id > idInterval ? 1 : input[0].id; // cap to 5
            
            particle.id = input[0].id;
            
            // Position
            particle.position = emitPosition;
            if (randomizePosition)
            {
                float3 randPos = float3(
                                        RandVec3((float) particle.id + vRand.x).x,
                                        RandVec3((float) particle.id + vRand.y).x,
                                        RandVec3((float) particle.id + vRand.z).x);
                particle.position += (randomizePosBounds * randPos);
            }
            
            // Velocity
            particle.velocity = 0.08f * vRand;
            if (randomizeDirection)
                particle.velocity *= 5.f;
            
            particle.size = input[0].size * (1 + (rand * scaleVariationMax));
            particle.rotation = input[0].rotation + (rand * rotationVariationMax);
            particle.age = 0.0f;
            particle.type = PT_PARTICLE;
            
            particleStream.Append(particle);
            
            // reset the time to emit
            input[0].age = 0.0f;
        }
        
        // always keep emitters
        particleStream.Append(input[0]);
    }
    else
    {
        // Should die or not
        if (input[0].age <= lifetime)
        {
            bool collided = false;
            
            // Collision
            float3 acceleration = emitDirection;
            if (randomizeDirection)
                acceleration = input[0].velocity;
            
            float3 newPosition = (deltaTime * ((input[0].age * acceleration) + input[0].velocity)) + input[0].position;
            
            float4 viewPos = mul(float4(newPosition, 1.f), viewMatrix);
            float4 screenSpacePos = mul(viewPos, projMatrix);
            screenSpacePos.xyz /= screenSpacePos.w;
            
            // Is on screen, with offset
            if (screenSpacePos.x > -1.f &&
                screenSpacePos.x <  1.f &&
                screenSpacePos.y > -1.f &&
                screenSpacePos.y <  1.f)
            {
                float2 uv = screenSpacePos.xy * .5f + .5f;
                uv.y = 1.f - uv.y;
                float sceneDepth = ndcDepthToViewDepth(DepthTexture.SampleLevel(sampState, uv, 0).x) + input[0].size.x; // only correct if size is uniform
                
                if ((viewPos.z > sceneDepth - COLLISION_OFFSET) &&
                    (viewPos.z < sceneDepth - COLLISION_OFFSET + COLLISION_THICKNESS))
                {
                    if (!dieOnCollition)
                    {
                        float3 wNormal = NormalTexture.SampleLevel(sampState, uv, 0).xyz;
                        input[0].velocity = reflect(input[0].velocity, wNormal) * RESTITUTION; // Bounce
                    }
                    collided = true;
                }
            }
            if (!dieOnCollition || (dieOnCollition && !collided))
            {
                input[0].position = (deltaTime * ((input[0].age * acceleration) + input[0].velocity)) + input[0].position;
            
                // Add to Stream Output
                particleStream.Append(input[0]);
            }
        }
    }
}