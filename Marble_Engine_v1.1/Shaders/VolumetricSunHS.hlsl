// Gloabals
#define NUM_CONTROL_POINTS 4

#define MIN_TESS_FACTOR 0.f
#define MIN_TESS_DISTANCE 0.f
#define MAX_TESS_FACTOR 6.f // 6.f
#define MAX_TESS_DISTANCE 150.f

// Vertex In
struct HS_IN
{
    float3 wPosition : POSITION;
    float2 texCoord : TEXCOORD;
};

// Constant Buffer
cbuffer tessFactor : register(b0)
{
    float factor;
}

cbuffer cameraBuffer : register(b1)
{
    float4 camPosition;
};

cbuffer sunData : register(b2)
{
    float3 sunDirection;
    float sunBoundsDepth;
    float3 sunColor;
};

// Texture
Texture2D ShadowMap: TEXTURE : register(t0);
SamplerState sampState : SAMPLER : register(s0);

// Output Hull Data
struct HullShaderConstantDataOutput
{
    float EdgeTessFactor[4] : SV_TessFactor; // e.g. would be [4] for a quad domain
    float InsideTessFactor[2] : SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
};

// Functions
float TessLodCalculation(float3 position)
{
    position.y = camPosition.y; // Ignore Y dimension
    float dist = distance(position, camPosition.xyz);
    float s = saturate((dist - MIN_TESS_DISTANCE) / (MAX_TESS_DISTANCE - MIN_TESS_DISTANCE));
    
    return pow(2, (lerp(MAX_TESS_FACTOR, MIN_TESS_FACTOR, s)));
}

HullShaderConstantDataOutput CalcHSPatchConstants(InputPatch<HS_IN, NUM_CONTROL_POINTS> ip, uint PatchID : SV_PrimitiveID)
{
    HullShaderConstantDataOutput output;
    //if (ip[0].texCoord.x != 0.f && ip[0].texCoord.y != 0.f && ip[0].texCoord.x != 1.f && ip[0].texCoord.y != 1.f)
    //{
        float height0 = ShadowMap.SampleLevel(sampState, ip[0].texCoord, 0).r;
        float height1 = ShadowMap.SampleLevel(sampState, ip[1].texCoord, 0).r;
        float height2 = ShadowMap.SampleLevel(sampState, ip[2].texCoord, 0).r;
        float height3 = ShadowMap.SampleLevel(sampState, ip[3].texCoord, 0).r;
    
        float3 e0 = (0.5f * (ip[0].wPosition + ip[2].wPosition)) + (sunDirection * (lerp(height0, height2, 0.5f) * sunBoundsDepth));
        float3 e1 = (0.5f * (ip[0].wPosition + ip[1].wPosition)) + (sunDirection * (lerp(height0, height1, 0.5f) * sunBoundsDepth));
        float3 e2 = (0.5f * (ip[1].wPosition + ip[3].wPosition)) + (sunDirection * (lerp(height1, height3, 0.5f) * sunBoundsDepth));
        float3 e3 = (0.5f * (ip[2].wPosition + ip[3].wPosition)) + (sunDirection * (lerp(height2, height3, 0.5f) * sunBoundsDepth));
    
        float3 c = 0.25f * (ip[0].wPosition + (sunDirection * height0 * sunBoundsDepth)) +
                            (ip[1].wPosition + (sunDirection * height1 * sunBoundsDepth)) +
                            (ip[2].wPosition + (sunDirection * height2 * sunBoundsDepth)) +
                            (ip[3].wPosition + (sunDirection * height3 * sunBoundsDepth));
    
        output.EdgeTessFactor[0] = TessLodCalculation(e0);
        output.EdgeTessFactor[1] = TessLodCalculation(e1);
        output.EdgeTessFactor[2] = TessLodCalculation(e2);
        output.EdgeTessFactor[3] = TessLodCalculation(e3);
    
        output.InsideTessFactor[0] = TessLodCalculation(c);
        output.InsideTessFactor[1] = output.InsideTessFactor[0];
    //}
    //else
    //{
    //    output.EdgeTessFactor[0] = 0.f;
    //    output.EdgeTessFactor[1] = 0.f;
    //    output.EdgeTessFactor[2] = 0.f;
    //    output.EdgeTessFactor[3] = 0.f;
    
    //    output.InsideTessFactor[0] = 0.f;
    //    output.InsideTessFactor[1] = 0.f;
    //}
    
    //float3 e0 = 0.5f * (ip[0].wPosition + ip[2].wPosition);
    //float3 e1 = 0.5f * (ip[0].wPosition + ip[1].wPosition);
    //float3 e2 = 0.5f * (ip[1].wPosition + ip[2].wPosition);
    //float3 c = 0.333333333f * (ip[0].wPosition + ip[1].wPosition + ip[2].wPosition);
    
    //output.EdgeTessFactor[0] = TessLodCalculation(e0);
    //output.EdgeTessFactor[1] = TessLodCalculation(e1);
    //output.EdgeTessFactor[2] = TessLodCalculation(e2);
    //output.InsideTessFactor = TessLodCalculation(c);
    
    //output.EdgeTessFactor[0] = TessLodCalculation(ip[0].wPosition);
    //output.EdgeTessFactor[1] = TessLodCalculation(ip[0].wPosition);
    //output.EdgeTessFactor[2] = TessLodCalculation(ip[0].wPosition);
    //output.InsideTessFactor = 0.25f * (output.EdgeTessFactor[0] + output.EdgeTessFactor[1] + output.EdgeTessFactor[2]);

    //output.EdgeTessFactor[0] =
		//output.EdgeTessFactor[1] =
		//output.EdgeTessFactor[2] =
		//output.InsideTessFactor = TessLodCalculation(ip[0].wPosition);
    
    return output;
}

[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("CalcHSPatchConstants")]

HS_IN main(
	InputPatch<HS_IN, NUM_CONTROL_POINTS> ip,
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID)
{
    //HS_IN output;
    //output = ip[i];

    return ip[i];
}