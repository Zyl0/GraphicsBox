#pragma once

#include "Shared/Annotations.h"

size_t SquareMatrixMullAddR_FLO(int Size);
int SquareMatrixMullAddR_RequiredMemoryAlignment();

// R = A * B + C
void SquareMatrixMulAddR_Baseline(float* Out, int Size, const float* A, const float* B, const float* C );

// R = A * B + C
// Loop interchange + tiling (L0, L1 & L2 optimizations)
void SquareMatrixMulAddR_OP0(float* Out, int Size, const float* A, const float* B, const float* C );

// R = A * B + C
// OP0 + Multicore + Auto SIMD (Compiler)
void SquareMatrixMulAddR_OP1(float* Out, int Size, const float* A, const float* B, const float* C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics (SSE)
void SquareMatrixMulAddR_OP1_SSE(float* ALIGNED(16) Out, int Size, const float* A, const float* B, const float* C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics (AVX2)
void SquareMatrixMulAddR_OP1_AVX(float* ALIGNED(32) Out, int Size, const float* A, const float* B, const float* C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics (AVX512)
void SquareMatrixMulAddR_OP1_AVX512(float* ALIGNED(64) Out, int Size, const float* A, const float* B, const float* C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics (AVX2)
void SquareMatrixMulAddR_OP1_AVXx4(float* ALIGNED(32) Out, int Size, const float* A, const float* B, const float* C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics (AVX512)
void SquareMatrixMulAddR_OP1_AVX512x2(float* ALIGNED(64) Out, int Size, const float* A, const float* B, const float* C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics Auto SIMD (MathSimt) (SSE)
void SquareMatrixMulAddR_OP1_SSE_MathSimt(float* ALIGNED(16) Out, int Size, const float* ALIGNED(16) A, const float* ALIGNED(16) B, const float* ALIGNED(16) C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics Auto SIMD (MathSimt) (AVX2)
void SquareMatrixMulAddR_OP1_AVX_MathSimt(float* ALIGNED(32) Out, int Size, const float* ALIGNED(32) A, const float* ALIGNED(32) B, const float* ALIGNED(32) C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics Auto SIMD (MathSimt) (AVX512)
void SquareMatrixMulAddR_OP1_AVX512_MathSimt(float* ALIGNED(64) Out, int Size, const float* ALIGNED(64) A, const float* ALIGNED(64) B, const float* ALIGNED(64) C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics (AVX2)
void SquareMatrixMulAddR_OP1_AVXx4_MathSimt(float* ALIGNED(32) Out, int Size, const float* A, const float* B, const float* C );

// R = A * B + C
// OP0 + Multicore + intel intrinsics (AVX512)
void SquareMatrixMulAddR_OP1_AVX512x2_MathSimt(float* ALIGNED(64) Out, int Size, const float* A, const float* B, const float* C );