#include "Modules/Rendering/Shaders/FresnelSchlick.h"

namespace Rendering
{
    float FresnelSchlick(float NdotL, float F0)
    {
        return F0 + (1.f - F0) * pow(1.f - std::clamp(NdotL, 0.f, 1.f), 5.f);
    }

    Math::Vector2f FresnelSchlick(float NdotL, const Math::Vector2f& F0)
    {
        return F0 + (1.f - F0) * pow(1.f - std::clamp(NdotL, 0.f, 1.f), 5.f);
    }

    Math::Vector3f FresnelSchlick(float NdotL, const Math::Vector3f& F0)
    {
        return F0 + (1.f - F0) * pow(1.f - std::clamp(NdotL, 0.f, 1.f), 5.f);
    }

    Math::Vector4f FresnelSchlick(float NdotL, const Math::Vector4f& F0)
    {
        return F0 + (1.f - F0) * pow(1.f - std::clamp(NdotL, 0.f, 1.f), 5.f);
    }
}
