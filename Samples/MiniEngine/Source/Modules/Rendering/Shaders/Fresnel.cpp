#include "Modules/Rendering/Shaders/Fresnel.h"

namespace Rendering
{
    float FresnelG(float NdotL, float Refraction)
    {
        return sqrt(Refraction * Refraction + NdotL * NdotL - 1);
    }

    Math::Vector2f FresnelG(float NdotL, Math::Vector2f Refraction)
    {
        return Math::Sqrt(Refraction * Refraction + NdotL * NdotL - 1.f);
    }

    Math::Vector3f FresnelG(float NdotL, Math::Vector3f Refraction)
    {
        return Math::Sqrt(Refraction * Refraction + NdotL * NdotL - 1.f);
    }

    Math::Vector4f FresnelG(float NdotL, Math::Vector4f Refraction)
    {
        return Math::Sqrt(Refraction * Refraction + NdotL * NdotL - 1.f);
    }

    float Fresnel(float NdotL, float F0)
    {
        float sqrtF0 = sqrt(F0);
        float Refraction = (1 + sqrtF0) / (1 - sqrtF0);

        float G = FresnelG(NdotL, Refraction);
        float C = NdotL;

        float PartA = (G - C) / (G + C);
        float PartB = (C * (G + C) - 1) / (C * (G + C) + 1);

        return (1 / 2) * (PartA * PartA) * (1 + (PartB * PartB));
    }

    Math::Vector2f Fresnel(float NdotL, Math::Vector2f F0)
    {
        Math::Vector2f sqrtF0 = Sqrt(F0);
        Math::Vector2f Refraction = (1.f + sqrtF0) / (1.f - sqrtF0);

        Math::Vector2f G = FresnelG(NdotL, Refraction);
        float C = NdotL;

        Math::Vector2f PartA = (G - C) / (G + C);
        Math::Vector2f PartB = (C * (G + C) - 1) / (C * (G + C) + 1);

        return (1.f / 2) * (PartA * PartA) * (1.f + (PartB * PartB));
    }

    Math::Vector3f Fresnel(float NdotL, Math::Vector3f F0)
    {
        Math::Vector3f sqrtF0 = Sqrt(F0);
        Math::Vector3f Refraction = (1.f + sqrtF0) / (1.f - sqrtF0);

        Math::Vector3f G = FresnelG(NdotL, Refraction);
        float C = NdotL;

        Math::Vector3f PartA = (G - C) / (G + C);
        Math::Vector3f PartB = (C * (G + C) - 1.f) / (C * (G + C) + 1.f);

        return (1.f / 2) * (PartA * PartA) * (1.f + (PartB * PartB));
    }

    Math::Vector4f Fresnel(float NdotL, Math::Vector4f F0)
    {
        Math::Vector4f sqrtF0 = Sqrt(F0);
        Math::Vector4f Refraction = (1.f + sqrtF0) / (1.f - sqrtF0);

        Math::Vector4f G = FresnelG(NdotL, Refraction);
        float C = NdotL;

        Math::Vector4f PartA = (G - C) / (G + C);
        Math::Vector4f PartB = (C * (G + C) - 1.f) / (C * (G + C) + 1.f);

        return (1.f / 2) * (PartA * PartA) * (1.f + (PartB * PartB));
    }
}
