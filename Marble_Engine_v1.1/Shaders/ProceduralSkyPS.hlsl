// Global
static const float PI = 3.14159265359;

// Pixel
struct PixelShaderInput
{
    float4 Position : SV_POSITION;
    float4 HPosition : POSITIONT;
    float3 WPosition : POSITION;
};

// Constant Buffers
cbuffer ProceduralSkyCB : register(b4)
{
    float3 skyColor;
    float skyExponent;
    float3 skyNightColor;
    float skyNightExponent;
    
    float3 horizonColor;
    float intensity;
    float3 horizonNightColor;
    bool useSunMoonColorForHorizon;

    float3 groundColor;
    float groundExponent;
    float3 groundNightColor;
    float groundNightExponent;
    
    float skySunIntensity;
    float skySunExponent;
    float skySunRadiusA;
    float skySunRadiusB;
    
    float3 sunSetRiseColor;
    
    float starsUVScale;
    float starsIntensity;
    
    float moonIntensity;
    float moonRadiusA;
    float moonRadiusB;
    
    float3 moonColor;
    float moonExponent;
};

cbuffer SunDataCB : register(b5)
{
    float3 sunDirection;
    float sunIntensity;
    float3 sunColor;
    int moonOrSun; // Unused in this Shader
    bool sunCastingShadow; // Unused in this Shader
};

// Functions
float remap(float value, float2 inMinMax, float2 outMinMax)
{
    return outMinMax.x + (value - inMinMax.x) * (outMinMax.y - outMinMax.x) / (inMinMax.y - inMinMax.x);
}

// Textures
Texture2D StarsTexture       : register(t7);
Texture2D MoonTexture        : register(t8);
Texture2D BlueNoiseTexture   : register(t9);
SamplerState sampState       : register(s1); // Imgui uses slot 0, use 1 for default

// Main
float4 main(PixelShaderInput input) : SV_TARGET
{
    float3 finalColor = (float3)0;
    
    // Day/Night interpolation
    float dayNightValue = dot(float3(0.f, -1.f, 0.f), -sunDirection);
    float sunSetRiseRange = abs(dayNightValue);
    dayNightValue = remap(dayNightValue, float2(-1.f, 1.f), float2(0.f, 1.f));
    
    // Altitude
    float3 worldPosNorm = normalize(input.WPosition);
    float3 worldPosSatNorm = saturate(normalize(input.WPosition));
    float altitude = worldPosNorm.y;
    float clampedAltitude = saturate(altitude);
    
    // Blue Noise
    float2 noiseUV = (1.f - clampedAltitude) * worldPosNorm.xz;
    noiseUV = ((noiseUV + worldPosNorm.xz) * 2.f);
    float blueNoise = (BlueNoiseTexture.Sample(sampState, noiseUV).r - 0.5f);
    
    // Gradient Altitudes
    float skyAltitude = (1.f - pow(min(1, 1.f - altitude), lerp(skyExponent, skyNightExponent, dayNightValue)));
    float groundAltitude = (1.f - pow(min(1, 1.f + altitude), lerp(groundExponent, groundNightExponent, dayNightValue)));
    float horizonAltitude = (1.f - skyAltitude - groundAltitude);
    
    float3 horizonCol = lerp(horizonColor, horizonNightColor, dayNightValue);
    
    // Sun Set/Rise
    float SunSetRiseRadius = 0.4f;
    
    float sunRangeUnderMax = step(sunSetRiseRange, SunSetRiseRadius);
    sunSetRiseRange = (SunSetRiseRadius - sunSetRiseRange) * sunRangeUnderMax; // remap from (max - 0 - max) to (0 - max - 0)
    float sunSetRiseColorLerp = smoothstep(0.f, SunSetRiseRadius, sunSetRiseRange);
    sunSetRiseRange = sunSetRiseColorLerp;
    
    // - Sun Distance
    float posToSunDistance = distance(worldPosNorm, -sunDirection);
    float sunSetRiseHorizonRange = sunSetRiseRange * smoothstep(0.f, 2.f, 2.f - posToSunDistance);
    sunSetRiseRange *= smoothstep(0.f, 2.f, 0.6f - posToSunDistance);
    sunSetRiseHorizonRange += step(0.00001, sunSetRiseHorizonRange) * blueNoise * 0.05f;
    sunSetRiseRange += step(0.00001, sunSetRiseRange) * blueNoise * 0.05f;
    
    horizonCol = lerp(horizonCol, sunSetRiseColor, sunSetRiseHorizonRange);
    
    // Stars
    float2 starAnimation = float2(.5f - sunDirection.x, -sunDirection.z); // Follows Moon
    float starFadeOut = 1.f;
    float nightOffset = -0.4f;
    // - Stars UV
    float2 starUV = (1.f - clampedAltitude) * worldPosNorm.xz;
    starUV = ((starUV + worldPosNorm.xz + starAnimation) * starsUVScale);
    // - Stars Texture
    float3 stars = StarsTexture.Sample(sampState, starUV) * starsIntensity;
    // - Stars Combine
    finalColor += (stars * pow(clampedAltitude, starFadeOut)) * lerp(0.f, 1.f, saturate(dayNightValue + nightOffset));
    
    // Sun
    float angleToSun = saturate(dot(worldPosNorm, -sunDirection));
    // - Sun Radius
    float circleRadiusMin = min(skySunRadiusA, skySunRadiusB);
    float circleRadiusMax = max(skySunRadiusA, skySunRadiusB);
    float2 circleRadius = float2(1.f - (circleRadiusMin * circleRadiusMin), 1.f - (circleRadiusMax * circleRadiusMax));
    float circleResult = pow(saturate(remap(angleToSun, circleRadius, float2(1.f, 0.f))), skySunExponent);
    // - Sun Combine
    finalColor += lerp(sunColor, sunSetRiseColor, sunSetRiseColorLerp) * circleResult * skySunIntensity;

    // Moon
    float angleToMoon = saturate(dot(worldPosNorm, sunDirection));
    // - Moon Radius
    circleRadiusMin = min(moonRadiusA, moonRadiusB);
    circleRadiusMax = max(moonRadiusA, moonRadiusB);
    circleRadius = float2(1.f - (circleRadiusMin * circleRadiusMin), 1.f - (circleRadiusMax * circleRadiusMax));
    circleResult = pow(saturate(remap(angleToMoon, circleRadius, float2(1.f, 0.f))), moonExponent);
    // - Moon Combine
    finalColor += moonColor * circleResult * moonIntensity;
    
    // Gradient
    dayNightValue = saturate(dayNightValue + 0.4f);
    float3 gradientColor = (lerp(skyColor, skyNightColor, dayNightValue) * skyAltitude) + (lerp(groundColor, groundNightColor, dayNightValue) * groundAltitude) + (horizonCol * horizonAltitude);
    finalColor += lerp(gradientColor, sunSetRiseColor, smoothstep(0.f, 1.f, sunSetRiseRange));
    
    return float4(finalColor * intensity, 1.f);
}