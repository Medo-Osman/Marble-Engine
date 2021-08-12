#define PT_EMITTER 0
#define PT_FLARE 1
 
struct GS_IN
{
    float3  position    : POSITION;
    float2  size        : SIZE;
    float4  color       : COLOR;
    uint    type        : TYPE;
};

struct GS_OUT
{
    float4 position : SV_Position;
    float4 color    : COLOR;
    float2 texCoord : TEXCOORD;
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

float2 quadTexCoord[4] =
{
    float2(0.0f, 1.0f),
    float2(1.0f, 1.0f),
    float2(0.0f, 0.0f),
    float2(1.0f, 0.0f)
};

// The draw GS just expands points into camera facing quads.
[maxvertexcount(4)]
void main(point GS_IN input[1], inout TriangleStream<GS_OUT> triStream)
{   
    // do not draw emitter particles.
    if (input[0].type != PT_EMITTER)
    {
        //
        // Compute world matrix so that billboard faces the camera.
        //
        float3 look = normalize(eyePosition.xyz - input[0].position);
        float3 right = normalize(cross(float3(0, 1, 0), look));
        float3 up = cross(look, right);
        
        //
        // Compute triangle strip vertices (quad) in world space.
        //
        float halfWidth = 0.5f * input[0].size.x;
        float halfHeight = 0.5f * input[0].size.y;
    
        float4 v[4];
        v[0] = float4(input[0].position + halfWidth * right - halfHeight * up, 1.0f);
        v[1] = float4(input[0].position + halfWidth * right + halfHeight * up, 1.0f);
        v[2] = float4(input[0].position - halfWidth * right - halfHeight * up, 1.0f);
        v[3] = float4(input[0].position - halfWidth * right + halfHeight * up, 1.0f);
        
        //
        // Transform quad vertices to world space and output 
        // them as a triangle strip.
        //
        GS_OUT output;
        [unroll]
        for (int i = 0; i < 4; ++i)
        {
            output.position = mul(v[i], viewProj);
            output.texCoord = quadTexCoord[i];
            output.color = input[0].color;
            triStream.Append(output);
        }
    }
}