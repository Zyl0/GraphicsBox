
#include <chrono>
#include <string_view>

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

#define BENCHMARK_PRINT_RES(BenchmarkResults)\
    std::printf("%s - time: %f ms (stddev %f ms) - flops: %f (%f GFlops) - mean of error: %f\n", \
        BenchmarkResults.Name.data(), \
        BenchmarkResults.AverageTimeMS,\
        BenchmarkResults.TimeStandardDeviationMS,\
        BenchmarkResults.FLOPS,\
        BenchmarkResults.FLOPS / 1000000000.,\
        BenchmarkResults.MeanOfError\
        );

#define MakeSquareFloatMatrix(Name, Size, Alignment, Value)\
    float* Name; PLATFORM_ALIGNED_MALLOC(float, Name, Alignment, Size * Size * sizeof(float));\
    for (size_t i = 0; i < Size * Size; i++) Name[i] = Value;

#define MatrixSquareZero(Name, Size) for (size_t i = 0; i < Size * Size; i++) Name[i] = 0

#define ReleaseSquareFloatMatrix(Name)\
    PLATFORM_ALIGNED_FREE(Name)

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
    BENCHMARK_PRINT_RES(ReferenceResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 0", Opt0Result, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP0(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt0Result)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 1", Opt1Result, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP1(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt1Result)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 2 - SSE - Intrinsics", Opt2SSEResult, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP1_SSE(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt2SSEResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 2 - AVX - Intrinsics", Opt2AVXResult, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP1_AVX(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt2AVXResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 2 - AVX x 4 - Intrinsics", Opt2AVXx4Result, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP1_AVXx4(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt2AVXx4Result)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 2 - AVX512 - Intrinsics", Opt2AVX512Result,  
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP1_AVX512(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt2AVX512Result)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 2 - AVX512 x 2 - Intrinsics", Opt2AVX512x2Result,  
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP1_AVX512x2(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt2AVX512x2Result)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 3 - SSE - Math SIMT", Opt3SSEResult, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP1_SSE_MathSimt(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt3SSEResult)
        
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 3 - AVX - Math SIMT", Opt3AVXResult, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP1_AVX_MathSimt(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt3AVXResult)
    
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 3 - AVX x 4 - Math SIMT", Opt3AVXx4Result, 
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP1_AVXx4_MathSimt(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt3AVXx4Result)
        
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 3 - AVX512 - Math SIMT", Opt3AVX512Result,  
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP1_AVX512_MathSimt(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt3AVX512Result)
        
    MatrixSquareZero(Out, Size);
    BECHMARK("Square Matrix MulAdd - Opt 3 - AVX512 x 2 - Math SIMT", Opt3AVX512x2Result,  
        Out, Ref, Error, Size * Size, 
        SquareMatrixMullAddR_FLO(Size), SquareMatrixMulAddR_OP1_AVX512x2_MathSimt(Out, Size, A, B, C))
    BENCHMARK_PRINT_RES(Opt3AVX512x2Result)
    

    
    ReleaseSquareFloatMatrix(A)
    ReleaseSquareFloatMatrix(B)
    ReleaseSquareFloatMatrix(C)
    ReleaseSquareFloatMatrix(Ref)
    ReleaseSquareFloatMatrix(Out)
    ReleaseSquareFloatMatrix(Error)
}

int main(int argc, char* argv[])
{
    for (int i = 1; i < 64; i += 8) 
        BenchmarkSquareMatrixMulAdd(64 * i);

    return 0;
}
