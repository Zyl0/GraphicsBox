#ifndef INCLUDE_GUARD_GLTF_GGX
#define INCLUDE_GUARD_GLTF_GGX

#define GLTF_Mat_None                  (0)
#define GLTF_Mat_TwoSided              (1 << 0)
#define GLTF_Mat_Masked                (1 << 1)
#define GLTF_Mat_Transparent           (1 << 2)
#define GLTF_Mat_NoShadows             (1 << 3)
#define GLTF_Mat_UseSpecularExt        (1 << 4)
#define GLTF_Mat_UseTransmissionExt    (1 << 5)
#define GLTF_Mat_UseClearCoatExt       (1 << 6)
#define GLTF_Mat_UseIORExt             (1 << 7)

// When enabled
// Color_diff = diffuse.rgb * (1 - max(specular.x, specular.y, specular.z))
// F0 = specular
// alpha = roughness ^ 2
#define GLTF_Mat_UseSpecularGlossinessPBRExt       (1 << 8)
#define GLTF_Mat_Emissive                          (1 << 0)

#endif // INCLUDE_GUARD_GLTF_GGX