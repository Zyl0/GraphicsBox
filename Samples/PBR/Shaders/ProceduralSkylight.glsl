#ifndef INCLUDE_GUARD_GLSL_PROCEDURAL_SKYLIGHT
#define INCLUDE_GUARD_GLSL_PROCEDURAL_SKYLIGHT

#include "Include/Math.glsl"
#include "Include/GGX.glsl"
#include "Include/FresnelSchlick.glsl"
#include "LightSources.glsl"
#include "Include/PBRLightingModel.glsl"
#include "Include/Camera.glsl"

// Procedural sky parameters
layout (binding = 2, std140)  uniform ProceduralSkySettings_t
{
    // XYZ is color, W is scale
    vec4 AtmosphereDayColor;

    // XYZ is color, W is scale
    vec4 AtmosphereNightColor;

    // XYZ is color, W is scale
    vec4 SunSetHighEnergy;

    // XYZ is color, W is scale
    vec4 SunSetLowEnergy;

    // XYZ is color, W is scale
    vec4 AtmosphericOcclusion;
    
    float AngleDayTime;
    float AngleSunSetHigh;
    float AngleSunSetLow;
    float AngleNight;

    vec3 AverageGroundColor;
    float AverageGroundRoughness;

    float AverageGroundMetalness;
    float FogHeight;
    
} SkySettings;

vec3 SampleSkylightColor(vec3 Direction)
{
    const vec2 PlanetaryCoordinates = SampleSphericalMap(Direction);
    
    // Sun light
    vec3 SunLightDirection = -normalize(LightSources.SunLight.LightDir);
    vec2 SunLightPlanetaryDirection = SampleSphericalMap(SunLightDirection);
    float SunLightZenitProximity = SunLightPlanetaryDirection.y;
    float SunLightProximity = dot(SunLightDirection , Direction);
    float HorizonProximity = 1 - abs(PlanetaryCoordinates.y * 2. - 1.);
    
    // Sly color
    vec3 SkyColor = vec3(0);
    if (PlanetaryCoordinates.y <= 0.5f)
    {
        float wDayTime = smoothstep( SkySettings.AngleSunSetHigh, SkySettings.AngleDayTime, SunLightZenitProximity);
        float wSunSetHigh = min(
            smoothstep( SkySettings.AngleDayTime, SkySettings.AngleSunSetHigh, SunLightZenitProximity),
            smoothstep( SkySettings.AngleSunSetLow, SkySettings.AngleSunSetHigh, SunLightZenitProximity)
        );
        float wSunSetLow = min(
            smoothstep( SkySettings.AngleSunSetHigh, SkySettings.AngleSunSetLow, SunLightZenitProximity),
            smoothstep( SkySettings.AngleNight, SkySettings.AngleSunSetLow, SunLightZenitProximity)
        );
        float wNightTime = smoothstep( SkySettings.AngleSunSetLow, SkySettings.AngleNight, SunLightZenitProximity);
    
        // Daylight color
        SkyColor += SkySettings.AtmosphereDayColor.xyz * SkySettings.AtmosphereDayColor.w * wDayTime;
        
        // Sunset High color
        SkyColor += SkySettings.SunSetHighEnergy.xyz * SkySettings.SunSetHighEnergy.w * wSunSetHigh;
        
        // Sunset Low color
        SkyColor += SkySettings.SunSetLowEnergy.xyz * SkySettings.SunSetLowEnergy.w * wSunSetLow;
        
        // Nightsky color
        SkyColor += SkySettings.AtmosphereNightColor.xyz * SkySettings.AtmosphereNightColor.w * wNightTime;
    }
    
    // Ground surface
    vec3 GroundColor = vec3(0);
    if (PlanetaryCoordinates.y > 0.5f)
    {
        // Surface properties
        vec3 n = vec3(0, 1, 0);
        vec3 v = -Direction;
        
        // Ground Material
        vec3 DiffuseColor = mix(SkySettings.AverageGroundColor, vec3(0), SkySettings.AverageGroundMetalness);
        vec3 F0 = mix(vec3(0.04), SkySettings.AverageGroundColor, SkySettings.AverageGroundMetalness);
        float Alpha = SkySettings.AverageGroundRoughness * SkySettings.AverageGroundRoughness;
        
        // Direct lighting
        vec3 l = normalize(-LightSources.SunLight.LightDir);
        vec3 h = normalize(v + l);
        float CosThetaL = dot(n, l);
        float CosThetaV = dot(n, v);
        if (CosThetaV > 0 && CosThetaL > 0)
        {
            float VdotH = dot(h, v);
            vec3 F = Fresnel(VdotH, F0);
    
            float D = D_GGX_Heitz2014_EQ71(h, n, Alpha);
    
            float G = G2_Heitz2014_EQ99(h, n, v, l, Alpha);
    
            float DGNormalized = (D * G) / max(4 * CosThetaL * CosThetaV, 0.0001);
    
            vec3 ReflectanceDielectrical = fDielectrical(DiffuseColor, DGNormalized, F);
            vec3 ReflectanceMetallic = fMetallic(DGNormalized, F);
    
            vec3 Reflectance = clamp(mix(ReflectanceDielectrical, ReflectanceMetallic, SkySettings.AverageGroundMetalness), 0, 1);
    
            vec3 Light = LightSources.SunLight.LightColor * CosThetaL * LightSources.SunLight.LightIntensity;

            GroundColor += Reflectance * Light;
        }
        
        // Indirect lighting (account sky color)
        // TODO see if it is needed to account roughness for Fresnel indirect
        l = vec3(0,1,0);
        h = normalize(v + l);
        {
            float VdotH = dot(h, v);
            vec3 F = Fresnel(VdotH, F0);
            
            float G1 = G1_Heitz2014_EQ98(h, n, v, Alpha);
            float G2 = G2_Heitz2014_EQ99(h, n, v, l, Alpha);

            //float D = D_GGX_Heitz2014_EQ71(h, n, Alpha);
            //float pdf = G1 * max(0, dot(n, Ne)) * D / n.z

            vec3 ReflectanceDielectrical = fDielectricalIndirect(DiffuseColor, G2 / G1, F);
            vec3 ReflectanceMetallic = fMetallicIndirect(DiffuseColor, G2 / G1, F);
            vec3 Reflectance = clamp(mix(ReflectanceDielectrical, ReflectanceMetallic, SkySettings.AverageGroundMetalness), 0, 1);
            
            GroundColor += SkyColor * Reflectance;
        }
    }

    vec3 backgroundColor = GroundColor + SkyColor;
    
    return mix(backgroundColor, RGBToLuminance(SkyColor) * SkySettings.AtmosphericOcclusion.xyz, smoothstep(SkySettings.FogHeight, 0, HorizonProximity));
}

#endif // INCLUDE_GUARD_GLSL_PROCEDURAL_SKYLIGHT