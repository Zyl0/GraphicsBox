#include "Modules/Rendering/Shaders/LightinModel.h"

namespace Rendering
{
    Math::Vector3f fDielectrical(const Math::Vector3f& DiffuseColor, float Normalisation, const Math::Vector3f& F)
    {
        return ((1.f - F) * DiffuseColor / static_cast<float>(M_PI)) + (F * Normalisation);
    }

    Math::Vector3f fDielectricalIndirect(const Math::Vector3f& DiffuseColor, float Normalisation, const Math::Vector3f& F)
    {
        return F * Normalisation * DiffuseColor;
    }

    Math::Vector3f fMetallic(float Normalisation, const Math::Vector3f& F)
    {
        return ((1.f - F) * Math::Vector3f(0)) + (F * Normalisation);
    }

    Math::Vector3f fMetallicIndirect(const Math::Vector3f& DiffuseColor, float Normalisation, const Math::Vector3f& F)
    {
        return F * Normalisation * Math::Vector3f(1.0f);
    }
}
