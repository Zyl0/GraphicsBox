#pragma once

#include "Shared/Annotations.h"

size_t SquareMatrixMullAddR_FLO(size_t Size);
int SquareMatrixMullAddR_RequiredMemoryAlignment();

// R = A * B + C
void SquareMatrixMulAddR_Baseline(float* Out, int Size, const float* A, const float* B, const float* C );

// R = A * B + C
// Loop interchange
void SquareMatrixMulAddR_OP0(float* Out, int Size, const float* A, const float* B, const float* C );

// R = A * B + C
// OP0 + tiling (L0, L1 & L2 optimizations) 16 * 16 + Auto SIMD (Compiler)
void SquareMatrixMulAddR_OP1_x16(float* Out, int Size, const float* A, const float* B, const float* C );

// R = A * B + C
// OP0 + tiling (L0, L1 & L2 optimizations) 24 * 24 + Auto SIMD (Compiler)
void SquareMatrixMulAddR_OP1_x24(float* Out, int Size, const float* A, const float* B, const float* C );

// R = A * B + C
// OP0 + tiling (L0, L1 & L2 optimizations) 32 * 32 + Auto SIMD (Compiler)
void SquareMatrixMulAddR_OP1_x32(float* Out, int Size, const float* A, const float* B, const float* C );

// R = A * B + C
// OP0 + tiling (L0, L1 & L2 optimizations) 48 * 48 + Auto SIMD (Compiler)
void SquareMatrixMulAddR_OP1_x48(float* Out, int Size, const float* A, const float* B, const float* C );

// R = A * B + C
// OP0 + tiling (L0, L1 & L2 optimizations) 64 * 64 + Auto SIMD (Compiler)
void SquareMatrixMulAddR_OP1_x64(float* Out, int Size, const float* A, const float* B, const float* C );

// R = A * B + C
// OP0 + tiling (L0, L1 & L2 optimizations) 16 * 16 + Auto SIMD (Compiler) + cached output buffer
void SquareMatrixMulAddR_OP1_x16_Cached(float* Out, int Size, const float* A, const float* B, const float* C );

// R = A * B + C
// OP0 + tiling (L0, L1 & L2 optimizations) 32 * 32 + Auto SIMD (Compiler) + cached output buffer
void SquareMatrixMulAddR_OP1_x32_Cached(float* Out, int Size, const float* A, const float* B, const float* C );

// R = A * B + C
// OP0 + tiling (L0, L1 & L2 optimizations) 64 * 64 + Auto SIMD (Compiler) + cached output buffer
void SquareMatrixMulAddR_OP1_x64_Cached(float* Out, int Size, const float* A, const float* B, const float* C );

// R = A * B + C
// OP0 + tiling (L0, L1 & L2 optimizations) 128 * 128 + Auto SIMD (Compiler) + cached output buffer
void SquareMatrixMulAddR_OP1_x128_Cached(float* Out, int Size, const float* A, const float* B, const float* C );

// R = A * B + C
// OP1 + Multicore
void SquareMatrixMulAddR_OP2(float* Out, int Size, const float* A, const float* B, const float* C );

// R = A * B + C
// OP1 + Multicore + cached output buffer
void SquareMatrixMulAddR_OP2_Cached(float* Out, int Size, const float* A, const float* B, const float* C );

// R = A * B + C
// OP1 + Multicore dynamic schedule + cached output buffer
void SquareMatrixMulAddR_OP2_Cached_DynamicSchedule(float* Out, int Size, const float* A, const float* B, const float* C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics (SSE)
void SquareMatrixMulAddR_OP3_SSE(float* ALIGNED(16) Out, int Size, const float* ALIGNED(16) A, const float* ALIGNED(16) B, const float* ALIGNED(16) C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics (AVX2)
void SquareMatrixMulAddR_OP3_AVX(float* ALIGNED(32) Out, int Size, const float* ALIGNED(32) A, const float* ALIGNED(32) B, const float* ALIGNED(32) C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics (AVX512)
void SquareMatrixMulAddR_OP3_AVX512(float* ALIGNED(64) Out, int Size, const float* ALIGNED(64) A, const float* ALIGNED(64) B, const float* ALIGNED(64) C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics (AVX2)
void SquareMatrixMulAddR_OP3_AVXx4(float* ALIGNED(32) Out, int Size, const float* ALIGNED(32) A, const float* ALIGNED(32) B, const float* ALIGNED(32) C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics (AVX512)
void SquareMatrixMulAddR_OP3_AVX512x2(float* ALIGNED(64) Out, int Size, const float* ALIGNED(64) A, const float* ALIGNED(64) B, const float* ALIGNED(64) C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics (SSE) + cached output buffer
void SquareMatrixMulAddR_OP3_SSE_Cached(float* ALIGNED(16) Out, int Size, const float* ALIGNED(16) A, const float* ALIGNED(16) B, const float* ALIGNED(16) C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics (AVX2)
void SquareMatrixMulAddR_OP3_AVX_Cached(float* ALIGNED(32) Out, int Size, const float* ALIGNED(32) A, const float* ALIGNED(32) B, const float* ALIGNED(32) C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics (AVX512) + cached output buffer
void SquareMatrixMulAddR_OP3_AVX512_Cached(float* ALIGNED(64) Out, int Size, const float* ALIGNED(64) A, const float* ALIGNED(64) B, const float* ALIGNED(64) C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics (AVX2) + cached output buffer
void SquareMatrixMulAddR_OP3_AVXx4_Cached(float* ALIGNED(32) Out, int Size, const float* ALIGNED(32) A, const float* ALIGNED(32) B, const float* ALIGNED(32) C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics (AVX512) + cached output buffer
void SquareMatrixMulAddR_OP3_AVX512x2_Cached(float* ALIGNED(64) Out, int Size, const float* ALIGNED(64) A, const float* ALIGNED(64) B, const float* ALIGNED(64) C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics Auto SIMD (MathSimt) (SSE)
void SquareMatrixMulAddR_OP4_SSE_MathSimt(float* ALIGNED(16) Out, int Size, const float* ALIGNED(16) A, const float* ALIGNED(16) B, const float* ALIGNED(16) C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics Auto SIMD (MathSimt) (AVX2)
void SquareMatrixMulAddR_OP4_AVX_MathSimt(float* ALIGNED(32) Out, int Size, const float* ALIGNED(32) A, const float* ALIGNED(32) B, const float* ALIGNED(32) C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics Auto SIMD (MathSimt) (AVX512)
void SquareMatrixMulAddR_OP4_AVX512_MathSimt(float* ALIGNED(64) Out, int Size, const float* ALIGNED(64) A, const float* ALIGNED(64) B, const float* ALIGNED(64) C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics (AVX2)
void SquareMatrixMulAddR_OP4_AVXx4_MathSimt(float* ALIGNED(32) Out, int Size, const float* ALIGNED(32) A, const float* ALIGNED(32) B, const float* ALIGNED(32) C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics (AVX512)
void SquareMatrixMulAddR_OP4_AVX512x2_MathSimt(float* ALIGNED(64) Out, int Size, const float* ALIGNED(64) A, const float* ALIGNED(64) B, const float*ALIGNED(64)  C );

// OP0 + Multicore + intel intrinsics (SSE) + cached output buffer
void SquareMatrixMulAddR_OP4_SSE_MathSimt_Cached(float* ALIGNED(16) Out, int Size, const float* ALIGNED(16) A, const float* ALIGNED(16) B, const float* ALIGNED(16) C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics (AVX2)
void SquareMatrixMulAddR_OP4_AVX_MathSimt_Cached(float* ALIGNED(32) Out, int Size, const float* ALIGNED(32) A, const float* ALIGNED(32) B, const float* ALIGNED(32) C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics (AVX512) + cached output buffer
void SquareMatrixMulAddR_OP4_AVX512_MathSimt_Cached(float* ALIGNED(64) Out, int Size, const float* ALIGNED(64) A, const float* ALIGNED(64) B, const float* ALIGNED(64) C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics (AVX2) + cached output buffer
void SquareMatrixMulAddR_OP4_AVXx4_MathSimt_Cached(float* ALIGNED(32) Out, int Size, const float* ALIGNED(32) A, const float* ALIGNED(32) B, const float* ALIGNED(32) C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics (AVX512) + cached output buffer
void SquareMatrixMulAddR_OP4_AVX512x2_MathSimt_Cached(float* ALIGNED(64) Out, int Size, const float* ALIGNED(64) A, const float* ALIGNED(64) B, const float* ALIGNED(64) C );