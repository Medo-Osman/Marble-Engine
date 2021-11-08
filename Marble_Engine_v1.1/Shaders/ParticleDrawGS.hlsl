#define PT_EMITTER 0
#define PT_FLARE 1
 
// Vertex/Point
struct GS_IN
{
    float3  position    : POSITION;
    float   rotation    : ROTATION;
    float2  size        : SIZE;
    float4  color       : COLOR;
    uint    type        : TYPE;
    uint    id          : ID;
};

struct GS_OUT
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
    float intensity;
    float scaleVariationMax;
    float rotationVariationMax;
    float lifetime;
    bool useNoise;
    float3 emitDirection;
    float emitInterval;
};

// Texture
Texture1D RandomTexture : register(t0);

SamplerState sampState : register(s0);

// Functions
float3 RandVec3(float offset)
{
    // Use game time plus offset to sample random texture.
    float sampleIndex = (gameTime + offset);
    
    // coordinates in [-1,1]
    float3 v = RandomTexture.SampleLevel(sampState, sampleIndex, 0).xyz;
    
    return v;
}

// Main
[maxvertexcount(4)]
void main(point GS_IN input[1], inout TriangleStream<GS_OUT> triStream)
{   
    // do not draw emitter particles.
    if (input[0].type != PT_EMITTER)
    {
        // Compute world matrix so that billboard faces the camera.
        float3 look = normalize(cameraPosition.xyz - input[0].position);
          
        // Random Value
        float rand = RandVec3(0.f).x;
        
        // Up
        float3 up = float3(0.f, 1.f, 0.f);
        
        // Right
        float3 right = normalize(cross(look, up));
        
        // Rotation
        float s, c;
        sincos(input[0].rotation, s, c);
        float one_minus_c = 1.0 - c;
        
        // - Create Rotation matrix
        float3x3 rot_mat =
        {
            one_minus_c * look.x * look.x + c, one_minus_c * look.x * look.y - look.z * s, one_minus_c * look.z * look.x + look.y * s,
            one_minus_c * look.x * look.y + look.z * s, one_minus_c * look.y * look.y + c, one_minus_c * look.y * look.z - look.x * s,
            one_minus_c * look.z * look.x - look.y * s, one_minus_c * look.y * look.z + look.x * s, one_minus_c * look.z * look.z + c
        };
        
        // - Rotate Vectors
        right = mul(rot_mat, right);
        up = normalize(cross(look, right));
        
        // Compute triangle strip vertices (quad) in world space.
        float halfWidth = input[0].size.x * 0.5f;
        float halfHeight = input[0].size.y * 0.5f;
        
        GS_OUT vertices[4];
        vertices[0].position = float4(input[0].position + (halfWidth * right) - (halfHeight * up), 1.0f);
        vertices[0].texCoord = float2(0.0f, 1.0f);
        vertices[0].color = input[0].color;
        vertices[0].id = input[0].id;
        
        vertices[1].position = float4(input[0].position + (halfWidth * right) + (halfHeight * up), 1.0f);
        vertices[1].texCoord = float2(0.0f, 0.0f);
        vertices[1].color = input[0].color;
        vertices[1].id = input[0].id;
        
        vertices[2].position = float4(input[0].position - (halfWidth * right) - (halfHeight * up), 1.0f);
        vertices[2].texCoord = float2(1.0f, 1.0f);
        vertices[2].color = input[0].color;
        vertices[2].id = input[0].id;
        
        vertices[3].position = float4(input[0].position - (halfWidth * right) + (halfHeight * up), 1.0f);
        vertices[3].texCoord = float2(1.0f, 0.0f);
        vertices[3].color = input[0].color;
        vertices[3].id = input[0].id;
        
        // Transform quad vertices to world space and output 
        // them as a triangle strip.
        [unroll]
        for (int i = 0; i < 4; ++i)
        {
            vertices[i].position = mul(vertices[i].position, viewMatrix);
            vertices[i].position = mul(vertices[i].position, projMatrix);
            triStream.Append(vertices[i]);
        }
    }
}