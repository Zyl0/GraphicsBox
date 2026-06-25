#include "ImageOps.h"

#include "Image.h"
#include "Math/Functons.h"
#include "Memory/Functions.h"
#include "Shared/Assertion.h"

namespace _Image
{
    template <class T>
    constexpr bool IsMathVector2Type = std::is_same_v<T, Math::Vector2t<typename T::Type>>;
        
    template <class T>
    constexpr bool IsMathVector3Type = std::is_same_v<T, Math::Vector3t<typename T::Type>>;
        
    template <class T>
    constexpr bool IsMathVector4Type = std::is_same_v<T, Math::Vector4t<typename T::Type>>;
        
    template <class T>
    constexpr bool IsMathVectorType = (IsMathVector2Type<T> || IsMathVector3Type<T> || IsMathVector3Type<T>);
        
    
    template <typename ToType, typename FromType> 
    ToType ConvertRangesAware(const FromType& From)
    {
        static_assert(sizeof(ToType) == 0, "Data convertion is no handled");
    }
    
    template <> 
    INLINE uint8_t ConvertRangesAware<uint8_t, uint8_t>(const uint8_t& From)
    {
        return From;
    }
    template <> uint8_t ConvertRangesAware<uint8_t, int8_t>(const int8_t& From)
    {
        return int8_to_uint8(From);
    }
    template <> uint8_t ConvertRangesAware<uint8_t, uint16_t>(const uint16_t& From)
    {
        return From >> 8;
    }
    template <> uint8_t ConvertRangesAware<uint8_t, int16_t>(const int16_t& From)
    {
        return int16_to_uint16(From) >> 8;
    }
    template <> uint8_t ConvertRangesAware<uint8_t, uint32_t>(const uint32_t& From)
    {
        return From >> 24;
    }
    template <> uint8_t ConvertRangesAware<uint8_t, int32_t>(const int32_t& From)
    {
        return int32_to_uint32(From) >> 24;
    }
    template <> uint8_t ConvertRangesAware<uint8_t, uint64_t>(const uint64_t& From)
    {
        return From >> 56;
    }
    template <> uint8_t ConvertRangesAware<uint8_t, int64_t>(const int64_t& From)
    {
        return int64_to_uint64(From) >> 56;
    }
    template <> uint8_t ConvertRangesAware<uint8_t, float>(const float& From)
    {
        return static_cast<uint8_t>(Math::Saturate(From) * static_cast<float>(UINT8_MAX));
    }
    template <> uint8_t ConvertRangesAware<uint8_t, double>(const double& From)
    {
        return static_cast<uint8_t>(Math::Saturate(From) * static_cast<double>(UINT8_MAX));
    }
    
    template <> int8_t ConvertRangesAware<int8_t, uint8_t>(const uint8_t& From)
    {
        return uint8_to_int8(From);
    }
    template <> 
    INLINE int8_t ConvertRangesAware<int8_t, int8_t>(const int8_t& From)
    {
        return From;
    }
    template <> int8_t ConvertRangesAware<int8_t, uint16_t>(const uint16_t& From)
    {
        return uint8_to_int8(From >> 8);
    }
    template <> int8_t ConvertRangesAware<int8_t, int16_t>(const int16_t& From)
    {
        return From >> 8;
    }
    template <> int8_t ConvertRangesAware<int8_t, uint32_t>(const uint32_t& From)
    {
        return uint8_to_int8(From >> 24);
    }
    template <> int8_t ConvertRangesAware<int8_t, int32_t>(const int32_t& From)
    {
        return From >> 24;
    }
    template <> int8_t ConvertRangesAware<int8_t, uint64_t>(const uint64_t& From)
    {
        return uint8_to_int8(From >> 56);
    }
    template <> int8_t ConvertRangesAware<int8_t, int64_t>(const int64_t& From)
    {
        return From >> 56;
    }
    template <> int8_t ConvertRangesAware<int8_t, float>(const float& From)
    {
        return uint8_to_int8(static_cast<uint8_t>(Math::Saturate(From) * static_cast<float>(UINT8_MAX)));
    }
    template <> int8_t ConvertRangesAware<int8_t, double>(const double& From)
    {
        return uint8_to_int8(static_cast<uint8_t>(Math::Saturate(From) * static_cast<double>(UINT8_MAX)));
    }
    
    template <> uint16_t ConvertRangesAware<uint16_t, uint8_t>(const uint8_t& From)
    {
        return From << 8;
    }
    template <> uint16_t ConvertRangesAware<uint16_t, int8_t>(const int8_t& From)
    {
        return static_cast<uint16_t>(int8_to_uint8(From)) << 8;
    }
    template <> 
    INLINE uint16_t ConvertRangesAware<uint16_t, uint16_t>(const uint16_t& From)
    {
        return From;
    }
    template <> uint16_t ConvertRangesAware<uint16_t, int16_t>(const int16_t& From)
    {
        return int16_to_uint16(From);
    }
    template <> uint16_t ConvertRangesAware<uint16_t, uint32_t>(const uint32_t& From)
    {
        return From >> 16;
    }
    template <> uint16_t ConvertRangesAware<uint16_t, int32_t>(const int32_t& From)
    {
        return int32_to_uint32(From) >> 16;
    }
    template <> uint16_t ConvertRangesAware<uint16_t, uint64_t>(const uint64_t& From)
    {
        return From >> 48;
    }
    template <> uint16_t ConvertRangesAware<uint16_t, int64_t>(const int64_t& From)
    {
        return int64_to_uint64(From) >> 48;
    }
    template <> uint16_t ConvertRangesAware<uint16_t, float>(const float& From)
    {
        return static_cast<uint16_t>(Math::Saturate(From) * static_cast<float>(UINT16_MAX));
    }
    template <> uint16_t ConvertRangesAware<uint16_t, double>(const double& From)
    {
        return static_cast<uint16_t>(Math::Saturate(From) * static_cast<double>(UINT16_MAX));
    }
    
    template <> int16_t ConvertRangesAware<int16_t, uint8_t>(const uint8_t& From)
    {
        return static_cast<int16_t>(uint8_to_int8(From)) << 8u;
    }
    template <> int16_t ConvertRangesAware<int16_t, int8_t>(const int8_t& From)
    {
        return static_cast<uint16_t>(From) << 8u;
    }
    template <> int16_t ConvertRangesAware<int16_t, uint16_t>(const uint16_t& From)
    {
        return uint16_to_int16(From);
    }
    template <> 
    INLINE int16_t ConvertRangesAware<int16_t, int16_t>(const int16_t& From)
    {
        return From;
    }
    template <> int16_t ConvertRangesAware<int16_t, uint32_t>(const uint32_t& From)
    {
        return uint16_to_int16(From >> 16);
    }
    template <> int16_t ConvertRangesAware<int16_t, int32_t>(const int32_t& From)
    {
        return (From >> 16);
    }
    template <> int16_t ConvertRangesAware<int16_t, uint64_t>(const uint64_t& From)
    {
        return uint16_to_int16(From >> 48);
    }
    template <> int16_t ConvertRangesAware<int16_t, int64_t>(const int64_t& From)
    {
        return (From >> 48);
    }
    template <> int16_t ConvertRangesAware<int16_t, float>(const float& From)
    {
        return uint16_to_int16(static_cast<uint16_t>(Math::Saturate(From) * static_cast<float>(UINT16_MAX)));
    }
    template <> int16_t ConvertRangesAware<int16_t, double>(const double& From)
    {
        return uint16_to_int16(static_cast<uint16_t>(Math::Saturate(From) * static_cast<double>(UINT16_MAX)));
    }
    
    template <> uint32_t ConvertRangesAware<uint32_t, uint8_t>(const uint8_t& From)
    {
        return From << 24;
    }
    template <> uint32_t ConvertRangesAware<uint32_t, int8_t>(const int8_t& From)
    {
        return uint8_to_int8(From) << 24;
    }
    template <> uint32_t ConvertRangesAware<uint32_t, uint16_t>(const uint16_t& From)
    {
        return From << 16;
    }
    template <> uint32_t ConvertRangesAware<uint32_t, int16_t>(const int16_t& From)
    {
        return int16_to_uint16(From) << 16;
    }
    template <> 
    INLINE uint32_t ConvertRangesAware<uint32_t, uint32_t>(const uint32_t& From)
    {
        return From;
    }
    template <> uint32_t ConvertRangesAware<uint32_t, int32_t>(const int32_t& From)
    {
        return int32_to_uint32(From);
    }
    template <> uint32_t ConvertRangesAware<uint32_t, uint64_t>(const uint64_t& From)
    {
        return From >> 32;
    }
    template <> uint32_t ConvertRangesAware<uint32_t, int64_t>(const int64_t& From)
    {
        return int64_to_uint64(From) >> 32;
    }
    template <> uint32_t ConvertRangesAware<uint32_t, float>(const float& From)
    {
        return static_cast<uint32_t>(Math::Saturate(From) * static_cast<float>(UINT32_MAX));
    }
    template <> uint32_t ConvertRangesAware<uint32_t, double>(const double& From)
    {
        return static_cast<uint32_t>(Math::Saturate(From) * static_cast<double>(UINT32_MAX));
    }
    
    template <> int32_t ConvertRangesAware<int32_t, uint8_t>(const uint8_t& From)
    {
        return uint8_to_int8(From) << 24;
    }
    template <> int32_t ConvertRangesAware<int32_t, int8_t>(const int8_t& From)
    {
        return From << 24;
    }
    template <> int32_t ConvertRangesAware<int32_t, uint16_t>(const uint16_t& From)
    {
        return uint16_to_int16(From) << 16;
    }
    template <> int32_t ConvertRangesAware<int32_t, int16_t>(const int16_t& From)
    {
        return From << 16;
    }
    template <> int32_t ConvertRangesAware<int32_t, uint32_t>(const uint32_t& From)
    {
        return uint32_to_int32(From);
    }
    template <> 
    INLINE int32_t ConvertRangesAware<int32_t, int32_t>(const int32_t& From)
    {
        return From;
    }
    template <> int32_t ConvertRangesAware<int32_t, uint64_t>(const uint64_t& From)
    {
        return uint64_to_int64(From) >> 32;
    }
    template <> int32_t ConvertRangesAware<int32_t, int64_t>(const int64_t& From)
    {
        return From >> 32;
    }
    template <> int32_t ConvertRangesAware<int32_t, float>(const float& From)
    {
        return uint32_to_int32(static_cast<uint32_t>(Math::Saturate(From) * static_cast<float>(UINT32_MAX)));
    }
    template <> int32_t ConvertRangesAware<int32_t, double>(const double& From)
    {
        return uint32_to_int32(static_cast<uint32_t>(Math::Saturate(From) * static_cast<double>(UINT32_MAX)));
    }
    
    template <> uint64_t ConvertRangesAware<uint64_t, uint8_t>(const uint8_t& From)
    {
        return From << 56;
    }
    template <> uint64_t ConvertRangesAware<uint64_t, int8_t>(const int8_t& From)
    {
        return int8_to_uint8(From) << 56;
    }
    template <> uint64_t ConvertRangesAware<uint64_t, uint16_t>(const uint16_t& From)
    {
        return From << 48;
    }
    template <> uint64_t ConvertRangesAware<uint64_t, int16_t>(const int16_t& From)
    {
        return uint16_to_int16(From) << 48;
    }
    template <> uint64_t ConvertRangesAware<uint64_t, uint32_t>(const uint32_t& From)
    {
        return From << 32;
    }
    template <> uint64_t ConvertRangesAware<uint64_t, int32_t>(const int32_t& From)
    {
        return uint32_to_int32(From) << 32;
    }
    template <> 
    INLINE uint64_t ConvertRangesAware<uint64_t, uint64_t>(const uint64_t& From)
    {
        return From;
    }
    template <> uint64_t ConvertRangesAware<uint64_t, int64_t>(const int64_t& From)
    {
        return uint64_to_int64(From);
    }
    template <> uint64_t ConvertRangesAware<uint64_t, float>(const float& From)
    {
        return static_cast<uint64_t>(Math::Saturate(From) * static_cast<float>(UINT64_MAX));
    }
    template <> uint64_t ConvertRangesAware<uint64_t, double>(const double& From)
    {
        return static_cast<uint64_t>(Math::Saturate(From) * static_cast<double>(UINT64_MAX));
    }
    
    template <> int64_t ConvertRangesAware<int64_t, uint8_t>(const uint8_t& From)
    {
        return uint8_to_int8(From) << 56;
    }
    template <> int64_t ConvertRangesAware<int64_t, int8_t>(const int8_t& From)
    {
        return From << 56;
    }
    template <> int64_t ConvertRangesAware<int64_t, uint16_t>(const uint16_t& From)
    {
        return uint16_to_int16(From) << 48;
    }
    template <> int64_t ConvertRangesAware<int64_t, int16_t>(const int16_t& From)
    {
        return From << 48;
    }
    template <> int64_t ConvertRangesAware<int64_t, uint32_t>(const uint32_t& From)
    {
        return uint32_to_int32(From) << 32;
    }
    template <> int64_t ConvertRangesAware<int64_t, int32_t>(const int32_t& From)
    {
        return From << 32;
    }
    template <> int64_t ConvertRangesAware<int64_t, uint64_t>(const uint64_t& From)
    {
        return uint64_to_int64(From);
    }
    template <> 
    INLINE int64_t ConvertRangesAware<int64_t, int64_t>(const int64_t& From)
    {
        return From;
    }
    template <> int64_t ConvertRangesAware<int64_t, float>(const float& From)
    {
        return uint64_to_int64(static_cast<uint64_t>(Math::Saturate(From) * static_cast<float>(UINT64_MAX)));
    }
    template <> int64_t ConvertRangesAware<int64_t, double>(const double& From)
    {
        return uint64_to_int64(static_cast<uint64_t>(Math::Saturate(From) * static_cast<double>(UINT64_MAX)));
    }
    
    template <> float ConvertRangesAware<float, uint8_t>(const uint8_t& From)
    {
        return static_cast<float>(From) / static_cast<float>(UINT8_MAX);
    }
    template <> float ConvertRangesAware<float, int8_t>(const int8_t& From)
    {
        return static_cast<float>(int8_to_uint8(From)) / static_cast<float>(UINT8_MAX);
    }
    template <> float ConvertRangesAware<float, uint16_t>(const uint16_t& From)
    {
        return static_cast<float>(From) / static_cast<float>(UINT16_MAX);
    }
    template <> float ConvertRangesAware<float, int16_t>(const int16_t& From)
    {
        return static_cast<float>(int16_to_uint16(From)) / static_cast<float>(UINT16_MAX);
    }
    template <> float ConvertRangesAware<float, uint32_t>(const uint32_t& From)
    {
        return static_cast<float>(From) / static_cast<float>(UINT32_MAX);
    }
    template <> float ConvertRangesAware<float, int32_t>(const int32_t& From)
    {
        return static_cast<float>(int32_to_uint32(From)) / static_cast<float>(UINT32_MAX);
    }
    template <> float ConvertRangesAware<float, uint64_t>(const uint64_t& From)
    {
        return static_cast<float>(From) / static_cast<float>(UINT64_MAX);
    }
    template <> float ConvertRangesAware<float, int64_t>(const int64_t& From)
    {
        return static_cast<float>(int64_to_uint64(From)) / static_cast<float>(UINT64_MAX);
    }
    template <> 
    INLINE float ConvertRangesAware<float, float>(const float& From)
    {
        return From;
    }
    template <> 
    INLINE float ConvertRangesAware<float, double>(const double& From)
    {
        return static_cast<float>(From);
    }
    
    template <> double ConvertRangesAware<double, int8_t>(const int8_t& From)
    {
        return static_cast<double>(int8_to_uint8(From)) / static_cast<double>(UINT8_MAX);
    }
    template <> double ConvertRangesAware<double, uint16_t>(const uint16_t& From)
    {
        return static_cast<double>(From) / static_cast<double>(UINT16_MAX);
    }
    template <> double ConvertRangesAware<double, int16_t>(const int16_t& From)
    {
        return static_cast<double>(int16_to_uint16(From)) / static_cast<double>(UINT16_MAX);
    }
    template <> double ConvertRangesAware<double, uint32_t>(const uint32_t& From)
    {
        return static_cast<double>(From) / static_cast<double>(UINT32_MAX);
    }
    template <> double ConvertRangesAware<double, int32_t>(const int32_t& From)
    {
        return static_cast<double>(int32_to_uint32(From)) / static_cast<double>(UINT32_MAX);
    }
    template <> double ConvertRangesAware<double, uint64_t>(const uint64_t& From)
    {
        return static_cast<double>(From) / static_cast<double>(UINT64_MAX);
    }
    template <> double ConvertRangesAware<double, int64_t>(const int64_t& From)
    {
        return static_cast<double>(int64_to_uint64(From)) / static_cast<double>(UINT64_MAX);
    }
    template <> 
    INLINE double ConvertRangesAware<double, float>(const float& From)
    {
        return static_cast<double>(From);
    }
    template <> 
    INLINE double ConvertRangesAware<double, double>(const double& From)
    {
        return From;
    }
}
