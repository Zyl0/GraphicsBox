
#include <chrono>
#include <string_view>
#include <sciplot/sciplot.hpp>

#include "MatrixMulAdd.h"
#include "Shared/Logger.h"

#define TEST_COUNT 50
#define TEST_THROW_ERROR_CELLS false
#define TEST_CELL_ERROR_THRESHOLD 1e-3f

static struct BenchmarkResult
{
    std::string_view Name;
    double AverageTimeMS;
    double TimeStandardDeviationMS;
    double TimesMS[TEST_COUNT];
    double MeanOfError;
    double FLOPS;
    double FLO;
};

static void BenchmarkResultAverageTimings(BenchmarkResult& result)
{
    double sum = 0;
    for (int i = 0; i < TEST_COUNT; i++) sum += result.TimesMS[i];
    
    double avg = sum / static_cast<double>(TEST_COUNT);
    
    double avg_diff_sum = 0;
    for (int i = 0; i < TEST_COUNT; i++) avg_diff_sum += (avg - result.TimesMS[i]) * (avg - result.TimesMS[i]);
    
    double std_dev = sqrt(avg_diff_sum / TEST_COUNT);
    
    result.AverageTimeMS = avg;
    result.TimeStandardDeviationMS = std_dev;
    result.FLOPS = result.FLO / (avg / 1000.f);
}

#define BECHMARK(BenchName, BenchResult, OutBuffer, OutReference, ErrorBuffer, OutSize, FLOCount, Exec) BenchmarkResult BenchResult;\
{\
    BenchResult.Name = std::string_view(BenchName);\
    BenchResult.FLO = static_cast<double>(FLOCount);\
    \
    /*EngineLoggerLogF("Runnign benchmark \"%s\" for size %llu", BenchName, OutSize);*/\
    \
    /* Implementation validation */\
    {\
        Exec;\
        double t_bench_avg_error = 0;\
        for (size_t i = 0; i < OutSize; i++)\
        {\
            double t_err = std::abs(OutReference[i] - OutBuffer[i]);\
            t_bench_avg_error += t_err;\
            if (TEST_THROW_ERROR_CELLS && t_err > TEST_CELL_ERROR_THRESHOLD) EngineLoggerErrorF("Failed benchmark \"%s\" at offset %llu, %f != %f", BenchName, i, OutReference[i], OutBuffer[i]);\
        }\
        BenchResult.MeanOfError = t_bench_avg_error / static_cast<double>(OutSize);\
    }\
    \
    /* Timing recordings */\
    for (int i = 0; i < TEST_COUNT; i++)\
    {\
        auto t_begin = std::chrono::high_resolution_clock::now();\
        Exec;\
        auto t_end = std::chrono::high_resolution_clock::now();\
        auto t_elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(t_end-t_begin).count();\
        \
        BenchResult.TimesMS[i] = t_elapsed * 1e-6;\
    }\
    \
    BenchmarkResultAverageTimings(BenchResult);\
}

#define BENCHMARK_PRINT_RES(BenchmarkResults, ReferenceResults)\
    std::printf("%-76s - time: %8.2f ms (stddev %8.2f ms) - speedup: %6.1fx - flops: %16.1f (%6.1f GFlops) - mean of error: %12f\n", \
        BenchmarkResults.Name.data(), \
        BenchmarkResults.AverageTimeMS,\
        ReferenceResults.AverageTimeMS / BenchmarkResults.AverageTimeMS,\
        BenchmarkResults.TimeStandardDeviationMS,\
        BenchmarkResults.FLOPS,\
        BenchmarkResults.FLOPS / 1000000000.,\
        BenchmarkResults.MeanOfError\
        );

#define MakeSquareFloatMatrix(Name, Size, Alignment, Value)\
    float* Name; PLATFORM_ALIGNED_MALLOC(float, Name, Alignment, Size * Size * sizeof(float));\
    for (size_t i = 0; i < Size * Size; i++) Name[i] = Value;

#define MakeBuffer(Name, Size, Alignment) float* Name; PLATFORM_ALIGNED_MALLOC(float, Name, Alignment, Size * sizeof(float));
#define MakeBufferV(Name, Size, Alignment, Value) MakeBuffer(Name, Size, Alignment) for (size_t i = 0; i < Size; i++) Name[i] = Value;
#define ReleaseBuffer(Name) PLATFORM_ALIGNED_FREE(Name)

#define MakeSquareFloatMatrix(Name, Size, Alignment, Value) MakeBufferV(Name, Size * Size, Alignment, Value)
#define MatrixSquareZero(Name, Size) for (size_t i = 0; i < Size * Size; i++) Name[i] = 0
#define ReleaseSquareFloatMatrix(Name) ReleaseBuffer(Name)

void BenchmarkSquareMatrixMulAdd(size_t Size)
{
    MakeSquareFloatMatrix(A, Size, 64, (float)rand()/(float)(RAND_MAX))
    MakeSquareFloatMatrix(B, Size, 64, (float)rand()/(float)(RAND_MAX))
    MakeSquareFloatMatrix(C, Size, 64, (float)rand()/(float)(RAND_MAX))
    MakeSquareFloatMatrix(Ref, Size, 64, 0)
    MakeSquareFloatMatrix(Out, Size, 64, 0)
    MakeSquareFloatMatrix(Error, Size, 64, 0)
    
    SquareMatrixMulAddR_Baseline(Ref, Size, A, B, C);
    
    EngineLoggerLogF("Square Matrix MulAdd for size %llu (%llu Floating Points opeations)", Size, SquareMatrixMullAddR_FLO(Size));
    
    BECHMARK("Square Matrix MulAdd - Reference", ReferenceResult, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_Baseline(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(ReferenceResult, ReferenceResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 0", Opt0Result, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP0(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt0Result, ReferenceResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 1 - tile 16", Opt1Result16, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP1_x16(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt1Result16, ReferenceResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 1 - tile 32", Opt1Result32, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP1_x32(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt1Result32, ReferenceResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 1 - tile 64", Opt1Result64, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP1_x64(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt1Result64, ReferenceResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 1 - tile 16 + Cached Output", Opt1Result16CO, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP1_x16_Cached(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt1Result16CO, ReferenceResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 1 - tile 32 + Cached Output", Opt1Result32CO, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP1_x32_Cached(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt1Result32CO, ReferenceResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 1 - tile 64 + Cached Output", Opt1Result64CO, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP1_x64_Cached(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt1Result64CO, ReferenceResult)
    
    if (Size >= 128)
    {
        MatrixSquareZero(Out, Size);
        BECHMARK("Square Matrix MulAdd - Opt 1 - tile 128 + Cached Output", Opt1Result128CO, 
            Out, Ref, Error, Size * Size, 
            SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP1_x128_Cached(Out, Size, A, B, C))
        BENCHMARK_PRINT_RES(Opt1Result128CO, ReferenceResult)
    }
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 2 - Multicore", Opt2Result, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP2(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt2Result, ReferenceResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 2 - Multicore + Cached Output", Opt2ResultCO, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP2_Cached(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt2ResultCO, ReferenceResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 2 - Multicore in Dynamic schedule + Cached Output", Opt2ResultDCO, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP2_Cached_DynamicSchedule(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt2ResultDCO, ReferenceResult)    
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 3 - SSE - Intrinsics", Opt2SSEResult, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP3_SSE(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt2SSEResult, ReferenceResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 3 - AVX - Intrinsics", Opt2AVXResult, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP3_AVX(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt2AVXResult, ReferenceResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 3 - AVX x 4 - Intrinsics", Opt2AVXx4Result, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP3_AVXx4(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt2AVXx4Result, ReferenceResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 3 - AVX512 - Intrinsics", Opt2AVX512Result,  
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP3_AVX512(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt2AVX512Result, ReferenceResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 3 - AVX512 x 2 - Intrinsics", Opt2AVX512x2Result,  
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP3_AVX512x2(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt2AVX512x2Result, ReferenceResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 3 - SSE - Intrinsics + Cached Output", Opt2SSEResultCO, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP3_SSE_Cached(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt2SSEResultCO, ReferenceResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 3 - AVX - Intrinsics + Cached Output", Opt2AVXResultCO, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP3_AVX_Cached(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt2AVXResultCO, ReferenceResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 3 - AVX x 4 - Intrinsics + Cached Output", Opt2AVXx4ResultCO, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP3_AVXx4_Cached(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt2AVXx4ResultCO, ReferenceResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 3 - AVX512 - Intrinsics + Cached Output", Opt2AVX512ResultCO,  
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP3_AVX512_Cached(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt2AVX512ResultCO, ReferenceResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 3 - AVX512 x 2 - Intrinsics + Cached Output", Opt2AVX512x2ResultCO,  
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP3_AVX512x2_Cached(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt2AVX512x2ResultCO, ReferenceResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 4 - SSE - Math SIMT", Opt3SSEResult, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP4_SSE_MathSimt(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt3SSEResult, ReferenceResult)
        
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 4 - AVX - Math SIMT", Opt3AVXResult, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP4_AVX_MathSimt(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt3AVXResult, ReferenceResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 4 - AVX x 4 - Math SIMT", Opt3AVXx4Result, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP4_AVXx4_MathSimt(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt3AVXx4Result, ReferenceResult)
        
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 4 - AVX512 - Math SIMT", Opt3AVX512Result,  
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP4_AVX512_MathSimt(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt3AVX512Result, ReferenceResult)
        
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 4 - AVX512 x 2 - Math SIMT", Opt3AVX512x2Result,  
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP4_AVX512x2_MathSimt(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt3AVX512x2Result, ReferenceResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 4 - SSE - Math SIMT + Cached Output", Opt3SSEResultOC, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP4_SSE_MathSimt_Cached(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt3SSEResultOC, ReferenceResult)
        
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 4 - AVX - Math SIMT + Cached Output", Opt3AVXResultOC, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP4_AVX_MathSimt_Cached(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt3AVXResultOC, ReferenceResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 4 - AVX x 4 - Math SIMT + Cached Output", Opt3AVXx4ResultOC, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP4_AVXx4_MathSimt_Cached(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt3AVXx4ResultOC, ReferenceResult)
        
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 4 - AVX512 - Math SIMT + Cached Output", Opt3AVX512ResultOC,  
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP4_AVX512_MathSimt_Cached(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt3AVX512ResultOC, ReferenceResult)
        
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 4 - AVX512 x 2 - Math SIMT + Cached Output", Opt3AVX512x2ResultOC,  
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP4_AVX512x2_MathSimt_Cached(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt3AVX512x2ResultOC, ReferenceResult)
    

    
    ReleaseSquareFloatMatrix(A)
    ReleaseSquareFloatMatrix(B)
    ReleaseSquareFloatMatrix(C)
    ReleaseSquareFloatMatrix(Ref)
    ReleaseSquareFloatMatrix(Out)
    ReleaseSquareFloatMatrix(Error)
}

int main(int argc, char* argv[])
{
    // Bandwidth measurements
#ifndef CONFIG_DEBUG
    {
        constexpr size_t BufferSize = 1llu * 1024 * 1024 * 1024; // 4gb
        
        MakeBufferV(A, BufferSize, 4, (float)rand()/(float)(RAND_MAX))
        MakeBuffer(B, BufferSize, 4)
        
        auto t_begin = std::chrono::high_resolution_clock::now();
        std::memcpy(B, A, BufferSize * sizeof(float));
        auto t_end = std::chrono::high_resolution_clock::now();
        auto t_elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(t_end-t_begin).count();
        
        double elapsed_seconds = static_cast<double>(t_elapsed) * 1e-9;
        EngineLoggerLogF("Memory Bandwidth: %f MegaBytes/Seconds", (static_cast<double>(BufferSize) / elapsed_seconds) / (1024 * 1024)); 
        
        ReleaseBuffer(A)
        ReleaseBuffer(B)
    }
#endif // !CONFIG_DEBUG
    
    // Matrix MulAdd benchmark
    for (size_t i = 1; i <= 64llu; i *= 2) 
        BenchmarkSquareMatrixMulAdd(64llu * i);

    return 0;
}
