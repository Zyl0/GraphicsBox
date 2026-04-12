#define TINYGLTF3_IMPLEMENTATION
#define TINYGLTF3_ENABLE_FS

#ifdef CONFIG_RELEASE
#define TINYGLTF3_JSON_SIMD_AVX2
#endif // CONFIG_RELEASE

#ifdef CONFIG_DEBUG

#include "Assertion.h"

// Overrides the tg3 assert call
#ifndef TINYGLTF3_ASSERT
#define TINYGLTF3_ASSERT(x) Assert(x)
#endif // TINYGLTF3_ASSERT

#endif // CONFIG_DEBUG

#include "tiny_gltf_v3.h"