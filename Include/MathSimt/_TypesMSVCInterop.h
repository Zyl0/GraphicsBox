#pragma once

// Fallbacks for instructions introduced in AVX512(VL) not present in MSVC's immintrin.h header 
#if defined(_MSC_VER) || !defined(USE_AVX512)
    #define _mm_load_epi32(addr)  _mm_load_si128((const __m128i*)(addr))
    #define _mm_loadu_epi32(addr) _mm_loadu_si128((const __m128i*)(addr))
    #define _mm_store_epi32(addr, reg) _mm_store_si128((__m128i*)(addr), reg)
    #define _mm_storeu_epi32(addr, reg) _mm_storeu_si128((__m128i*)(addr), reg)

    #define _mm256_load_epi32(addr)  _mm256_load_si256((const __m256i*)(addr))
    #define _mm256_loadu_epi32(addr) _mm256_loadu_si256((const __m256i*)(addr))
    #define _mm256_store_epi32(addr, reg) _mm256_store_si256((__m256i*)(addr), reg)
    #define _mm256_storeu_epi32(addr, reg) _mm256_storeu_si256((__m256i*)(addr), reg)
#endif