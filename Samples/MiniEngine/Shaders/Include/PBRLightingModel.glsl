#ifndef INCLUDE_GUARD_GLSL_LIGHTING_MODEL
#define INCLUDE_GUARD_GLSL_LIGHTING_MODEL

vec3 fDielectrical( vec3 DiffuseColor, float Normalisation, vec3 F )
{
    return ((1 - F) * DiffuseColor / M_PI) + (F * Normalisation);
}

vec3 fDielectricalIndirect( vec3 DiffuseColor, float Normalisation, vec3 F )
{
    return F * Normalisation * DiffuseColor;
}


vec3 fMetallic(float Normalisation, vec3 F )
{
    return ((1 - F) * vec3(0)) + (F * Normalisation);
}

vec3 fMetallicIndirect( vec3 DiffuseColor, float Normalisation, vec3 F )
{
    return F * Normalisation * vec3(1.0f);
}

#endif // INCLUDE_GUARD_GLSL_LIGHTING_MODEL