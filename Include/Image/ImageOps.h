#pragma once

#include <cstdint>

#include "Math/Vector.h"
#include "Image.h"

#ifndef CONFIG_RELEASE
#include "Shared/Assertion.h"
#endif // CONFIG_RELEASE

template<typename TexelType> 
struct ImageBuffer
{
    using Type = TexelType;
    static constexpr bool IsScalarType = std::is_arithmetic_v<Type>;
    static constexpr bool IsMathVector2Type = std::is_same_v<Type, Math::Vector2t<typename Type::Type>>;
    static constexpr bool IsMathVector3Type = std::is_same_v<Type, Math::Vector3t<typename Type::Type>>;
    static constexpr bool IsMathVector4Type = std::is_same_v<Type, Math::Vector4t<typename Type::Type>>;
    static constexpr bool IsMathVectorType = (IsMathVector2Type || IsMathVector3Type || IsMathVector3Type);
    
    ImageBuffer(const Image& Image);
    ImageBuffer(const ImageBuffer& Other);
    ImageBuffer& operator=(const ImageBuffer& Other);
    ~ImageBuffer() = default;

    uint32_t Width() const { return m_Width; }
    uint32_t Height() const { return m_Height; }
    Image::Type ComponentType() const { return m_ComponentType; }
    Image::Layout ComponentLayout() const { return m_ComponentLayout; }
    Image::Encoding ComponentEncoding() const { return m_ComponentEncoding; }

    uint32_t ChannelSize() const {return Image::ChannelSize(m_ComponentType);}
    uint32_t PixelSize() const {return Image::PixelSize(m_ComponentType, m_ComponentLayout);}
    uint32_t ComponentCount() const {return Image::ComponentCount(m_ComponentLayout);}
    size_t DataSize() const {return m_Width * m_Height * PixelSize();}
    void* Data() const { return m_Data; }
    
    TexelType Read(uint32_t x, uint32_t y) const;
    void Write(uint32_t x, uint32_t y, TexelType data);

private:
    TexelType* m_Data;
    uint32_t m_Width;
    uint32_t m_Height;
    Image::Layout m_ComponentLayout;
    Image::Type m_ComponentType;
    Image::Encoding m_ComponentEncoding;
};

template <typename TexelType>
ImageBuffer<TexelType>::ImageBuffer(const Image& Image)
{
#ifndef CONFIG_RELEASE
    if constexpr (IsScalarType)
    {
        switch (Image.ComponentType())
        {
        case Image::UnsignedByte:   AssertOrError((std::is_same_v<TexelType, uint8_t>), "Image buffer type missmatch") break;
        case Image::Byte:           AssertOrError((std::is_same_v<TexelType, int8_t>), "Image buffer type missmatch") break;
        case Image::UnsignedShort:  AssertOrError((std::is_same_v<TexelType, uint16_t>), "Image buffer type missmatch") break;
        case Image::Short:          AssertOrError((std::is_same_v<TexelType, int16_t>), "Image buffer type missmatch") break;
        case Image::UnsignedInt:    AssertOrError((std::is_same_v<TexelType, uint32_t>), "Image buffer type missmatch") break;
        case Image::Int:            AssertOrError((std::is_same_v<TexelType, int32_t>), "Image buffer type missmatch") break;
        case Image::Float:          AssertOrError((std::is_same_v<TexelType, float>), "Image buffer type missmatch") break;
        case Image::Double:         AssertOrError((std::is_same_v<TexelType, double>), "Image buffer type missmatch") break;
            
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported image type")
        }
        
        AssertOrError(Image.ComponentLayout() == Image::R, "Image buffer layout missmatch. Image must have only 1 component for this buffer")
    }
    else
    {
        switch (Image.ComponentType())
        {
        case Image::UnsignedByte:   AssertOrError((std::is_same_v<typename TexelType::Type, uint8_t>), "Image buffer type missmatch") break;
        case Image::Byte:           AssertOrError((std::is_same_v<typename TexelType::Type, int8_t>), "Image buffer type missmatch") break;
        case Image::UnsignedShort:  AssertOrError((std::is_same_v<typename TexelType::Type, uint16_t>), "Image buffer type missmatch") break;
        case Image::Short:          AssertOrError((std::is_same_v<typename TexelType::Type, int16_t>), "Image buffer type missmatch") break;
        case Image::UnsignedInt:    AssertOrError((std::is_same_v<typename TexelType::Type, uint32_t>), "Image buffer type missmatch") break;
        case Image::Int:            AssertOrError((std::is_same_v<typename TexelType::Type, int32_t>), "Image buffer type missmatch") break;
        case Image::Float:          AssertOrError((std::is_same_v<typename TexelType::Type, float>), "Image buffer type missmatch") break;
        case Image::Double:         AssertOrError((std::is_same_v<typename TexelType::Type, double>), "Image buffer type missmatch") break;
            
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported image type")
        }
        
        switch (Image.ComponentLayout())
        {
        case Image::R:
            AssertOrErrorF(false, "Image buffer layout missmatch. Image must have only %u component for this buffer", TexelType::ComponentCount)
            break;
            
        case Image::RG:
            AssertOrErrorF(IsMathVector2Type, "Image buffer layout missmatch. Image must have only %u component for this buffer", TexelType::ComponentCount)
            break;
            
        case Image::RGB:
        case Image::BGR:
            AssertOrErrorF(IsMathVector3Type, "Image buffer layout missmatch. Image must have only %u component for this buffer", TexelType::ComponentCount)
            break;
            
        case Image::RGBA:
        case Image::ARGB:
        case Image::ABGR:
            AssertOrErrorF(IsMathVector4Type, "Image buffer layout missmatch. Image must have only %u component for this buffer", TexelType::ComponentCount)
            break;
            
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported image layout")
        }
    }
    
    AssertOrError(Image.PixelSize() == sizeof(TexelType), "Buffer pixel size missmatch")
#endif // CONFIG_RELEASE
    
    m_Data = static_cast<TexelType*>(Image.Data());
    m_Width = Image.Width();
    m_Height = Image.Height();
    m_ComponentLayout = Image.ComponentLayout();
    m_ComponentType = Image.ComponentType();
    m_ComponentEncoding = Image.ComponentEncoding();
}

template <typename TexelType>
ImageBuffer<TexelType>::ImageBuffer(const ImageBuffer& Other): 
    m_Data(Other.m_Data),
    m_Width(Other.m_Width),
    m_Height(Other.m_Height),
    m_ComponentLayout(Other.m_ComponentLayout),
    m_ComponentType(Other.m_ComponentType),
    m_ComponentEncoding(Other.m_ComponentEncoding)
{
}

template <typename TexelType>
ImageBuffer<TexelType>& ImageBuffer<TexelType>::operator=(const ImageBuffer& Other)
{
    if (this == &Other)
        return *this;
    m_Data = Other.m_Data;
    m_Width = Other.m_Width;
    m_Height = Other.m_Height;
    m_ComponentLayout = Other.m_ComponentLayout;
    m_ComponentType = Other.m_ComponentType;
    m_ComponentEncoding = Other.m_ComponentEncoding;
    return *this;
}

template <typename TexelType>
TexelType ImageBuffer<TexelType>::Read(uint32_t x, uint32_t y) const
{
#ifndef CONFIG_RELEASE
    AssertOrError(x < m_Width && y < m_Height, "Pixel index out of range")
#endif // CONFIG_RELEASE
    
    if constexpr (IsMathVector3Type)
    {
        switch (m_ComponentLayout)
        {
        case Image::R:
        case Image::RG:
        case Image::RGBA:
        case Image::ARGB:
        case Image::ABGR:
            UNREACHABLE;
            break;
            
        case Image::RGB: 
            return m_Data[y * m_Width + x];
            
        case Image::BGR:
            TexelType sample;
            
            sample.x = m_Data[y * m_Width + x].z;
            sample.y = m_Data[y * m_Width + x].y;
            sample.z = m_Data[y * m_Width + x].x;
            
            return sample;
        }
    }
    
    if constexpr (IsMathVector4Type)
    {
        switch (m_ComponentLayout)
        {
        case Image::R:
        case Image::RG:
        case Image::RGB:
        case Image::BGR:
            UNREACHABLE;
            break;
            
        case Image::RGBA:
            return m_Data[y * m_Width + x];
            
        case Image::ARGB:
            {
                TexelType sample;
            
                sample.w = m_Data[y * m_Width + x].x;
                sample.x = m_Data[y * m_Width + x].y;
                sample.y = m_Data[y * m_Width + x].z;
                sample.z = m_Data[y * m_Width + x].w;
            
                return sample;
            }
            break;
            
        case Image::ABGR:
            {
                TexelType sample;
            
                sample.w = m_Data[y * m_Width + x].x;
                sample.x = m_Data[y * m_Width + x].w;
                sample.y = m_Data[y * m_Width + x].z;
                sample.z = m_Data[y * m_Width + x].x;
            
                return sample;
            }
            break;
        }
    }
        
    return m_Data[y * m_Width + x];
}

template <typename TexelType>
void ImageBuffer<TexelType>::Write(uint32_t x, uint32_t y, TexelType data)
{
#ifndef CONFIG_RELEASE
    AssertOrError(x < m_Width && y < m_Height, "Pixel index out of range")
#endif // CONFIG_RELEASE
    
    if constexpr (IsMathVector3Type)
    {
        switch (m_ComponentLayout)
        {
        case Image::R:
        case Image::RG:
        case Image::RGBA:
        case Image::ARGB:
        case Image::ABGR:
            UNREACHABLE;
            break;
            
        case Image::RGB:
            break;
            
        case Image::BGR:
            m_Data[y * m_Width + x].z = data.x;
            m_Data[y * m_Width + x].y = data.y;
            m_Data[y * m_Width + x].x = data.z;
            break;
        }
    }
    
    if constexpr (IsMathVector4Type)
    {
        switch (m_ComponentLayout)
        {
        case Image::R:
        case Image::RG:
        case Image::RGB:
        case Image::BGR:
            UNREACHABLE;
            break;
            
        case Image::RGBA:
            break;
            
        case Image::ARGB:       
            m_Data[y * m_Width + x].x = data.w;
            m_Data[y * m_Width + x].y = data.x;
            m_Data[y * m_Width + x].z = data.y;
            m_Data[y * m_Width + x].w = data.z;
            break;
            
        case Image::ABGR:       
            m_Data[y * m_Width + x].x = data.w;
            m_Data[y * m_Width + x].w = data.x;
            m_Data[y * m_Width + x].z = data.y;
            m_Data[y * m_Width + x].x = data.z;
            break;
        }
    }
    
    m_Data[y * m_Width + x] = data;
}

void ClearBuffer(ImageBuffer<Math::Vector3t<uint8_t>>& ImageBuffer);
Math::Vector3f ReadBuffer(const ImageBuffer<Math::Vector3t<uint8_t>>& ImageBuffer, uint32_t x, uint32_t y);
void WriteBuffer(ImageBuffer<Math::Vector3t<uint8_t>>& ImageBuffer, uint32_t x, uint32_t y, Math::Vector3f data);