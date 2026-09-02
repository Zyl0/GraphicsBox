#include "MatrixMulAdd.h"

#include <cstring>
#include <immintrin.h>

#include "MathSimt/RMath.h"
using namespace Math::Simt;

size_t SquareMatrixMullAddR_FLO(int Size)
{
    int SquareSize = Size * Size;
    return 2 * (SquareSize * Size) + SquareSize; 
}

int SquareMatrixMullAddR_RequiredMemoryAlignment()
{
    return alignof(__m512);
}

void SquareMatrixMulAddR_Baseline(float* Out, int Size, const float* A, const float* B, const float* C)
{
    for (int i = 0; i < Size; i++)
    for (int j = 0; j < Size; j++)
    {
        for (int k = 0; k < Size; k++)
            Out[i * Size + j] += A[i * Size + k] * B[k * Size + j]; 
        
        Out[i * Size + j] += C[i * Size + j];
    }
}

void SquareMatrixMulAddR_OP0(float* Out, int Size, const float* A, const float* B, const float* C)
{
    for (int i = 0; i < Size; i++)
    for (int k = 0; k < Size; k++)
    for (int j = 0; j < Size; j++)
        Out[i * Size + j] += A[i * Size + k] * B[k * Size + j]; 
    
    
    for (int i = 0; i < Size; i++)
    for (int j = 0; j < Size; j++)
        Out[i * Size + j] += C[i * Size + j];
}

void SquareMatrixMulAddR_OP1(float* Out, int Size, const float* A, const float* B, const float* C)
{
    constexpr int TileSize = 32;
    
    #pragma omp parallel 
    {
        #pragma omp for collapse(3)
        for (int ti = 0; ti < Size; ti+=TileSize)
        for (int tk = 0; tk < Size; tk+=TileSize)
        for (int tj = 0; tj < Size; tj+=TileSize)
            for (int i = ti; i < (ti + TileSize); i++)
            for (int k = tk; k < (tk + TileSize); k++)
            for (int j = tj; j < (tj + TileSize); j++)
                Out[i * Size + j] += A[i * Size + k] * B[k * Size + j]; 
    
        #pragma omp for collapse(2)
        for (int i = 0; i < Size; i++)
        for (int j = 0; j < Size; j++)
            Out[i * Size + j] += C[i * Size + j];
    }
}

void SquareMatrixMulAddR_OP1_SSE(float* ALIGNED(16) Out, int Size, const float* ALIGNED(16) A, const float* ALIGNED(16) B, const float* ALIGNED(16) C)
{
    constexpr int TileSize = 32;
    
#pragma omp parallel 
    {
#pragma omp for collapse(3)
        for (int ti = 0; ti < Size; ti+=TileSize)
        for (int tk = 0; tk < Size; tk+=TileSize)
        for (int tj = 0; tj < Size; tj+=TileSize)
        {            
            for (int i = ti; i < (ti + TileSize); i++)
            for (int k = tk; k < (tk + TileSize); k++)
            {
                __m128 a = _mm_set1_ps(A[i * Size + k]);
                for (int j = tj; j < (tj + TileSize); j += 4 /* SSE lane size */ )
                {
                    __m128 in = _mm_load_ps(Out + (i * Size + j));
                    __m128 b = _mm_load_ps(B + (k * Size + j));

                    __m128 res = _mm_add_ps(in, _mm_mul_ps(a, b));
                    _mm_store_ps(Out + (i * Size + j), res);
                }
            }
        }
    
#pragma omp for collapse(2)
        for (int i = 0; i < Size; i++)
        for (int j = 0; j < Size; j += 4)
        {
            __m128 in = _mm_load_ps(Out + (i * Size + j));
            __m128 c = _mm_loadu_ps(C + (i * Size + j));
            
            __m128 res = _mm_mul_ps(in, c);
            
            _mm_store_ps(Out + (i * Size + j), res);
        }
    }
}

void SquareMatrixMulAddR_OP1_AVX(float* ALIGNED(32) Out, int Size, const float* ALIGNED(32) A, const float* ALIGNED(32) B, const float* ALIGNED(32) C)
{
    constexpr int TileSize = 32;
    
#pragma omp parallel 
    {
#pragma omp for collapse(3)
        for (int ti = 0; ti < Size; ti+=TileSize)
        for (int tk = 0; tk < Size; tk+=TileSize)
        for (int tj = 0; tj < Size; tj+=TileSize)
        {    
            for (int i = ti; i < (ti + TileSize); i++)
            for (int k = tk; k < (tk + TileSize); k++)
            {
                __m256 a = _mm256_set1_ps(A[i * Size + k]);
                for (int j = tj; j < (tj + TileSize); j += 8 /* AVX lane size */ )
                {
                    __m256 in = _mm256_load_ps(Out + (i * Size + j));
                    __m256 b = _mm256_load_ps(B + (k * Size + j));

                    __m256 res = _mm256_add_ps(in, _mm256_mul_ps(a, b));
                    _mm256_store_ps(Out + (i * Size + j), res);
                }
            }
        }
    
#pragma omp for collapse(2)
        for (int i = 0; i < Size; i++)
        for (int j = 0; j < Size; j += 8 /* AVX lane size */ )
        {
            __m256 in = _mm256_load_ps(Out + (i * Size + j));
            __m256 c = _mm256_loadu_ps(C + (i * Size + j));
        
            __m256 res = _mm256_mul_ps(in, c);
        
            _mm256_store_ps(Out + (i * Size + j), res);
        }
    }
}

void SquareMatrixMulAddR_OP1_AVX512(float* ALIGNED(64) Out, int Size, const float* ALIGNED(64) A, const float* ALIGNED(64) B, const float* ALIGNED(64) C)
{
    constexpr int TileSize = 32;
    
#pragma omp parallel 
    {        
#pragma omp for collapse(3) 
        for (int ti = 0; ti < Size; ti+=TileSize)
        for (int tk = 0; tk < Size; tk+=TileSize)
        for (int tj = 0; tj < Size; tj+=TileSize)
        {
            for (int i = ti; i < (ti + TileSize); i++)
            for (int k = tk; k < (tk + TileSize); k++)
            {
                __m512 a = _mm512_set1_ps(A[i * Size + k]);
                for (int j = tj; j < (tj + TileSize); j += 16 /* AVX512 lane size */ )
                {
                    __m512 in = _mm512_load_ps(Out + (i * Size + j));
                    __m512 b = _mm512_load_ps(B + (k * Size + j));

                    __m512 res = _mm512_add_ps(in, _mm512_mul_ps(a, b));
                    _mm512_store_ps(Out + (i * Size + j), res);
                }
            }
        }
    
#pragma omp for collapse(2)
        for (int i = 0; i < Size; i++)
        for (int j = 0; j < Size; j += 16 /* AVX lane size */ )
        {
            __m512 in = _mm512_load_ps(Out + (i * Size + j));
            __m512 c = _mm512_loadu_ps(C + (i * Size + j));
        
            __m512 res = _mm512_mul_ps(in, c);
        
            _mm512_store_ps(Out + (i * Size + j), res);
        }
    }
}

void SquareMatrixMulAddR_OP1_AVXx4(float* ALIGNED(32) Out, int Size, const float* ALIGNED(32) A, const float* ALIGNED(32) B, const float* ALIGNED(32) C)
{
    constexpr int TileSize = 32;
    
#pragma omp parallel 
    {
#pragma omp for collapse(3)
        for (int ti = 0; ti < Size; ti+=TileSize)
        for (int tk = 0; tk < Size; tk+=TileSize)
        for (int tj = 0; tj < Size; tj+=TileSize)
        {    
            for (int i = ti; i < (ti + TileSize); i++)
            for (int k = tk; k < (tk + TileSize); k++)
            {
                __m256 a = _mm256_set1_ps(A[i * Size + k]);
                for (int j = tj; j < (tj + TileSize); j += (8 * 4) /* AVX lane size x 4 */ )
                {
                    __m256 in0 = _mm256_load_ps(Out + (i * Size + j));
                    __m256 in1 = _mm256_load_ps(Out + (i * Size + j + 8));
                    __m256 in2 = _mm256_load_ps(Out + (i * Size + j + 16));
                    __m256 in3 = _mm256_load_ps(Out + (i * Size + j + 24));
                    __m256 b0 = _mm256_load_ps(B + (k * Size + j));
                    __m256 b1 = _mm256_load_ps(B + (k * Size + j + 8));
                    __m256 b2 = _mm256_load_ps(B + (k * Size + j + 16));
                    __m256 b3 = _mm256_load_ps(B + (k * Size + j + 24));

                    __m256 res0 = _mm256_add_ps(in0, _mm256_mul_ps(a, b0));
                    __m256 res1 = _mm256_add_ps(in1, _mm256_mul_ps(a, b1));
                    __m256 res2 = _mm256_add_ps(in2, _mm256_mul_ps(a, b2));
                    __m256 res3 = _mm256_add_ps(in3, _mm256_mul_ps(a, b3));
                    
                    _mm256_store_ps(Out + (i * Size + j), res0);
                    _mm256_store_ps(Out + (i * Size + j + 8), res1);
                    _mm256_store_ps(Out + (i * Size + j + 16), res2);
                    _mm256_store_ps(Out + (i * Size + j + 24), res3);
                }
            }
        }
    
#pragma omp for collapse(2)
        for (int i = 0; i < Size; i++)
        for (int j = 0; j < Size; j += (8 * 4) /* AVX lane size x 4 */  )
        {
            __m256 in0 = _mm256_load_ps(Out + (i * Size + j));
            __m256 in1 = _mm256_load_ps(Out + (i * Size + j + 8));
            __m256 in2 = _mm256_load_ps(Out + (i * Size + j + 16));
            __m256 in3 = _mm256_load_ps(Out + (i * Size + j + 24));
            __m256 c0 = _mm256_loadu_ps(C + (i * Size + j));
            __m256 c1 = _mm256_loadu_ps(C + (i * Size + j + 8));
            __m256 c2 = _mm256_loadu_ps(C + (i * Size + j + 16));
            __m256 c3 = _mm256_loadu_ps(C + (i * Size + j + 24));
    
            __m256 res0 = _mm256_mul_ps(in0, c0);
            __m256 res1 = _mm256_mul_ps(in1, c1);
            __m256 res2 = _mm256_mul_ps(in2, c2);
            __m256 res3 = _mm256_mul_ps(in3, c3);
    
            _mm256_store_ps(Out + (i * Size + j), res0);
            _mm256_store_ps(Out + (i * Size + j + 8), res1);
            _mm256_store_ps(Out + (i * Size + j + 16), res2);
            _mm256_store_ps(Out + (i * Size + j + 24), res3);
        }
    }
}

void SquareMatrixMulAddR_OP1_AVX512x2(float* ALIGNED(64) Out, int Size, const float* ALIGNED(64) A, const float* ALIGNED(64) B, const float* ALIGNED(64) C)
{
    constexpr int TileSize = 32;
    
#pragma omp parallel 
    {        
#pragma omp for collapse(3) 
        for (int ti = 0; ti < Size; ti+=TileSize)
        for (int tk = 0; tk < Size; tk+=TileSize)
            for (int tj = 0; tj < Size; tj+=TileSize)
            {
                for (int i = ti; i < (ti + TileSize); i++)
                for (int k = tk; k < (tk + TileSize); k++)
                {
                    __m512 a = _mm512_set1_ps(A[i * Size + k]);
                    for (int j = tj; j < (tj + TileSize); j += (16 * 2) /* AVX512 lane size x 2 */ )
                    {
                        __m512 in0 = _mm512_load_ps(Out + (i * Size + j));
                        __m512 in1 = _mm512_load_ps(Out + (i * Size + j + 16));
                        __m512 b0 = _mm512_load_ps(B + (k * Size + j));
                        __m512 b1 = _mm512_load_ps(B + (k * Size + j + 16));

                        __m512 res0 = _mm512_add_ps(in0, _mm512_mul_ps(a, b0));
                        __m512 res1 = _mm512_add_ps(in1, _mm512_mul_ps(a, b1));
                        _mm512_store_ps(Out + (i * Size + j), res0);
                        _mm512_store_ps(Out + (i * Size + j + 16), res1);
                    }
                }
            }
    
#pragma omp for collapse(2)
        for (int i = 0; i < Size; i++)
        for (int j = 0; j < Size; j += (16 * 2) /* AVX512 lane size x 2 */  )
        {
            __m512 in0 = _mm512_load_ps(Out + (i * Size + j));
            __m512 in1 = _mm512_load_ps(Out + (i * Size + j + 16));
            __m512 c0 = _mm512_loadu_ps(C + (i * Size + j));
            __m512 c1 = _mm512_loadu_ps(C + (i * Size + j + 16));
    
            __m512 res0 = _mm512_mul_ps(in0, c0);
            __m512 res1 = _mm512_mul_ps(in1, c1);
    
            _mm512_store_ps(Out + (i * Size + j), res0);
            _mm512_store_ps(Out + (i * Size + j + 16), res1);
        }
    }
}

void SquareMatrixMulAddR_OP1_SSE_MathSimt(float* ALIGNED(16) Out, int Size, const float* ALIGNED(16) A, const float* ALIGNED(16) B, const float* ALIGNED(16) C)
{
    using Scalar = Scalar<float, 4>;
    constexpr int TileSize = 32;

#pragma omp parallel 
    {
#pragma omp for collapse(3)
        for (int ti = 0; ti < Size; ti+=TileSize)
        for (int tk = 0; tk < Size; tk+=TileSize)
        for (int tj = 0; tj < Size; tj+=TileSize)
        {
            for (int i = ti; i < (ti + TileSize); i++)
            for (int k = tk; k < (tk + TileSize); k++)
            {
                Scalar a(A[i * Size + k]);
                for (int j = tj; j < (tj + TileSize); j += Scalar::kThreadCount )
                {
                    Scalar in = Scalar::LoadAligned(Out + (i * Size + j));
                    Scalar b = Scalar::LoadAligned(B + (k * Size + j));

                    Scalar res = in + (a * b);
                    res.StoreAligned(Out + (i * Size + j));
                }
            }
        }

#pragma omp for collapse(2)
        for (int i = 0; i < Size; i++)
        for (int j = 0; j < Size; j += Scalar::kThreadCount)
        {    
            Scalar res = Scalar::LoadAligned(Out + (i * Size + j)) + Scalar::LoadAligned(B + (i * Size + j));
            res.StoreAligned(Out + (i * Size + j));
        }
    }
}

void SquareMatrixMulAddR_OP1_AVX_MathSimt(float* ALIGNED(32) Out, int Size, const float* ALIGNED(32) A, const float* ALIGNED(32) B, const float* ALIGNED(32) C)
{
    using Scalar = Scalar<float, 8>;
    constexpr int TileSize = 32;

#pragma omp parallel 
    {
#pragma omp for collapse(3)
        for (int ti = 0; ti < Size; ti+=TileSize)
        for (int tk = 0; tk < Size; tk+=TileSize)
        for (int tj = 0; tj < Size; tj+=TileSize)
        {
            for (int i = ti; i < (ti + TileSize); i++)
            for (int k = tk; k < (tk + TileSize); k++)
            {
                Scalar a(A[i * Size + k]);
                for (int j = tj; j < (tj + TileSize); j += Scalar::kThreadCount )
                {
                    Scalar in = Scalar::LoadAligned(Out + (i * Size + j));
                    Scalar b = Scalar::LoadAligned(B + (k * Size + j));

                    Scalar res = in + (a * b);
                    res.StoreAligned(Out + (i * Size + j));
                }
            }
        }

#pragma omp for collapse(2)
        for (int i = 0; i < Size; i++)
        for (int j = 0; j < Size; j += Scalar::kThreadCount)
        {    
            Scalar res = Scalar::LoadAligned(Out + (i * Size + j)) + Scalar::LoadAligned(B + (i * Size + j));
            res.StoreAligned(Out + (i * Size + j));
        }
    }
}

void SquareMatrixMulAddR_OP1_AVX512_MathSimt(float* ALIGNED(64) Out, int Size, const float* ALIGNED(64) A, const float* ALIGNED(64) B, const float* ALIGNED(64) C)
{
    using Scalar = Scalar<float, 16>;
    constexpr int TileSize = 32;

#pragma omp parallel 
    {
#pragma omp for collapse(3)
        for (int ti = 0; ti < Size; ti+=TileSize)
        for (int tk = 0; tk < Size; tk+=TileSize)
        for (int tj = 0; tj < Size; tj+=TileSize)
        {
            for (int i = ti; i < (ti + TileSize); i++)
            for (int k = tk; k < (tk + TileSize); k++)
            {
                Scalar a(A[i * Size + k]);
                for (int j = tj; j < (tj + TileSize); j += Scalar::kThreadCount )
                {
                    Scalar in = Scalar::LoadAligned(Out + (i * Size + j));
                    Scalar b = Scalar::LoadAligned(B + (k * Size + j));

                    Scalar res = in + (a * b);
                    res.StoreAligned(Out + (i * Size + j));
                }
            }
        }

#pragma omp for collapse(2)
        for (int i = 0; i < Size; i++)
        for (int j = 0; j < Size; j += Scalar::kThreadCount)
        {    
            Scalar res = Scalar::LoadAligned(Out + (i * Size + j)) + Scalar::LoadAligned(B + (i * Size + j));
            res.StoreAligned(Out + (i * Size + j));
        }
    }
}

void SquareMatrixMulAddR_OP1_AVXx4_MathSimt(float* Out, int Size, const float* A, const float* B, const float* C)
{
    using Scalar = Scalar<float, 8>;
    constexpr int TileSize = 32;

#pragma omp parallel 
    {
#pragma omp for collapse(3)
        for (int ti = 0; ti < Size; ti+=TileSize)
        for (int tk = 0; tk < Size; tk+=TileSize)
        for (int tj = 0; tj < Size; tj+=TileSize)
        {
            for (int i = ti; i < (ti + TileSize); i++)
            for (int k = tk; k < (tk + TileSize); k++)
            {
                Scalar a(A[i * Size + k]);
                for (int j = tj; j < (tj + TileSize); j += Scalar::kThreadCount * 4)
                {
                    Scalar in0 = Scalar::LoadAligned(Out + (i * Size + j));
                    Scalar in1 = Scalar::LoadAligned(Out + (i * Size + j + 8));
                    Scalar in2 = Scalar::LoadAligned(Out + (i * Size + j + 16));
                    Scalar in3 = Scalar::LoadAligned(Out + (i * Size + j + 24));
                    Scalar b0 = Scalar::LoadAligned(B + (k * Size + j));
                    Scalar b1 = Scalar::LoadAligned(B + (k * Size + j + 8));
                    Scalar b2 = Scalar::LoadAligned(B + (k * Size + j + 16));
                    Scalar b3 = Scalar::LoadAligned(B + (k * Size + j + 24));

                    Scalar res0 = in0 + (a * b0);
                    Scalar res1 = in1 + (a * b1);
                    Scalar res2 = in2 + (a * b2);
                    Scalar res3 = in3 + (a * b3);
                    
                    res0.StoreAligned(Out + (i * Size + j));
                    res1.StoreAligned(Out + (i * Size + j + 8));
                    res2.StoreAligned(Out + (i * Size + j + 16));
                    res3.StoreAligned(Out + (i * Size + j + 24));
                }
            }
        }

#pragma omp for collapse(2)
        for (int i = 0; i < Size; i++)
            for (int j = 0; j < Size; j += Scalar::kThreadCount * 4)
            {    
                Scalar res0 = Scalar::LoadAligned(Out + (i * Size + j)) + Scalar::LoadAligned(B + (i * Size + j));
                Scalar res1 = Scalar::LoadAligned(Out + (i * Size + j + 8)) + Scalar::LoadAligned(B + (i * Size + j + 8));
                Scalar res2 = Scalar::LoadAligned(Out + (i * Size + j + 16)) + Scalar::LoadAligned(B + (i * Size + j + 16));
                Scalar res3 = Scalar::LoadAligned(Out + (i * Size + j + 24)) + Scalar::LoadAligned(B + (i * Size + j + 24));
                res0.StoreAligned(Out + (i * Size + j));
                res1.StoreAligned(Out + (i * Size + j + 8));
                res2.StoreAligned(Out + (i * Size + j + 16));
                res3.StoreAligned(Out + (i * Size + j + 24));
            }
    }
}

void SquareMatrixMulAddR_OP1_AVX512x2_MathSimt(float* Out, int Size, const float* A, const float* B, const float* C)
{
    using Scalar = Scalar<float, 16>;
    constexpr int TileSize = 32;

#pragma omp parallel 
    {
#pragma omp for collapse(3)
        for (int ti = 0; ti < Size; ti+=TileSize)
        for (int tk = 0; tk < Size; tk+=TileSize)
        for (int tj = 0; tj < Size; tj+=TileSize)
        {
            for (int i = ti; i < (ti + TileSize); i++)
            for (int k = tk; k < (tk + TileSize); k++)
            {
                Scalar a(A[i * Size + k]);
                for (int j = tj; j < (tj + TileSize); j += Scalar::kThreadCount * 2)
                {
                    Scalar in0 = Scalar::LoadAligned(Out + (i * Size + j));
                    Scalar in1 = Scalar::LoadAligned(Out + (i * Size + j + 16));
                    Scalar b0 = Scalar::LoadAligned(B + (k * Size + j));
                    Scalar b1 = Scalar::LoadAligned(B + (k * Size + j + 16));

                    Scalar res0 = in0 + (a * b0);
                    Scalar res1 = in1 + (a * b1);
                    res0.StoreAligned(Out + (i * Size + j));
                    res1.StoreAligned(Out + (i * Size + j + 16));
                }
            }
        }

#pragma omp for collapse(2)
        for (int i = 0; i < Size; i++)
        for (int j = 0; j < Size; j += Scalar::kThreadCount * 2)
        {    
            Scalar res = Scalar::LoadAligned(Out + (i * Size + j)) + Scalar::LoadAligned(B + (i * Size + j));
            res.StoreAligned(Out + (i * Size + j));
        }
    }
}
