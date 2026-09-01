#pragma once

#include "_Types.h"

#ifdef USE_SSE
#include "MathSimt/_Types_SSE.h"
#endif // USE_SSE
#ifdef USE_AVX
#include "MathSimt/_Types_AVX.h"
#endif // USE_AVX
#ifdef USE_AVX512
#include "MathSimt/_Types_AVX512.h"
#endif // USE_AVX512

namespace Math::Simt
{
    template <int... Indices, typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> Permute(
    const Scalar<DataType, ThreadCount>& V
    )
    {
        static_assert(sizeof...(Indices) == ThreadCount, "Permute requires exactly N indices");
        static const typename Scalar<DataType, ThreadCount>::IndexerType I(Indices...);
        return Permute(V, I);
    }
}