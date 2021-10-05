// Globals
#define M_PI 3.14159265f
static const float strengthPerRay = 0.1875f; // strength / numRays
static const uint numRays = 8;
static const uint maxStepsPerRay = 100; // = 5;
static const float R = 0.5f; // sampleRadius
static const float R2 = R * R; // sampleRadius * sampleRadius
static const float NegInvR2 = -1 / R2; // sampleRadius * sampleRadius
static const float halfSampleRadius = .25f; // sampleRadius / 2
static const float fallOff = 2.0f; // the maximum distance to count samples
static const float bias = .03f; // minimum factor to start counting occluders
static const float tanBias = tan(30.0 * M_PI / 180.0);
//static const float2 PIXEL_SIZE = float2(1.f, 1.f);

//static float g_NumSteps = 3; // 0..32
//static float g_NumDir = 5; // 0..25
//static float m_RadiusMultiplier = 8.0; // 0.0..2.0
//static float m_AngleBias = 6.0; // 0.0..60.0

//static float g_R = m_RadiusMultiplier * 1.0f; // R = m_RadiusMultiplier (1.0f) * m_AORadius (radius*SceneScale) https://code.google.com/p/core-fusion/source/browse/trunk/SubKrieg/Fusion.Rendering/RenderSystem.HBAO.cs?r=1202
//static float g_Radius2 = g_R * g_R;
//static float g_NegInvRadius2 = -1.0f / g_Radius2; // g_NegInvRadius2 (???) --> -1.0f / g_R;
//static float g_AngleBias = m_AngleBias * M_PI / 180; // m_AngleBias (30) * D3DX_PI / 180;
//static float g_TanAngleBias = tan(g_AngleBias);
// static float  g_Contrast = m_Contrast / (1.0f - sin(m_AngleBias * M_PI / 180));  // m_Contrast (1.25) / (1.0f - sin(m_AngleBias * D3DX_PI / 180));


//static float g_Strength = 1.0f; // 0.0..3.0
//static float g_IntensityMul = 1.0f; // 1.0..3.0

static float nearZ = 0.1f;
static float farZ = 1000.f;
//#define FAR_PLANE_Z (460.0)

struct PS_IN
{
    float4 Position : SV_POSITION;
    float4 FrustumVector : POSITION;
    float2 TexCoord : TEXCOORD1;
};

cbuffer cameraBuffer : register(b0)
{
    //matrix viewToTexMatrix;
    matrix projectionMatrix; // the local projection matrix
    matrix invProjectionMatrix; // the inverse of the local projection matrix
    matrix viewMatrix; // the local view matrix
    float4 viewFrustumVectors[4]; // Frustum Corners
    float2 renderTargetResolution;
};

//static float g_MaxRadiusPixels = 0.1f * min(renderTargetResolution.x, renderTargetResolution.y);

cbuffer HBAOData : register(b1)
{
    float2 sampleDirections[16];
    float2 ditherScale; // the ratio between the render target size and the dither texture size. Normally: renderTargetResolution / 4
};

Texture2D DepthTexture : register(t0);
Texture2D NormalRoughnessTexture : register(t1);
Texture2D<float3> DitherTexture : register(t2); // dither texture contains: x = cos(angle), y = -sin(angle), z = jitter

SamplerState depthNormalSampler: register(s3);
SamplerState randomSampler : register(s4);






// Basic SSAO
//static float occlusionRadius = 0.5f;
//static float occlusionFadeStart = 0.2f;
//static float occlusionFadeEnd = 2.0f;
//static float surfaceEpsilon = 0.05f;
//static const int SAMPLE_COUNT = 14;

//float NdcDepthToViewDepth(float depth)
//{
//    // We can invert the calculation from NDC space to view space for the
//    // z-coordinate. We have that
//    // z_ndc = A + B/viewZ, where gProj[2,2]=A and gProj[3, 2] = B.
//    //return nearZ / (farZ - depth * (farZ - nearZ)) * farZ;
//    return projectionMatrix[3][2] / (depth - projectionMatrix[2][2]);
//}

//float occlusionFunction(float distZ)
//{
//    float occlusion = 0.f;
//    if (distZ > surfaceEpsilon)
//    {
//        float fadeLength = occlusionFadeEnd - occlusionFadeStart;
//        occlusion = saturate((occlusionFadeEnd - distZ) / fadeLength);
//    }
//    return occlusion;
//}

//float4 main(PS_IN input) : SV_TARGET
//{
//    float3 n = normalize(NormalRoughnessTexture.SampleLevel(depthNormalSampler, input.TexCoord, 0.0f).xyz);
//    n = mul(n, (float3x3) viewMatrix);
//    float pz = NdcDepthToViewDepth(DepthTexture.SampleLevel(depthNormalSampler, input.TexCoord, 0.0f).r);
//    float3 p = (pz / input.FrustumVector.z) * input.FrustumVector;
    
//    float3 randVec = DitherTexture.SampleLevel(randomSampler, input.TexCoord, 0);
    
//    float occlusionSum = 0.0f;
    
//    [unroll]
//    for (int i = 0; i < SAMPLE_COUNT; ++i)
//    {
//        float3 offset = reflect(sampleDirections[i].xyz, randVec);
    
//        float flip = sign(dot(offset, n));
        
//        float3 q = p + flip * occlusionRadius * offset;
        
//        float4 projQ = mul(float4(q, 1.0f), viewToTexMatrix);
//        projQ /= projQ.w;

//        float rz = NdcDepthToViewDepth(DepthTexture.SampleLevel(depthNormalSampler, projQ.xy, 0.0f).r);
//        float3 r = (rz / q.z) * q;
        
//        float distZ = p.z - r.z;
//        float dp = max(dot(n, normalize(r - p)), 0.0f);
//        float occlusion = dp * occlusionFunction(distZ);
        
//        occlusionSum += occlusion;
//    }
    
//    occlusionSum /= SAMPLE_COUNT;
    
//    float access = 1.0f - occlusionSum;
//    float ao = saturate(pow(access, 4.f));
//    return float4(ao, ao, ao, 1.f);
//}









//// Unproject a value from the depth buffer to the Z value in view space.
//// Multiply the result with an interpolated frustum vector to get the actual view-space coordinates
//float DepthToViewZ(float depthValue)
//{
//    //return depthValue;
//    return projectionMatrix[3][2] / (depthValue - projectionMatrix[2][2]);
//}

//// snaps a uv coord to the nearest texel centre
//float2 SnapToTexel(float2 uv, float2 maxScreenCoords)
//{
//    return round(uv * maxScreenCoords) * rcp(maxScreenCoords);
//}

//// rotates a sample direction according to the row-vectors of the rotation matrix
//float2 Rotate(float2 vec, float2 rotationX, float2 rotationY)
//{
//    float2 rotated;
//	// just so we can use dot product
//    float3 expanded = float3(vec, 0.0);
//    rotated.x = dot(expanded.xyz, rotationX.xyy);
//    rotated.y = dot(expanded.xyz, rotationY.xyy);
//    return rotated;
//}
static float fovY = 1.22173047f;

//static float2 focalLen = float2(1.0f / tan(fovY * 0.5f) * (renderTargetResolution.x / renderTargetResolution.y), 1.0f / tan(fovY * 0.5f));
//static float2 invFocalLen = float2(1.0f / focalLen.x, 1.0f / focalLen.y);


//static float2 focalLen = float2(abs(projectionMatrix[0][0]), abs(projectionMatrix[1][1]));
//static float2 invFocalLen = float2(1.f / focalLen.x, 1.f / focalLen.y);

//static float2 UVToViewA = float2(2.0f * invFocalLen.x, -2.0f * invFocalLen.y);
//static float2 UVToViewB = float2(-1.0f * invFocalLen.x, 1.0f * invFocalLen.y);



//// Gets the view position for a uv coord
//float3 GetViewPosition(float2 uv, float2 frustumDiff)
//{
//    float depth = DepthToViewZ(DepthTexture.SampleLevel(depthNormalSampler, uv, 0).r);
//    //float3 frustumVector = float3(normalize(viewFrustumVectors[3].xy) + uv * frustumDiff, 1.0);
//    //return frustumVector * DepthToViewZ(depth);
    
//    //float z = DepthTexture.Sample(depthNormalSampler, uv).r;
//    //float x = uv.x * 2 - 1;
//    //float y = (1 - uv.y) * 2 - 1;
//    //float4 ndcPosition = float4(x, y, z, 1.f);
//    //float4 viewPosition = mul(ndcPosition, invProjectionMatrix);
//    //viewPosition /= viewPosition.w;
//    //return viewPosition.xyz;
    
//    //return float3((uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f)) * invFocalLen * depth, depth);
    
//    uv = UVToViewA * uv + UVToViewB;
//    return float3(uv * depth, depth);
//}

//// Retrieves the occlusion factor for a particular sample
//// uv: the centre coordinate of the kernel
//// frustumVector: The frustum vector of the sample point
//// centerViewPos: The view space position of the centre point
//// centerNormal: The normal of the centre point
//// tangent: The tangent vector in the sampling direction at the centre point
//// topOcclusion: The maximum cos(angle) found so far, will be updated when a new occlusion segment has been found
//float GetSampleOcclusion(float2 uv, float3 frustumVector, float3 centerViewPos, float3 centerNormal, float3 tangent, inout float topOcclusion)
//{
//	// reconstruct sample's view space position based on depth buffer and interpolated frustum vector
//    float sampleDepth = DepthTexture.SampleLevel(depthNormalSampler, uv, 0);
//    float3 sampleViewPos = frustumVector * DepthToViewZ(sampleDepth);

//	// get occlusion factor based on candidate horizon elevation
//    float3 horizonVector = sampleViewPos - centerViewPos;
//    float horizonVectorLength = length(horizonVector);
	
//    float occlusion;

//	// If the horizon vector points away from the tangent, make an estimate
//    if (dot(tangent, horizonVector) < 0.0)
//        return 0.5;
//    else
//        occlusion = dot(centerNormal, horizonVector) / horizonVectorLength;
	
//	// this adds occlusion only if angle of the horizon vector is higher than the previous highest one without branching
//    float diff = max(occlusion - topOcclusion, 0.0);
//    topOcclusion = max(occlusion, topOcclusion);

//	// attenuate occlusion contribution using distance function 1 - (d/f)^2
//    float distanceFactor = saturate(horizonVectorLength / fallOff);
//    distanceFactor = 1.0 - distanceFactor * distanceFactor;
//    return diff * distanceFactor;
//}

//// Retrieves the occlusion for a given ray
//// origin: The uv coordinates of the ray origin (the AO kernel centre)
//// direction: The direction of the ray
//// jitter: The random jitter factor by which to offset the start position
//// maxScreenCoords: The maximum screen position (the texel that corresponds with uv = 1)
//// projectedRadii: The sample radius in uv space
//// numStepsPerRay: The amount of samples to take along the ray
//// centerViewPos: The view space position of the centre point
//// centerNormal: The normal of the centre point
//// frustumDiff: The difference between frustum vectors horizontally and vertically, used for frustum vector interpolation
//float GetRayOcclusion(float2 origin, float2 direction, float jitter, float2 maxScreenCoords, float2 projectedRadii, uint numStepsPerRay, float3 centerViewPos, float3 centerNormal, float2 frustumDiff)
//{
//	// calculate the nearest neighbour sample along the direction vector
//    float2 texelSizedStep = direction * rcp(renderTargetResolution);
//    direction *= projectedRadii;
	
//	// gets the tangent for the current ray, this will be used to handle opposing horizon vectors
//	// Tangent is corrected with respect to per-pixel normal by projecting it onto the tangent plane defined by the normal
//    float3 tangent = GetViewPosition(origin + texelSizedStep, frustumDiff) - centerViewPos;
//    tangent -= dot(centerNormal, tangent) * centerNormal;
	
//	// calculate uv increments per marching step, snapped to texel centres to avoid depth discontinuity artefacts
//    float2 stepUV = SnapToTexel(direction.xy / (numStepsPerRay - 1), maxScreenCoords);
	
//	// jitter the starting position for ray marching between the nearest neighbour and the sample step size
//    float2 jitteredOffset = lerp(texelSizedStep, stepUV, jitter);
//    float2 uv = SnapToTexel(origin + jitteredOffset, maxScreenCoords);
	
//	// initial frustum vector matching the starting position and its per-step increments
//    float3 frustumVector = float3(normalize(viewFrustumVectors[3].xy) + uv * frustumDiff, 1.0);
//    float2 frustumVectorStep = stepUV * frustumDiff;

//	// top occlusion keeps track of the occlusion contribution of the last found occluder.
//	// set to bias value to avoid near-occluders
//    float topOcclusion = bias;
//    float occlusion = 0.0;

//	// march!
//    for (uint step = 0; step < numStepsPerRay; ++step)
//    {
//        occlusion += GetSampleOcclusion(uv, frustumVector, centerViewPos, centerNormal, tangent, topOcclusion);

//        uv += stepUV;
//        frustumVector.xy += frustumVectorStep.xy;
//    }

//    return occlusion;
//}

//float4 main(PS_IN input) : SV_TARGET
//{
//	// The maximum screen position (the texel that corresponds with uv = 1), used to snap to texels
//	// (normally, this would be passed in as a constant)
//    float2 maxScreenCoords = renderTargetResolution - 1.0;

//	// reconstruct view-space position from depth buffer
//    float centerDepth = DepthTexture.SampleLevel(depthNormalSampler, input.TexCoord, 0);
//    float3 centerViewPos = input.FrustumVector.xyz * DepthToViewZ(centerDepth);
//    centerViewPos = GetViewPosition(input.TexCoord, float2(0, 0));
//    //return float4(centerViewPos, 1.f);
    
//    // unpack normal
//    float3 centerNormal = normalize(NormalRoughnessTexture.SampleLevel(depthNormalSampler, input.TexCoord, 0).xyz);
//    centerNormal = mul(centerNormal, (float3x3)viewMatrix);
    
//	// Get the random factors and construct the row vectors for the 2D matrix from cos(a) and -sin(a) to rotate the sample directions
//    float3 randomFactors = DitherTexture.SampleLevel(randomSampler, input.TexCoord * ditherScale, 0);
//    randomFactors = float3(1,0,0);
//    float2 rotationX = normalize(randomFactors.xy);
//    float2 rotationY = rotationX.yx * float2(-1.0f, 1.0f);
	
//	// normally, you'd pass this in as a constant, but placing it here makes things easier to understand.
//	// basically, this is just so we can use UV coords to interpolate frustum vectors
//    float2 frustumDiff = float2(viewFrustumVectors[1].x - viewFrustumVectors[2].x, viewFrustumVectors[3].y - viewFrustumVectors[2].y);
//    //return float4(viewFrustumVectors[1].xyz, 1);
//    frustumDiff = viewFrustumVectors[2] * 2;
//    // FIX FRUSTUM VECTOR ORDER!!!!
    
//    // scale the sample radius perspectively according to the given view depth (becomes ellipse)
//    float w = centerViewPos.z * projectionMatrix[2][3] + projectionMatrix[3][3];
//    float2 projectedRadii = halfSampleRadius * float2(projectionMatrix[1][1], projectionMatrix[2][2]) / w; // half radius because projection ([-1, 1]) -> uv ([0, 1])
//    float screenRadius = projectedRadii.x * renderTargetResolution.x;
    
//	// bail out if there's nothing to march
//    if (screenRadius < 1.0)
//        return float4(1.f, 0, 0, 1.f);

//	// do not take more steps than there are pixels		
//    uint numStepsPerRay = min(maxStepsPerRay, screenRadius);
	
//    float totalOcclusion = 0.0;

//    for (uint i = 0; i < numRays; ++i)
//    {
//        float2 sampleDir = Rotate(sampleDirections[i], rotationX, rotationY);
//        totalOcclusion += GetRayOcclusion(input.TexCoord, sampleDir, randomFactors.y, maxScreenCoords, projectedRadii, numStepsPerRay, centerViewPos, centerNormal, frustumDiff);
//    }
    
//    float ao = 1.0 - saturate(strengthPerRay * totalOcclusion);
//    return float4(ao, ao, ao, 1.f);
//}












// ------------------------------------------------------------------------------------------

//float3 fetchEyePos(in float2 v)
//{
//	// float z = tex2Dlod(depthSampler, float4(uv, 0, 0)).r;
//    // return uv_to_eye(uv, z);
//    float z = DepthTexture.Sample(sampState, v).r;
//    v = v * 2 - 1; // (v * float2(2.0, -2.0) - float2(1.0, -1.0));
//    float3 result = float3(v * 1.0, 1) * z;
//    return result;
//}

////----------------------------------------------------------------------------------
//float length2(float3 v)
//{
//    return dot(v, v);
//}

////----------------------------------------------------------------------------------
//float invLength(float2 v)
//{
//    return rsqrt(dot(v, v));
//}

////----------------------------------------------------------------------------------
//float3 minDiff(float3 P, float3 Pr, float3 Pl)
//{
//    float3 V1 = Pr - P;
//    float3 V2 = P - Pl;
//    return (length2(V1) < length2(V2)) ? V1 : V2;
//}

////----------------------------------------------------------------------------------
//float2 rotateDirections(float2 Dir, float2 CosSin)
//{
//    return float2(Dir.x * CosSin.x - Dir.y * CosSin.y, Dir.x * CosSin.y + Dir.y * CosSin.x);
//}

////----------------------------------------------------------------------------------
//float falloff(float d2)
//{
//    return d2 * g_NegInvRadius2 + 1.0f;
//}

////----------------------------------------------------------------------------------
//float2 snapUVOffset(float2 uv)
//{
//    return round(uv * renderTargetResolution) * PIXEL_SIZE;
//}

////----------------------------------------------------------------------------------
//float tangent(float3 T)
//{
//    return -T.z * invLength(T.xy);
//}

////----------------------------------------------------------------------------------
//float tangent(float3 P, float3 S)
//{
//    return (P.z - S.z) * invLength(S.xy - P.xy);
//}

////----------------------------------------------------------------------------------
//float biasedtangent(float3 T)
//{
//    // float phi = atan(tangent(T)) + g_AngleBias;
//    // return tan(min(phi, M_PI*0.5));
//    return tangent(T) + g_TanAngleBias;
//}

////----------------------------------------------------------------------------------
//float3 tangentVector(float2 deltaUV, float3 dPdu, float3 dPdv)
//{
//    return deltaUV.x * dPdu + deltaUV.y * dPdv;
//}

////----------------------------------------------------------------------------------
//float tanToSin(float x)
//{
//    return x * rsqrt(x * x + 1.0f);
//}

////----------------------------------------------------------------------------------
//void computeSteps(inout float2 step_size_uv, inout float numSteps, float ray_radius_pix, float rand)
//{
//    // Avoid oversampling if g_NumSteps is greater than the kernel radius in pixels
//    numSteps = min(g_NumSteps, ray_radius_pix);

//    // Divide by Ns+1 so that the farthest samples are not fully attenuated
//    float step_size_pix = ray_radius_pix / (numSteps + 1);

//    // Clamp numSteps if it is greater than the max kernel footprint
//    float maxNumSteps = g_MaxRadiusPixels / step_size_pix;
//	[branch]
//    if (maxNumSteps < numSteps)
//    {
//        // Use dithering to avoid AO discontinuities
//        numSteps = floor(maxNumSteps + rand);
//        numSteps = max(numSteps, 1);
//        step_size_pix = g_MaxRadiusPixels / numSteps;
//    }

//    // Step size in uv space
//    step_size_uv = step_size_pix * PIXEL_SIZE;
//}

////----------------------------------------------------------------------------------
//float integerateOcclusion(float2 uv0, float2 snapped_duv, float3 P, float3 dPdu, float3 dPdv, inout float tanH)
//{
//    float ao = 0;

//    // Compute a tangent vector for snapped_duv
//    float3 T1 = tangentVector(snapped_duv, dPdu, dPdv);
//    float tanT = biasedtangent(T1);
//    float sinT = tanToSin(tanT);

//    float3 S = fetchEyePos(uv0 + snapped_duv);
//    float tanS = tangent(P, S);

//    float sinS = tanToSin(tanS);
//    float d2 = length2(S - P);

//	[branch]	
//    if ((d2 < g_Radius2) && (tanS > tanT))
//    {
//        // Compute AO between the tangent plane and the sample
//        ao = falloff(d2) * (sinS - sinT);

//        // Update the horizon angle
//        tanH = max(tanH, tanS);
//    }

//    return ao;
//}

////----------------------------------------------------------------------------------
//float horizon_occlusion(float2 deltaUV, float2 texelDeltaUV, float2 uv0, float3 P, float numSteps, float randstep, float3 dPdu, float3 dPdv)
//{
//    float ao = 0;

//    // Randomize starting point within the first sample distance
//    float2 uv = uv0 + snapUVOffset(randstep * deltaUV);

//    // Snap increments to pixels to avoid disparities between xy
//    // and z sample locations and sample along a line
//    deltaUV = snapUVOffset(deltaUV);

//    // Compute tangent vector using the tangent plane
//    float3 T = deltaUV.x * dPdu + deltaUV.y * dPdv;

//    float tanH = biasedtangent(T);

//#if SAMPLE_FIRST_STEP
//    // Take a first sample between uv0 and uv0 + deltaUV
//    float2 snapped_duv = snapUVOffset( randstep * deltaUV + texelDeltaUV );
//    ao = integerateOcclusion(uv0, snapped_duv, P, dPdu, dPdv, tanH);
//    --numSteps;
//#endif

//    float sinH = tanH / sqrt(1.0f + tanH * tanH);

//	[loop]
//    for (float j = 1; j <= numSteps; ++j)
//    {
//        uv += deltaUV;
//        float3 S = fetchEyePos(uv);
//        float tanS = tangent(P, S);
//        float d2 = length2(S - P);

//        // Use a merged dynamic branch
//		[branch]
//        if ((d2 < g_Radius2) && (tanS > tanH))
//        {
//            // Accumulate AO between the horizon and the sample
//            float sinS = tanS / sqrt(1.0f + tanS * tanS);
//            ao += falloff(d2) * (sinS - sinH);

//            // Update the current horizon angle
//            tanH = tanS;
//            sinH = sinS;
//        }
//    }

//    return ao;
//}

///** Used for packing Z into the GB channels */
//float CSZToKey(float z)
//{
//    return clamp(z * (1.0 / FAR_PLANE_Z), 0.0, 1.0);
//}

///** Used for packing Z into the GB channels */
//void packKey(float key, out float2 p)
//{
//    // Round to the nearest 1/256.0
//    float temp = floor(key * 256.0);
//    // Integer part
//    p.x = temp * (1.0 / 256.0);
//    // Fractional part
//    p.y = key * 256.0 - temp;
//}

//float unpackKey(float2 p)
//{
//    return p.x * (256.0 / 257.0) + p.y * (1.0 / 257.0);
//}

///** Negative, "linear" values in world-space units */
//float LinearizeDepth(float depth)
//{
//    //return (2.0f * nearZ) / (nearZ + farZ - depth * (farZ - nearZ));
//    return rcp(depth * ((farZ - nearZ) / (-farZ * nearZ)) + farZ / (farZ * nearZ));
//}

//float4 reconstructCSZPass(PS_IN input) : COLOR0
//{
//    return float4(LinearizeDepth(DepthTexture.Sample(sampState, input.TexCoord).r), 0, 0, 0);
//}

////----------------------------------------------------------------------------------

//float4 main(PS_IN input) : SV_TARGET
//{
//    float4 output = float4(1, 1, 1, 1);
//    float3 P = fetchEyePos(input.TexCoord);
   
//    // r = cos(alpha), g = sin(alpha), b = jitter
//    float3 randomFactors = DitherTexture.SampleLevel(sampState, input.TexCoord * ditherScale, 0);
//    //float3 rand = tex2Dlod(noiseSampler, float4(vPos / 64.0, 0, 0)).rgb * 2 - 1; // 64.0 <-- noise texture width
//	//float3 rand = tex2Dlod(noiseSampler, float4((vPos.x - floor(vPos.x)) * 64, (vPos.y - floor(vPos.y)) * 64, 0, 0)).rgb;
//	//float3 rand = tex2Dlod(noiseSampler, float4(input.TexCoord*24.0, 0, 0)).rgb;
//	//float3 rand = tex2Dlod(noiseSampler, float4(input.TexCoord * (SCREEN_SIZE / 4), 0, 0)).rgb;

//    // Compute projection of disk of radius radius into uv space
//    // Multiply by 0.5 to scale from [-1,1]^2 to [0,1]^2
//    float2 ray_radius_uv = 0.5 * g_R * 30.8956 / P.z;
//    float ray_radius_pix = ray_radius_uv.x * renderTargetResolution.x;
//    if (ray_radius_pix < 10)
//    {
//        return 1.0;
//    }

//    float numSteps;
//    float2 step_size;
//    computeSteps(step_size, numSteps, ray_radius_pix, randomFactors.z);

//	// Store Z for blur
//    // packKey(CSZToKey(P.z), output.gb);
//	//float z = tex2D(depthSampler, input.TexCoord).r;
//	//packKey(CSZToKey(z), output.gb);
//    packKey(CSZToKey(P.z), output.gb);

//    // Nearest neighbor pixels on the tangent plane
//    float3 Pr, Pl, Pt, Pb;
//    Pr = fetchEyePos(input.TexCoord + float2(PIXEL_SIZE.x, 0));
//    Pl = fetchEyePos(input.TexCoord + float2(-PIXEL_SIZE.x, 0));
//    Pt = fetchEyePos(input.TexCoord + float2(0, PIXEL_SIZE.y));
//    Pb = fetchEyePos(input.TexCoord + float2(0, -PIXEL_SIZE.y));

//    // Screen-aligned basis for the tangent plane
//    float3 dPdu = minDiff(P, Pr, Pl) * (renderTargetResolution.x * PIXEL_SIZE.y);
//    float3 dPdv = minDiff(P, Pt, Pb) * (renderTargetResolution.y * PIXEL_SIZE.x);

//    float ao = 0;
//    float d;
//    float alpha = 2.0f * M_PI / g_NumDir;

//	[loop]	// <-- without this the shader takes 1 mn to compile
//    for (d = 0; d < g_NumDir; ++d)
//    {
//        float angle = alpha * d;
//        float2 dir = rotateDirections(float2(cos(angle), sin(angle)), randomFactors.xy);
//        float2 deltaUV = dir * step_size.xy;
//        float2 texelDeltaUV = dir * PIXEL_SIZE;
//        ao += horizon_occlusion(deltaUV, texelDeltaUV, input.TexCoord, P, numSteps, randomFactors.z, dPdu, dPdv);
//    }

//    output.r = 1.0 - ao / g_NumDir * g_Strength * g_IntensityMul;
//	return float4(output.r, output.r, output.r, 1.0);
//#ifdef SHOW_UNBLURRED
//#endif
	
//    return output;
//}


const float2 LinMAD = float2(0.1-10.0, 0.1+10.0) / (2.0*0.1*10.0);
float ViewSpaceZFromDepth(float d)
{
	// [0,1] -> [-1,1] clip space
    //d = d * 2.0 - 1.0;

	// Get view space Z
    //return -1.0 / (LinMAD.x * d + LinMAD.y);
    
    
    return projectionMatrix[3][2] / (d - projectionMatrix[2][2]);
}

float3 UVToViewSpace(float2 uv, float z)
{
    float2 focalLen = float2(abs(projectionMatrix[0][0]), abs(projectionMatrix[1][1]));
    float2 invFocalLen = float2(1.f / focalLen.x, 1.f / focalLen.y);

    float2 UVToViewA = float2(2.0f * invFocalLen.x, -2.0f * invFocalLen.y);
    float2 UVToViewB = float2(-1.0f * invFocalLen.x, 1.0f * invFocalLen.y);
    
    uv = UVToViewA * uv + UVToViewB;
    return float3(uv * z, z);
}

float3 GetViewPos(float2 uv)
{
    float z = ViewSpaceZFromDepth(DepthTexture.SampleLevel(depthNormalSampler, uv, 0).r);
    return UVToViewSpace(uv, z);
}

//float3 GetViewPosPoint(float2 uv)
//{
//    float2 coord = float2(gl_FragCoord.xy) + uv;
//    float z = texelFetch(texture0, coord, 0).r;
//    return UVToViewSpace(uv, z);
//}

float TanToSin(float x)
{
    return x * rsqrt(x * x + 1.0);
}

float InvLength(float2 V)
{
    return rsqrt(dot(V, V));
}

float Tangent(float3 V)
{
    return V.z * InvLength(V.xy);
}

float BiasedTangent(float3 V)
{
    return V.z * InvLength(V.xy) + tanBias;
}

float Tangent(float3 P, float3 S)
{
    return -(P.z - S.z) * InvLength(S.xy - P.xy);
}

float Length2(float3 V)
{
    return dot(V, V);
}

float3 MinDiff(float3 P, float3 Pr, float3 Pl)
{
    float3 V1 = Pr - P;
    float3 V2 = P - Pl;
    return (Length2(V1) < Length2(V2)) ? V1 : V2;
}

float2 SnapUVOffset(float2 uv)
{
    return round(uv * renderTargetResolution) * (1 / renderTargetResolution);
}

float Falloff(float d2)
{
    return d2 * NegInvR2 + 1.0f;
}

float3 HorizonOcclusion(float2 texCoord, 
                        float2 deltaUV,
						float3 P,
						float3 dPdu,
						float3 dPdv,
						float randstep,
						float numSamples)
{
    float ao = 0;

	// Offset the first coord with some noise
    float2 uv = texCoord + SnapUVOffset(randstep * deltaUV);
    deltaUV = SnapUVOffset(deltaUV);

	// Calculate the tangent vector
    float3 T = deltaUV.x * dPdu + deltaUV.y * dPdv;
    return float3(0,0, 1);

	// Get the angle of the tangent vector from the viewspace axis
    float tanH = BiasedTangent(T);
    float sinH = TanToSin(tanH);

    float tanS;
    float d2;
    float3 S;

	// Sample to find the maximum angle
    for (float s = 1; s <= numSamples; ++s)
    {
        uv += deltaUV;
        S = GetViewPos(uv);
        tanS = Tangent(P, S);
        d2 = Length2(S - P);

		// Is the sample within the radius and the angle greater?
        if (d2 < R2 && tanS > tanH)
        {
            float sinS = TanToSin(tanS);
			// Apply falloff based on the distance
            ao += Falloff(d2) * (sinS - sinH);

            tanH = tanS;
            sinH = sinS;
        }
    }
	
    return float3(ao, ao, ao);
}

float2 RotateDirections(float2 Dir, float2 CosSin)
{
    return float2(Dir.x * CosSin.x - Dir.y * CosSin.y,
                  Dir.x * CosSin.y + Dir.y * CosSin.x);
}

void ComputeSteps(inout float2 stepSizeUv, inout float numSteps, float rayRadiusPix, float rand)
{
    // Avoid oversampling if numSteps is greater than the kernel radius in pixels
    numSteps = min(numRays, rayRadiusPix);

    // Divide by Ns+1 so that the farthest samples are not fully attenuated
    float stepSizePix = rayRadiusPix / (numSteps + 1);

    // Clamp numSteps if it is greater than the max kernel footprint
    float maxNumSteps = maxStepsPerRay / stepSizePix;
    if (maxNumSteps < numSteps)
    {
        // Use dithering to avoid AO discontinuities
        numSteps = floor(maxNumSteps + rand);
        numSteps = max(numSteps, 1);
        stepSizePix = maxStepsPerRay / numSteps;
    }

    // Step size in uv space
    stepSizeUv = stepSizePix * (1 / renderTargetResolution);
}

float4 main(PS_IN input) : SV_TARGET
{
    float2 focalLen = float2(abs(projectionMatrix[0][0]), abs(projectionMatrix[1][1]));
    float2 invFocalLen = float2(1.f / focalLen.x, 1.f / focalLen.y);

    float2 UVToViewA = float2(2.0f * invFocalLen.x, -2.0f * invFocalLen.y);
    float2 UVToViewB = float2(-1.0f * invFocalLen.x, 1.0f * invFocalLen.y);
    
    float numDirections = numRays;
    float2 InvAORes = (1 / renderTargetResolution);
    
    float3 P, Pr, Pl, Pt, Pb;
    P = GetViewPos(input.TexCoord);
	// Sample neighboring pixels
    Pr = GetViewPos(input.TexCoord + float2(InvAORes.x, 0));
    Pl = GetViewPos(input.TexCoord + float2(-InvAORes.x, 0));
    Pt = GetViewPos(input.TexCoord + float2(0, InvAORes.y));
    Pb = GetViewPos(input.TexCoord + float2(0, -InvAORes.y));

    // Calculate tangent basis vectors using the minimu difference
    float3 dPdu = MinDiff(P, Pr, Pl);
    float3 dPdv = MinDiff(P, Pt, Pb) * (renderTargetResolution.y * InvAORes.x);

    // Get the random samples from the noise texture
    float3 random = DitherTexture.SampleLevel(randomSampler, input.TexCoord * (renderTargetResolution / 2), 0);

	// Calculate the projected size of the hemisphere
    float2 rayRadiusUV = 0.5 * R * focalLen / -P.z;
    float rayRadiusPix = rayRadiusUV.x * renderTargetResolution.x;

    float ao = 1.0;
    return float4(normalize(rayRadiusUV), 0, 1.f);

    // Make sure the radius of the evaluated hemisphere is more than a pixel
    if (rayRadiusPix > 1.0)
    {
        return float4(0,1,0, 1.f);
        ao = 0.0;
        float numSteps;
        float2 stepSizeUV;

    	// Compute the number of steps
        ComputeSteps(stepSizeUV, numSteps, rayRadiusPix, random.z);

        float alpha = 2.0 * M_PI / numDirections;

		// Calculate the horizon occlusion of each direction
        for (float d = 0; d < numDirections; ++d)
        {
            float theta = alpha * d;

			// Apply noise to the direction
            float2 dir = RotateDirections(float2(cos(theta), sin(theta)), random.xy);
            float2 deltaUV = dir * stepSizeUV;

			// Sample the pixels along the direction
            return float4(HorizonOcclusion(input.TexCoord,
                                    deltaUV,
									P,
									dPdu,
									dPdv,
									random.z,
									numSteps), 1.f);

        }

		// Average the results and produce the final AO
        ao = 1.0 - ao / numDirections * strengthPerRay;
    }

    return float4(ao, ao, ao, 1.f);
}