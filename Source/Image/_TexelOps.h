#pragma once

#include "ColorSpaces.h"
#include "ImageOps.h"

#include "Memory/Functions.h"

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
        static_assert(sizeof(ToType) == 0, "Data conversion from type to this type is not handled");
    }
    
    template <> 
    INLINE uint8_t ConvertRangesAware<uint8_t, uint8_t>(const uint8_t& From)
    {
        return From;
    }
    template <>
    INLINE uint8_t ConvertRangesAware<uint8_t, int8_t>(const int8_t& From)
    {
        return int8_to_uint8(From);
    }
    template <>
    INLINE uint8_t ConvertRangesAware<uint8_t, uint16_t>(const uint16_t& From)
    {
        return From >> 8;
    }
    template <>
    INLINE uint8_t ConvertRangesAware<uint8_t, int16_t>(const int16_t& From)
    {
        return int16_to_uint16(From) >> 8;
    }
    template <>
    INLINE uint8_t ConvertRangesAware<uint8_t, uint32_t>(const uint32_t& From)
    {
        return From >> 24;
    }
    template <>
    INLINE uint8_t ConvertRangesAware<uint8_t, int32_t>(const int32_t& From)
    {
        return int32_to_uint32(From) >> 24;
    }
    template <>
    INLINE uint8_t ConvertRangesAware<uint8_t, uint64_t>(const uint64_t& From)
    {
        return From >> 56;
    }
    template <>
    INLINE uint8_t ConvertRangesAware<uint8_t, int64_t>(const int64_t& From)
    {
        return int64_to_uint64(From) >> 56;
    }
    template <>
    INLINE uint8_t ConvertRangesAware<uint8_t, float>(const float& From)
    {
        return static_cast<uint8_t>(Math::Saturate(From) * static_cast<float>(UINT8_MAX));
    }
    template <>
    INLINE uint8_t ConvertRangesAware<uint8_t, double>(const double& From)
    {
        return static_cast<uint8_t>(Math::Saturate(From) * static_cast<double>(UINT8_MAX));
    }
    
    template <>
    INLINE int8_t ConvertRangesAware<int8_t, uint8_t>(const uint8_t& From)
    {
        return uint8_to_int8(From);
    }
    template <> 
    INLINE int8_t ConvertRangesAware<int8_t, int8_t>(const int8_t& From)
    {
        return From;
    }
    template <>
    INLINE int8_t ConvertRangesAware<int8_t, uint16_t>(const uint16_t& From)
    {
        return uint8_to_int8(From >> 8);
    }
    template <>
    INLINE int8_t ConvertRangesAware<int8_t, int16_t>(const int16_t& From)
    {
        return From >> 8;
    }
    template <>
    INLINE int8_t ConvertRangesAware<int8_t, uint32_t>(const uint32_t& From)
    {
        return uint8_to_int8(From >> 24);
    }
    template <>
    INLINE int8_t ConvertRangesAware<int8_t, int32_t>(const int32_t& From)
    {
        return From >> 24;
    }
    template <>
    INLINE int8_t ConvertRangesAware<int8_t, uint64_t>(const uint64_t& From)
    {
        return uint8_to_int8(From >> 56);
    }
    template <>
    INLINE int8_t ConvertRangesAware<int8_t, int64_t>(const int64_t& From)
    {
        return From >> 56;
    }
    template <>
    INLINE int8_t ConvertRangesAware<int8_t, float>(const float& From)
    {
        return uint8_to_int8(static_cast<uint8_t>(Math::Saturate(From) * static_cast<float>(UINT8_MAX)));
    }
    template <>
    INLINE int8_t ConvertRangesAware<int8_t, double>(const double& From)
    {
        return uint8_to_int8(static_cast<uint8_t>(Math::Saturate(From) * static_cast<double>(UINT8_MAX)));
    }
    
    template <>
    INLINE uint16_t ConvertRangesAware<uint16_t, uint8_t>(const uint8_t& From)
    {
        return From << 8;
    }
    template <>
    INLINE uint16_t ConvertRangesAware<uint16_t, int8_t>(const int8_t& From)
    {
        return static_cast<uint16_t>(int8_to_uint8(From)) << 8;
    }
    template <> 
    INLINE uint16_t ConvertRangesAware<uint16_t, uint16_t>(const uint16_t& From)
    {
        return From;
    }
    template <>
    INLINE uint16_t ConvertRangesAware<uint16_t, int16_t>(const int16_t& From)
    {
        return int16_to_uint16(From);
    }
    template <>
    INLINE uint16_t ConvertRangesAware<uint16_t, uint32_t>(const uint32_t& From)
    {
        return From >> 16;
    }
    template <>
    INLINE uint16_t ConvertRangesAware<uint16_t, int32_t>(const int32_t& From)
    {
        return int32_to_uint32(From) >> 16;
    }
    template <>
    INLINE uint16_t ConvertRangesAware<uint16_t, uint64_t>(const uint64_t& From)
    {
        return From >> 48;
    }
    template <>
    INLINE uint16_t ConvertRangesAware<uint16_t, int64_t>(const int64_t& From)
    {
        return int64_to_uint64(From) >> 48;
    }
    template <>
    INLINE uint16_t ConvertRangesAware<uint16_t, float>(const float& From)
    {
        return static_cast<uint16_t>(Math::Saturate(From) * static_cast<float>(UINT16_MAX));
    }
    template <>
    INLINE uint16_t ConvertRangesAware<uint16_t, double>(const double& From)
    {
        return static_cast<uint16_t>(Math::Saturate(From) * static_cast<double>(UINT16_MAX));
    }
    
    template <>
    INLINE int16_t ConvertRangesAware<int16_t, uint8_t>(const uint8_t& From)
    {
        return static_cast<int16_t>(uint8_to_int8(From)) << 8u;
    }
    template <>
    INLINE int16_t ConvertRangesAware<int16_t, int8_t>(const int8_t& From)
    {
        return static_cast<uint16_t>(From) << 8u;
    }
    template <>
    INLINE int16_t ConvertRangesAware<int16_t, uint16_t>(const uint16_t& From)
    {
        return uint16_to_int16(From);
    }
    template <> 
    INLINE int16_t ConvertRangesAware<int16_t, int16_t>(const int16_t& From)
    {
        return From;
    }
    template <> 
    INLINE int16_t ConvertRangesAware<int16_t, uint32_t>(const uint32_t& From)
    {
        return uint16_to_int16(From >> 16);
    }
    template <> 
    INLINE int16_t ConvertRangesAware<int16_t, int32_t>(const int32_t& From)
    {
        return (From >> 16);
    }
    template <> 
    INLINE int16_t ConvertRangesAware<int16_t, uint64_t>(const uint64_t& From)
    {
        return uint16_to_int16(From >> 48);
    }
    template <> 
    INLINE int16_t ConvertRangesAware<int16_t, int64_t>(const int64_t& From)
    {
        return (From >> 48);
    }
    template <> 
    INLINE int16_t ConvertRangesAware<int16_t, float>(const float& From)
    {
        return uint16_to_int16(static_cast<uint16_t>(Math::Saturate(From) * static_cast<float>(UINT16_MAX)));
    }
    template <> 
    INLINE int16_t ConvertRangesAware<int16_t, double>(const double& From)
    {
        return uint16_to_int16(static_cast<uint16_t>(Math::Saturate(From) * static_cast<double>(UINT16_MAX)));
    }
    
    template <> 
    INLINE uint32_t ConvertRangesAware<uint32_t, uint8_t>(const uint8_t& From)
    {
        return From << 24;
    }
    template <> 
    INLINE uint32_t ConvertRangesAware<uint32_t, int8_t>(const int8_t& From)
    {
        return uint8_to_int8(From) << 24;
    }
    template <> 
    INLINE uint32_t ConvertRangesAware<uint32_t, uint16_t>(const uint16_t& From)
    {
        return From << 16;
    }
    template <> 
    INLINE uint32_t ConvertRangesAware<uint32_t, int16_t>(const int16_t& From)
    {
        return int16_to_uint16(From) << 16;
    }
    template <> 
    INLINE uint32_t ConvertRangesAware<uint32_t, uint32_t>(const uint32_t& From)
    {
        return From;
    }
    template <> 
    INLINE uint32_t ConvertRangesAware<uint32_t, int32_t>(const int32_t& From)
    {
        return int32_to_uint32(From);
    }
    template <> 
    INLINE uint32_t ConvertRangesAware<uint32_t, uint64_t>(const uint64_t& From)
    {
        return From >> 32;
    }
    template <> 
    INLINE uint32_t ConvertRangesAware<uint32_t, int64_t>(const int64_t& From)
    {
        return int64_to_uint64(From) >> 32;
    }
    template <> 
    INLINE uint32_t ConvertRangesAware<uint32_t, float>(const float& From)
    {
        return static_cast<uint32_t>(Math::Saturate(From) * static_cast<float>(UINT32_MAX));
    }
    template <> 
    INLINE uint32_t ConvertRangesAware<uint32_t, double>(const double& From)
    {
        return static_cast<uint32_t>(Math::Saturate(From) * static_cast<double>(UINT32_MAX));
    }
    
    template <> 
    INLINE int32_t ConvertRangesAware<int32_t, uint8_t>(const uint8_t& From)
    {
        return uint8_to_int8(From) << 24;
    }
    template <> 
    INLINE int32_t ConvertRangesAware<int32_t, int8_t>(const int8_t& From)
    {
        return From << 24;
    }
    template <> 
    INLINE int32_t ConvertRangesAware<int32_t, uint16_t>(const uint16_t& From)
    {
        return uint16_to_int16(From) << 16;
    }
    template <> 
    INLINE int32_t ConvertRangesAware<int32_t, int16_t>(const int16_t& From)
    {
        return From << 16;
    }
    template <> 
    INLINE int32_t ConvertRangesAware<int32_t, uint32_t>(const uint32_t& From)
    {
        return uint32_to_int32(From);
    }
    template <> 
    INLINE int32_t ConvertRangesAware<int32_t, int32_t>(const int32_t& From)
    {
        return From;
    }
    template <> 
    INLINE int32_t ConvertRangesAware<int32_t, uint64_t>(const uint64_t& From)
    {
        return uint64_to_int64(From) >> 32;
    }
    template <> 
    INLINE int32_t ConvertRangesAware<int32_t, int64_t>(const int64_t& From)
    {
        return From >> 32;
    }
    template <> 
    INLINE int32_t ConvertRangesAware<int32_t, float>(const float& From)
    {
        return uint32_to_int32(static_cast<uint32_t>(Math::Saturate(From) * static_cast<float>(UINT32_MAX)));
    }
    template <> 
    INLINE int32_t ConvertRangesAware<int32_t, double>(const double& From)
    {
        return uint32_to_int32(static_cast<uint32_t>(Math::Saturate(From) * static_cast<double>(UINT32_MAX)));
    }
    
    template <> 
    INLINE uint64_t ConvertRangesAware<uint64_t, uint8_t>(const uint8_t& From)
    {
        return From << 56;
    }
    template <> 
    INLINE uint64_t ConvertRangesAware<uint64_t, int8_t>(const int8_t& From)
    {
        return int8_to_uint8(From) << 56;
    }
    template <> 
    INLINE uint64_t ConvertRangesAware<uint64_t, uint16_t>(const uint16_t& From)
    {
        return From << 48;
    }
    template <> 
    INLINE uint64_t ConvertRangesAware<uint64_t, int16_t>(const int16_t& From)
    {
        return uint16_to_int16(From) << 48;
    }
    template <> 
    INLINE uint64_t ConvertRangesAware<uint64_t, uint32_t>(const uint32_t& From)
    {
        return From << 32;
    }
    template <> 
    INLINE uint64_t ConvertRangesAware<uint64_t, int32_t>(const int32_t& From)
    {
        return uint32_to_int32(From) << 32;
    }
    template <> 
    INLINE uint64_t ConvertRangesAware<uint64_t, uint64_t>(const uint64_t& From)
    {
        return From;
    }
    template <> 
    INLINE uint64_t ConvertRangesAware<uint64_t, int64_t>(const int64_t& From)
    {
        return uint64_to_int64(From);
    }
    template <> 
    INLINE uint64_t ConvertRangesAware<uint64_t, float>(const float& From)
    {
        return static_cast<uint64_t>(Math::Saturate(From) * static_cast<float>(UINT64_MAX));
    }
    template <> 
    INLINE uint64_t ConvertRangesAware<uint64_t, double>(const double& From)
    {
        return static_cast<uint64_t>(Math::Saturate(From) * static_cast<double>(UINT64_MAX));
    }
    
    template <> 
    INLINE int64_t ConvertRangesAware<int64_t, uint8_t>(const uint8_t& From)
    {
        return uint8_to_int8(From) << 56;
    }
    template <> 
    INLINE int64_t ConvertRangesAware<int64_t, int8_t>(const int8_t& From)
    {
        return From << 56;
    }
    template <> 
    INLINE int64_t ConvertRangesAware<int64_t, uint16_t>(const uint16_t& From)
    {
        return uint16_to_int16(From) << 48;
    }
    template <> 
    INLINE int64_t ConvertRangesAware<int64_t, int16_t>(const int16_t& From)
    {
        return From << 48;
    }
    template <> 
    INLINE int64_t ConvertRangesAware<int64_t, uint32_t>(const uint32_t& From)
    {
        return uint32_to_int32(From) << 32;
    }
    template <> 
    INLINE int64_t ConvertRangesAware<int64_t, int32_t>(const int32_t& From)
    {
        return From << 32;
    }
    template <> 
    INLINE int64_t ConvertRangesAware<int64_t, uint64_t>(const uint64_t& From)
    {
        return uint64_to_int64(From);
    }
    template <> 
    INLINE int64_t ConvertRangesAware<int64_t, int64_t>(const int64_t& From)
    {
        return From;
    }
    template <> 
    INLINE int64_t ConvertRangesAware<int64_t, float>(const float& From)
    {
        return uint64_to_int64(static_cast<uint64_t>(Math::Saturate(From) * static_cast<float>(UINT64_MAX)));
    }
    template <> 
    INLINE int64_t ConvertRangesAware<int64_t, double>(const double& From)
    {
        return uint64_to_int64(static_cast<uint64_t>(Math::Saturate(From) * static_cast<double>(UINT64_MAX)));
    }
    
    template <> 
    INLINE float ConvertRangesAware<float, uint8_t>(const uint8_t& From)
    {
        return static_cast<float>(From) / static_cast<float>(UINT8_MAX);
    }
    template <> 
    INLINE float ConvertRangesAware<float, int8_t>(const int8_t& From)
    {
        return static_cast<float>(int8_to_uint8(From)) / static_cast<float>(UINT8_MAX);
    }
    template <> 
    INLINE float ConvertRangesAware<float, uint16_t>(const uint16_t& From)
    {
        return static_cast<float>(From) / static_cast<float>(UINT16_MAX);
    }
    template <> 
    INLINE float ConvertRangesAware<float, int16_t>(const int16_t& From)
    {
        return static_cast<float>(int16_to_uint16(From)) / static_cast<float>(UINT16_MAX);
    }
    template <> 
    INLINE float ConvertRangesAware<float, uint32_t>(const uint32_t& From)
    {
        return static_cast<float>(From) / static_cast<float>(UINT32_MAX);
    }
    template <> 
    INLINE float ConvertRangesAware<float, int32_t>(const int32_t& From)
    {
        return static_cast<float>(int32_to_uint32(From)) / static_cast<float>(UINT32_MAX);
    }
    template <> 
    INLINE float ConvertRangesAware<float, uint64_t>(const uint64_t& From)
    {
        return static_cast<float>(From) / static_cast<float>(UINT64_MAX);
    }
    template <> 
    INLINE float ConvertRangesAware<float, int64_t>(const int64_t& From)
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
    
    template <> 
    INLINE double ConvertRangesAware<double, uint8_t>(const uint8_t& From)
    {
        return static_cast<double>(From) / static_cast<double>(UINT8_MAX);
    }
    template <> 
    INLINE double ConvertRangesAware<double, int8_t>(const int8_t& From)
    {
        return static_cast<double>(int8_to_uint8(From)) / static_cast<double>(UINT8_MAX);
    }
    template <> 
    INLINE double ConvertRangesAware<double, uint16_t>(const uint16_t& From)
    {
        return static_cast<double>(From) / static_cast<double>(UINT16_MAX);
    }
    template <> 
    INLINE double ConvertRangesAware<double, int16_t>(const int16_t& From)
    {
        return static_cast<double>(int16_to_uint16(From)) / static_cast<double>(UINT16_MAX);
    }
    template <> 
    INLINE double ConvertRangesAware<double, uint32_t>(const uint32_t& From)
    {
        return static_cast<double>(From) / static_cast<double>(UINT32_MAX);
    }
    template <> 
    INLINE double ConvertRangesAware<double, int32_t>(const int32_t& From)
    {
        return static_cast<double>(int32_to_uint32(From)) / static_cast<double>(UINT32_MAX);
    }
    template <> 
    INLINE double ConvertRangesAware<double, uint64_t>(const uint64_t& From)
    {
        return static_cast<double>(From) / static_cast<double>(UINT64_MAX);
    }
    template <> 
    INLINE double ConvertRangesAware<double, int64_t>(const int64_t& From)
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
    
    INLINE float Decode(float Encoded, Image::Encoding Encoding)
    {
        switch (Encoding)
        {
        case Image::Linear:
        case Image::Unencoded:
            break;

        case Image::sRGB:
            Encoded = sRGB::EOTF(Encoded);
            break;

        case Image::LogC:
            Encoded = ArriLogC::GArriLogCToGLinear(Encoded);
            break;

        case Image::PQ:
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported encoding")
        }
        return Encoded;
    }
    
    INLINE Math::Vector2f Decode(Math::Vector2f Encoded, Image::Encoding Encoding)
    {
        switch (Encoding)
        {
        case Image::Linear:
        case Image::Unencoded:
            break;

        case Image::sRGB:
            Encoded.x = sRGB::EOTF(Encoded.x);
            Encoded.y = sRGB::EOTF(Encoded.y);
            break;

        case Image::LogC:
            Encoded.x = ArriLogC::GArriLogCToGLinear(Encoded.x);
            Encoded.y = ArriLogC::GArriLogCToGLinear(Encoded.y);
            break;

        case Image::PQ:
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported encoding")
        }
        return Encoded;
    }

    INLINE Math::Vector3f Decode(Math::Vector3f Encoded, Image::Encoding Encoding)
    {
        switch (Encoding)
        {
        case Image::Linear:
        case Image::Unencoded:
            break;

        case Image::sRGB:
            Encoded.x = sRGB::EOTF(Encoded.x);
            Encoded.y = sRGB::EOTF(Encoded.y);
            Encoded.z = sRGB::EOTF(Encoded.z);
            break;

        case Image::LogC:
            Encoded.x = ArriLogC::GArriLogCToGLinear(Encoded.x);
            Encoded.y = ArriLogC::GArriLogCToGLinear(Encoded.y);
            Encoded.z = ArriLogC::GArriLogCToGLinear(Encoded.z);
            break;

        case Image::PQ:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported encoding")
        }
        return Encoded;
    }
    
    INLINE Math::Vector4f Decode(Math::Vector4f Encoded, Image::Encoding Encoding)
    {
        switch (Encoding)
        {
        case Image::Linear:
        case Image::Unencoded:
            break;

        case Image::sRGB:
            Encoded.x = sRGB::EOTF(Encoded.x);
            Encoded.y = sRGB::EOTF(Encoded.y);
            Encoded.z = sRGB::EOTF(Encoded.z);
            break;

        case Image::LogC:
            Encoded.x = ArriLogC::GArriLogCToGLinear(Encoded.x);
            Encoded.y = ArriLogC::GArriLogCToGLinear(Encoded.y);
            Encoded.z = ArriLogC::GArriLogCToGLinear(Encoded.z);
            break;

        case Image::PQ:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported encoding")
        }
        return Encoded;
    }
    
    INLINE float Encode(float Linear, Image::Encoding Encoding)
    {
        switch (Encoding)
        {
        case Image::Linear:
        case Image::Unencoded:
            break;

        case Image::sRGB:
            Linear = sRGB::OETF(Linear);
            break;

        case Image::LogC:
            Linear = ArriLogC::GLinearToGArriLogC(Linear);
            break;

        case Image::PQ:
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported encoding")
            }
        return Linear;
    }
    
    INLINE Math::Vector2f Encode(Math::Vector2f Linear, Image::Encoding Encoding)
    {
        switch (Encoding)
        {
        case Image::Linear:
        case Image::Unencoded:
            break;

        case Image::sRGB:
            Linear.x = sRGB::OETF(Linear.x);
            Linear.y = sRGB::OETF(Linear.y);
            break;

        case Image::LogC:
            Linear.x = ArriLogC::GLinearToGArriLogC(Linear.x);
            Linear.y = ArriLogC::GLinearToGArriLogC(Linear.y);
            break;

        case Image::PQ:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported encoding")
        }
        return Linear;
    }

    INLINE Math::Vector3f Encode(Math::Vector3f Linear, Image::Encoding Encoding)
    {
        switch (Encoding)
        {
        case Image::Linear:
        case Image::Unencoded:
            break;

        case Image::sRGB:
            Linear.x = sRGB::OETF(Linear.x);
            Linear.y = sRGB::OETF(Linear.y);
            Linear.z = sRGB::OETF(Linear.z);
            break;

        case Image::LogC:
            Linear.x = ArriLogC::GLinearToGArriLogC(Linear.x);
            Linear.y = ArriLogC::GLinearToGArriLogC(Linear.y);
            Linear.z = ArriLogC::GLinearToGArriLogC(Linear.z);
            break;

        case Image::PQ:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported encoding")
        }
        return Linear;
    }
    
    INLINE Math::Vector4f Encode(Math::Vector4f Linear, Image::Encoding Encoding)
    {
        switch (Encoding)
        {
        case Image::Linear:
        case Image::Unencoded:
            break;

        case Image::sRGB:
            Linear.x = sRGB::OETF(Linear.x);
            Linear.y = sRGB::OETF(Linear.y);
            Linear.z = sRGB::OETF(Linear.z);
            break;

        case Image::LogC:
            Linear.x = ArriLogC::GLinearToGArriLogC(Linear.x);
            Linear.y = ArriLogC::GLinearToGArriLogC(Linear.y);
            Linear.z = ArriLogC::GLinearToGArriLogC(Linear.z);
            break;

        case Image::PQ:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported encoding")
        }
        return Linear;
    }
    
        
    INLINE double Decode(double Encoded, Image::Encoding Encoding)
    {
        switch (Encoding)
        {
        case Image::Linear:
        case Image::Unencoded:
            break;

        case Image::sRGB:
            Encoded = sRGB::EOTF(Encoded);
            break;

        case Image::LogC:
            Encoded = ArriLogC::GArriLogCToGLinear(Encoded);
            break;

        case Image::PQ:
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported encoding")
        }
        return Encoded;
    }
    
    INLINE Math::Vector2d Decode(Math::Vector2d Encoded, Image::Encoding Encoding)
    {
        switch (Encoding)
        {
        case Image::Linear:
        case Image::Unencoded:
            break;

        case Image::sRGB:
            Encoded.x = sRGB::EOTF(Encoded.x);
            Encoded.y = sRGB::EOTF(Encoded.y);
            break;

        case Image::LogC:
            Encoded.x = ArriLogC::GArriLogCToGLinear(Encoded.x);
            Encoded.y = ArriLogC::GArriLogCToGLinear(Encoded.y);
            break;

        case Image::PQ:
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported encoding")
        }
        return Encoded;
    }

    INLINE Math::Vector3d Decode(Math::Vector3d Encoded, Image::Encoding Encoding)
    {
        switch (Encoding)
        {
        case Image::Linear:
        case Image::Unencoded:
            break;

        case Image::sRGB:
            Encoded.x = sRGB::EOTF(Encoded.x);
            Encoded.y = sRGB::EOTF(Encoded.y);
            Encoded.z = sRGB::EOTF(Encoded.z);
            break;

        case Image::LogC:
            Encoded.x = ArriLogC::GArriLogCToGLinear(Encoded.x);
            Encoded.y = ArriLogC::GArriLogCToGLinear(Encoded.y);
            Encoded.z = ArriLogC::GArriLogCToGLinear(Encoded.z);
            break;

        case Image::PQ:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported encoding")
        }
        return Encoded;
    }
    
    INLINE Math::Vector4d Decode(Math::Vector4d Encoded, Image::Encoding Encoding)
    {
        switch (Encoding)
        {
        case Image::Linear:
        case Image::Unencoded:
            break;

        case Image::sRGB:
            Encoded.x = sRGB::EOTF(Encoded.x);
            Encoded.y = sRGB::EOTF(Encoded.y);
            Encoded.z = sRGB::EOTF(Encoded.z);
            break;

        case Image::LogC:
            Encoded.x = ArriLogC::GArriLogCToGLinear(Encoded.x);
            Encoded.y = ArriLogC::GArriLogCToGLinear(Encoded.y);
            Encoded.z = ArriLogC::GArriLogCToGLinear(Encoded.z);
            break;

        case Image::PQ:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported encoding")
        }
        return Encoded;
    }
    
    INLINE double Encode(double Linear, Image::Encoding Encoding)
    {
        switch (Encoding)
        {
        case Image::Linear:
        case Image::Unencoded:
            break;

        case Image::sRGB:
            Linear = sRGB::OETF(Linear);
            break;

        case Image::LogC:
            Linear = ArriLogC::GLinearToGArriLogC(Linear);
            break;

        case Image::PQ:
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported encoding")
            }
        return Linear;
    }
    
    INLINE Math::Vector2d Encode(Math::Vector2d Linear, Image::Encoding Encoding)
    {
        switch (Encoding)
        {
        case Image::Linear:
        case Image::Unencoded:
            break;

        case Image::sRGB:
            Linear.x = sRGB::OETF(Linear.x);
            Linear.y = sRGB::OETF(Linear.y);
            break;

        case Image::LogC:
            Linear.x = ArriLogC::GLinearToGArriLogC(Linear.x);
            Linear.y = ArriLogC::GLinearToGArriLogC(Linear.y);
            break;

        case Image::PQ:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported encoding")
        }
        return Linear;
    }

    INLINE Math::Vector3d Encode(Math::Vector3d Linear, Image::Encoding Encoding)
    {
        switch (Encoding)
        {
        case Image::Linear:
        case Image::Unencoded:
            break;

        case Image::sRGB:
            Linear.x = sRGB::OETF(Linear.x);
            Linear.y = sRGB::OETF(Linear.y);
            Linear.z = sRGB::OETF(Linear.z);
            break;

        case Image::LogC:
            Linear.x = ArriLogC::GLinearToGArriLogC(Linear.x);
            Linear.y = ArriLogC::GLinearToGArriLogC(Linear.y);
            Linear.z = ArriLogC::GLinearToGArriLogC(Linear.z);
            break;

        case Image::PQ:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported encoding")
        }
        return Linear;
    }
    
    INLINE Math::Vector4d Encode(Math::Vector4d Linear, Image::Encoding Encoding)
    {
        switch (Encoding)
        {
        case Image::Linear:
        case Image::Unencoded:
            break;

        case Image::sRGB:
            Linear.x = sRGB::OETF(Linear.x);
            Linear.y = sRGB::OETF(Linear.y);
            Linear.z = sRGB::OETF(Linear.z);
            break;

        case Image::LogC:
            Linear.x = ArriLogC::GLinearToGArriLogC(Linear.x);
            Linear.y = ArriLogC::GLinearToGArriLogC(Linear.y);
            Linear.z = ArriLogC::GLinearToGArriLogC(Linear.z);
            break;

        case Image::PQ:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported encoding")
        }
        return Linear;
    }
    
    INLINE float ApplyWarping(float Coordinate, ImageSampler::WarpMode WarpMode)
    {
        switch (WarpMode)
        {
        case ImageSampler::WM_Repeat:
            return abs(fmodf(Coordinate, 1.0f));
            
        case ImageSampler::WM_MirrorRepeat:
            {
                float mod = fmodf(Coordinate, 2.0f);
                
                mod = mod < 0 ? mod + 2.0f : mod;
                
                return mod > 1.0f ? 2.0f - mod : mod;
            }
            
        case ImageSampler::WM_ClampToEdge:
        case ImageSampler::WM_ClampToBorder:
            return Math::Clamp(Coordinate, 0.f, 1.f);
            
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported sampler warp mode")
        }
    }
}
