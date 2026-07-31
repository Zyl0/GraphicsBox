#include "Image.h"

#include <cstring>
#include <fstream>
#include <vector>
#include <stb_image.h>
#include <stb_image_write.h>
#include <tinyddsloader.h>
#include <bcdec.h>
#include <tinyexr.h>

#include "Shared/Annotations.h"
#include "Shared/Assertion.h"

uint32_t Image::ChannelSize(Type ComponentType)
{
    switch (ComponentType)
    {
    case UnsignedByte:
    case Byte:          return 1;
        
    case UnsignedShort:
    case Short:         return 2;
        
    case UnsignedInt:
    case Int:
    case Float:         return 4;
    case Double:        return 8;

    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported channel type")
    }
}

uint32_t Image::ComponentCount(Layout ComponentLayout)
{
    switch (ComponentLayout)
    {
    case R:         return 1;
    case RG:        return 2;
    case RGB:
    case BGR:       return 3;
    case RGBA:
    case ARGB:
    case ABGR:      return 4;

    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported channel layout")
    }
}

uint32_t Image::PixelSize(Type ComponentType, Layout ComponentLayout)
{
    return ComponentCount(ComponentLayout) * ChannelSize(ComponentType);
}

Image::Image():
    m_Width(0), m_Height(0),
    m_ComponentType(Byte),
    m_ComponentLayout(R),
    m_Data(nullptr)
{
}

Image::Image(uint32_t Width, uint32_t Height, Type ComponentType, Layout ComponentLayout, Encoding ComponentEncoding, const void* Data) :
    m_Width(Width), m_Height(Height),
    m_ComponentType(ComponentType),
    m_ComponentLayout(ComponentLayout),
    m_ComponentEncoding(ComponentEncoding),
    m_Data(m_Width * m_Height > 0 ? malloc(DataSize()) : nullptr)
{
    if (Data != nullptr && m_Data != nullptr)
    {
        memcpy(m_Data, Data, DataSize());
    }
}

Image::~Image()
{
    if (m_Data == nullptr) return;

    free(m_Data);
}

Image::Image(const Image& Other): 
    m_Width(Other.m_Width),
    m_Height(Other.m_Height),
    m_ComponentType(Other.m_ComponentType),
    m_ComponentLayout(Other.m_ComponentLayout),
    m_ComponentEncoding(Other.m_ComponentEncoding),
    m_Data(m_Width * m_Height > 0 ? malloc(DataSize()) : nullptr)
{
    if (m_Data != nullptr && Other.m_Data != nullptr)
    {
        memcpy(m_Data, Other.m_Data, DataSize());
    }
}

Image::Image(Image&& Other) noexcept: 
    m_Width(Other.m_Width),
    m_Height(Other.m_Height),
    m_ComponentType(Other.m_ComponentType),
    m_ComponentLayout(Other.m_ComponentLayout),
    m_ComponentEncoding(Other.m_ComponentEncoding),
    m_Data(Other.m_Data)
{
    Other.m_Data = nullptr;
}

Image& Image::operator=(const Image& Other)
{
    if (this == &Other)
        return *this;
    
    m_Width = Other.m_Width;
    m_Height = Other.m_Height;
    m_ComponentType = Other.m_ComponentType;
    m_ComponentLayout = Other.m_ComponentLayout;
    m_ComponentEncoding = Other.m_ComponentEncoding;
    m_Data = m_Width * m_Height > 0 ? malloc(DataSize()) : nullptr;
    
    if (m_Data != nullptr && Other.m_Data != nullptr)
    {
        memcpy(m_Data, Other.m_Data, DataSize());
    }
    
    return *this;
}

Image& Image::operator=(Image&& Other) noexcept
{
    if (this == &Other)
        return *this;
    
    m_Width = Other.m_Width;
    m_Height = Other.m_Height;
    m_ComponentType = Other.m_ComponentType;
    m_ComponentLayout = Other.m_ComponentLayout;
    m_ComponentEncoding = Other.m_ComponentEncoding;
    m_Data = Other.m_Data;
    
    Other.m_Data = nullptr;
    
    return *this;
}

uint32_t Image::ChannelSize() const
{
    switch (m_ComponentType)
    {
    case UnsignedByte:
    case Byte:          return 1;
        
    case UnsignedShort:
    case Short:         return 2;
        
    case UnsignedInt:
    case Int:
    case Float:         return 4;
    case Double:        return 8;

    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported channel type")
    }
}

uint32_t Image::PixelSize() const
{
    return ComponentCount() * ChannelSize();
}

uint32_t Image::ComponentCount() const
{
    switch (m_ComponentLayout)
    {
    case R:         return 1;
    case RG:        return 2;
    case RGB:
    case BGR:       return 3;
    case RGBA:
    case ARGB:
    case ABGR:      return 4;

    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported channel layout")
    }
}

size_t Image::DataSize() const
{
    return static_cast<size_t>(m_Width) * m_Height * PixelSize();
}

static Image LoadImageDDSFromMemory(const uint8_t* data, size_t size)
{
    using namespace tinyddsloader;
    
    AssertOrErrorCall(data != nullptr && size > 0, return Image(0,0, Image::UnsignedByte, Image::R), 
        "Invalid image data")
    
    DDSFile dds;
    auto ret = dds.Load(data, size);
    
    if (ret == Result::Success && dds.GetMipCount() > 0)
    {
        const DDSFile::ImageData* mip0 = dds.GetImageData(0, 0);
        uint32_t width = mip0->m_width;
        uint32_t height = mip0->m_height;
        DDSFile::DXGIFormat format = dds.GetFormat();

        switch (format)
        {
        case DDSFile::DXGIFormat::R32G32B32A32_Float:
            return Image(width, height, Image::Float, Image::RGBA, Image::Linear, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R32G32B32A32_UInt:
            return Image(width, height, Image::UnsignedInt, Image::RGBA, Image::Unencoded, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R32G32B32A32_SInt:
            return Image(width, height, Image::Int, Image::RGBA, Image::Unencoded, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R32G32B32_Float:
            return Image(width, height, Image::Float, Image::RGB, Image::Linear, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R32G32B32_UInt:
            return Image(width, height, Image::UnsignedInt, Image::RGB, Image::Unencoded, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R32G32B32_SInt:
            return Image(width, height, Image::Int, Image::RGB, Image::Unencoded, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R16G16B16A16_UNorm:
            return Image(width, height, Image::UnsignedShort, Image::RGBA, Image::Unencoded, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R16G16B16A16_UInt:
            return Image(width, height, Image::UnsignedShort, Image::RGBA, Image::Linear, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R16G16B16A16_SNorm:
            return Image(width, height, Image::Short, Image::RGBA, Image::Unencoded, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R16G16B16A16_SInt:
            return Image(width, height, Image::Short, Image::RGBA, Image::Linear, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R32G32_Float:
            return Image(width, height, Image::Float, Image::RG, Image::Linear, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R32G32_UInt:
            return Image(width, height, Image::UnsignedInt, Image::RG, Image::Unencoded, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R32G32_SInt:
            return Image(width, height, Image::Int, Image::RG, Image::Unencoded, mip0->m_mem);
        
        case DDSFile::DXGIFormat::R8G8B8A8_UNorm:
            return Image(width, height, Image::UnsignedByte, Image::RGBA, Image::Linear, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R8G8B8A8_UNorm_SRGB:
            return Image(width, height, Image::UnsignedByte, Image::RGBA, Image::sRGB, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R8G8B8A8_UInt:
            return Image(width, height, Image::UnsignedByte, Image::RGBA, Image::Unencoded, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R8G8B8A8_SNorm:
            return Image(width, height, Image::Byte, Image::RGBA, Image::Linear, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R8G8B8A8_SInt:
            return Image(width, height, Image::Byte, Image::RGBA, Image::Unencoded, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R16G16_UNorm:
            return Image(width, height, Image::UnsignedShort, Image::RG, Image::Linear, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R16G16_UInt:
            return Image(width, height, Image::UnsignedShort, Image::RG, Image::Unencoded, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R16G16_SNorm:
            return Image(width, height, Image::Short, Image::RG, Image::Linear, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R16G16_SInt:
            return Image(width, height, Image::Short, Image::RG, Image::Unencoded, mip0->m_mem);
            
        case DDSFile::DXGIFormat::D32_Float:
        case DDSFile::DXGIFormat::R32_Float:
            return Image(width, height, Image::Float, Image::R, Image::Linear, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R32_UInt:
            return Image(width, height, Image::UnsignedInt, Image::R, Image::Unencoded, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R32_SInt:
            return Image(width, height, Image::Int, Image::R, Image::Unencoded, mip0->m_mem);

        case DDSFile::DXGIFormat::R8G8_UNorm:
            return Image(width, height, Image::UnsignedByte, Image::RG, Image::Linear, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R8G8_UInt:
            return Image(width, height, Image::UnsignedByte, Image::RG, Image::Unencoded, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R8G8_SNorm:
            return Image(width, height, Image::Byte, Image::RG, Image::Linear, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R8G8_SInt:
            return Image(width, height, Image::Byte, Image::RG, Image::Unencoded, mip0->m_mem);
            
        case DDSFile::DXGIFormat::D16_UNorm:
        case DDSFile::DXGIFormat::R16_UNorm:
            return Image(width, height, Image::UnsignedShort, Image::R, Image::Linear, mip0->m_mem);
        
        case DDSFile::DXGIFormat::R16_UInt:
            return Image(width, height, Image::UnsignedShort, Image::R, Image::Unencoded, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R16_SNorm:
            return Image(width, height, Image::Short, Image::R, Image::Linear, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R16_SInt:
            return Image(width, height, Image::Short, Image::R, Image::Unencoded, mip0->m_mem);
            
        case DDSFile::DXGIFormat::A8_UNorm:
        case DDSFile::DXGIFormat::R8_UNorm:
            return Image(width, height, Image::UnsignedByte, Image::R, Image::Linear, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R8_UInt:
            return Image(width, height, Image::UnsignedByte, Image::R, Image::Unencoded, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R8_SNorm:
            return Image(width, height, Image::Byte, Image::R, Image::Linear, mip0->m_mem);
            
        case DDSFile::DXGIFormat::R8_SInt:
            return Image(width, height, Image::Byte, Image::R, Image::Unencoded, mip0->m_mem);
        

        case DDSFile::DXGIFormat::B8G8R8X8_UNorm:
            return Image(width, height, Image::UnsignedByte, Image::BGR, Image::Unencoded, mip0->m_mem);
        case DDSFile::DXGIFormat::B8G8R8X8_UNorm_SRGB:
            return Image(width, height, Image::UnsignedByte, Image::BGR, Image::sRGB, mip0->m_mem);
            
            
        case DDSFile::DXGIFormat::BC1_UNorm:
        case DDSFile::DXGIFormat::BC1_UNorm_SRGB:
        {
            std::vector<uint8_t> rgba(width * height * 4);
            uint32_t pitch = width * 4;
            for (uint32_t y = 0; y < height; y += 4)
            for (uint32_t x = 0; x < width; x += 4) 
            {
                const uint8_t* block = reinterpret_cast<const uint8_t*>(mip0->m_mem) + (y / 4) * (width / 4) * 8 + (x / 4) * 8;
                uint8_t* dst = rgba.data() + (size_t)(y * pitch + x * 4);
                bcdec_bc1(block, dst, pitch);
            }
            return Image(width, height, Image::UnsignedByte, Image::RGBA, Image::sRGB, rgba.data());
        }
            
        case DDSFile::DXGIFormat::BC2_UNorm:
        case DDSFile::DXGIFormat::BC2_UNorm_SRGB:
        {
            std::vector<uint8_t> rgba(width * height * 4);
            uint32_t pitch = width * 4;
            for (uint32_t y = 0; y < height; y += 4)
            for (uint32_t x = 0; x < width; x += 4) 
            {
                const uint8_t* block = reinterpret_cast<const uint8_t*>(mip0->m_mem) + (y / 4) * (width / 4) * 16 + (x / 4) * 16;
                uint8_t* dst = rgba.data() + (size_t)(y * pitch + x * 4);
                bcdec_bc2(block, dst, pitch);
            }
            return Image(width, height, Image::UnsignedByte, Image::RGBA, Image::sRGB, rgba.data());
        }
            
        case DDSFile::DXGIFormat::BC3_UNorm:
        case DDSFile::DXGIFormat::BC3_UNorm_SRGB:
        {
            std::vector<uint8_t> rgba(width * height * 4);
            uint32_t pitch = width * 4;
            for (uint32_t y = 0; y < height; y += 4)
            for (uint32_t x = 0; x < width; x += 4) 
            {
                const uint8_t* block = reinterpret_cast<const uint8_t*>(mip0->m_mem) + (y / 4) * (width / 4) * 16 + (x / 4) * 16;
                uint8_t* dst = rgba.data() + (size_t)(y * pitch + x * 4);
                bcdec_bc3(block, dst, pitch);
            }
            return Image(width, height, Image::UnsignedByte, Image::RGBA, Image::sRGB, rgba.data());
        }
            
        case DDSFile::DXGIFormat::BC4_UNorm:
        case DDSFile::DXGIFormat::BC4_SNorm:
        {
            std::vector<uint8_t> r(width * height * 1);
            uint32_t pitch = width * 1;
            for (uint32_t y = 0; y < height; y += 4)
            for (uint32_t x = 0; x < width; x += 4) 
            {
                const uint8_t* block = reinterpret_cast<const uint8_t*>(mip0->m_mem) + (y / 4) * (width / 4) * 8 + (x / 4) * 8;
                uint8_t* dst = r.data() + (size_t)(y * pitch + x * 4);
                bcdec_bc4(block, dst, pitch);
            }
            return Image(width, height, Image::UnsignedByte, Image::R, Image::Linear, r.data());
        }
            
        case DDSFile::DXGIFormat::BC5_UNorm:
        case DDSFile::DXGIFormat::BC5_SNorm:
        {
            std::vector<uint8_t> rg(width * height * 2);
            uint32_t pitch = width * 2;
            for (uint32_t y = 0; y < height; y += 4)
            for (uint32_t x = 0; x < width; x += 4) 
            {
                const uint8_t* block = reinterpret_cast<const uint8_t*>(mip0->m_mem) + (y / 4) * (width / 4) * 16 + (x / 4) * 16;
                uint8_t* dst = rg.data() + (size_t)(y * pitch + x * 4);
                bcdec_bc5(block, dst, pitch);
            }
            
            return Image(width, height, Image::UnsignedByte, Image::RG, Image::Linear, rg.data());
        }
        
        case DDSFile::DXGIFormat::BC6H_UF16:
        {
            std::vector<float> rgb(width * height * 3);
            int pitch = width * 4;
            for (uint32_t y = 0; y < height; y += 4)
                for (uint32_t x = 0; x < width; x += 4) 
                {
                    const uint8_t* block = reinterpret_cast<const uint8_t*>(mip0->m_mem) + (y / 4) * (width / 4) * 16 + (x / 4) * 16;
                    float* dst = rgb.data() + (size_t)(y * pitch + x * 3);
                    bcdec_bc6h_float(block, dst, pitch, false);
                }
    
            return Image(width, height, Image::Float, Image::RGB, Image::Linear, rgb.data());
        }
            
        case DDSFile::DXGIFormat::BC6H_SF16:
        {
            std::vector<float> rgb(width * height * 3);
            int pitch = width * 4;
            for (uint32_t y = 0; y < height; y += 4)
            for (uint32_t x = 0; x < width; x += 4) 
            {
                const uint8_t* block = reinterpret_cast<const uint8_t*>(mip0->m_mem) + (y / 4) * (width / 4) * 16 + (x / 4) * 16;
                float* dst = rgb.data() + (size_t)(y * pitch + x * 3);
                bcdec_bc6h_float(block, dst, pitch, true);
            }
        
            return Image(width, height, Image::Float, Image::RGB, Image::Linear, rgb.data());
        }
            
            break;
            
        case DDSFile::DXGIFormat::BC7_UNorm:
        case DDSFile::DXGIFormat::BC7_UNorm_SRGB:
        {
            std::vector<uint8_t> rgba(width * height * 4);
            int pitch = width * 4;
            for (uint32_t y = 0; y < height; y += 4)
            for (uint32_t x = 0; x < width; x += 4) 
            {
                const uint8_t* block = reinterpret_cast<const uint8_t*>(mip0->m_mem) + (y / 4) * (width / 4) * 16 + (x / 4) * 16;
                uint8_t* dst = rgba.data() + (size_t)(y * pitch + x * 4);
                bcdec_bc7(block, dst, pitch);
            }
            
            return Image(width, height, Image::UnsignedByte, Image::RGBA, Image::sRGB, rgba.data());
        }
            
        case DDSFile::DXGIFormat::B8G8R8A8_UNorm:
            // return Image(width, height, Image::UnsignedByte, Image::BGR, Image::Unencoded, mip0->m_mem);
        case DDSFile::DXGIFormat::B8G8R8A8_UNorm_SRGB:
            // return Image(width, height, Image::UnsignedByte, Image::BGR, Image::sRGB, mip0->m_mem);
        case DDSFile::DXGIFormat::Unknown:
        case DDSFile::DXGIFormat::R32G32B32A32_Typeless:
        case DDSFile::DXGIFormat::R32G32B32_Typeless:
        case DDSFile::DXGIFormat::R16G16B16A16_Typeless:
        case DDSFile::DXGIFormat::R32G32_Typeless:
        case DDSFile::DXGIFormat::R32G8X24_Typeless:
        case DDSFile::DXGIFormat::R10G10B10A2_Typeless:
        case DDSFile::DXGIFormat::R8G8B8A8_Typeless:
        case DDSFile::DXGIFormat::R16G16_Typeless:
        case DDSFile::DXGIFormat::R32_Typeless:
        case DDSFile::DXGIFormat::R24G8_Typeless:
        case DDSFile::DXGIFormat::R8G8_Typeless:
        case DDSFile::DXGIFormat::R16_Typeless:
        case DDSFile::DXGIFormat::R8_Typeless:
        case DDSFile::DXGIFormat::BC1_Typeless:
        case DDSFile::DXGIFormat::BC2_Typeless:
        case DDSFile::DXGIFormat::BC3_Typeless:
        case DDSFile::DXGIFormat::BC4_Typeless:
        case DDSFile::DXGIFormat::BC5_Typeless:
        case DDSFile::DXGIFormat::B8G8R8A8_Typeless:
        case DDSFile::DXGIFormat::B8G8R8X8_Typeless:
        case DDSFile::DXGIFormat::BC6H_Typeless:
        case DDSFile::DXGIFormat::BC7_Typeless:
        case DDSFile::DXGIFormat::R16G16B16A16_Float:
        case DDSFile::DXGIFormat::D32_Float_S8X24_UInt:
        case DDSFile::DXGIFormat::R32_Float_X8X24_Typeless:
        case DDSFile::DXGIFormat::X32_Typeless_G8X24_UInt:
        case DDSFile::DXGIFormat::R10G10B10A2_UNorm:
        case DDSFile::DXGIFormat::R10G10B10A2_UInt:
        case DDSFile::DXGIFormat::R11G11B10_Float:
        case DDSFile::DXGIFormat::R16G16_Float:
        case DDSFile::DXGIFormat::D24_UNorm_S8_UInt:
        case DDSFile::DXGIFormat::R24_UNorm_X8_Typeless:
        case DDSFile::DXGIFormat::X24_Typeless_G8_UInt:
        case DDSFile::DXGIFormat::R16_Float:
        case DDSFile::DXGIFormat::R1_UNorm:
        case DDSFile::DXGIFormat::R9G9B9E5_SHAREDEXP:
        case DDSFile::DXGIFormat::R8G8_B8G8_UNorm:
        case DDSFile::DXGIFormat::G8R8_G8B8_UNorm:
        case DDSFile::DXGIFormat::B5G6R5_UNorm:
        case DDSFile::DXGIFormat::B5G5R5A1_UNorm: 
        case DDSFile::DXGIFormat::R10G10B10_XR_BIAS_A2_UNorm:
        case DDSFile::DXGIFormat::AYUV:
        case DDSFile::DXGIFormat::Y410:
        case DDSFile::DXGIFormat::Y416:
        case DDSFile::DXGIFormat::NV12:
        case DDSFile::DXGIFormat::P010:
        case DDSFile::DXGIFormat::P016:
        case DDSFile::DXGIFormat::YUV420_OPAQUE:
        case DDSFile::DXGIFormat::YUY2:
        case DDSFile::DXGIFormat::Y210:
        case DDSFile::DXGIFormat::Y216:
        case DDSFile::DXGIFormat::NV11:
        case DDSFile::DXGIFormat::AI44:
        case DDSFile::DXGIFormat::IA44:
        case DDSFile::DXGIFormat::P8:
        case DDSFile::DXGIFormat::A8P8:
        case DDSFile::DXGIFormat::B4G4R4A4_UNorm:
        case DDSFile::DXGIFormat::P208:
        case DDSFile::DXGIFormat::V208:
        case DDSFile::DXGIFormat::V408:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported DDS texture type")
        }
    }

    switch (ret) 
    {
    case tinyddsloader::ErrorFileOpen:          EngineLoggerError("Failed to load DDS image. returned result is ErrorFileOpen"); break;
    case tinyddsloader::ErrorRead:              EngineLoggerError("Failed to load DDS image. returned result is ErrorRead"); break;
    case tinyddsloader::ErrorMagicWord:         EngineLoggerError("Failed to load DDS image. returned result is ErrorMagicWord"); break;
    case tinyddsloader::ErrorSize:              EngineLoggerError("Failed to load DDS image. returned result is ErrorSize"); break;
    case tinyddsloader::ErrorVerify:            EngineLoggerError("Failed to load DDS image. returned result is ErrorVerify"); break;
    case tinyddsloader::ErrorNotSupported:      EngineLoggerError("Failed to load DDS image. returned result is ErrorNotSupported"); break;
    case tinyddsloader::ErrorInvalidData:       EngineLoggerError("Failed to load DDS image. returned result is ErrorInvalidData"); break;
        
    case tinyddsloader::Success:
    if (dds.GetMipCount() == 0)                 EngineLoggerError("Failed to load DDS image. no error but image has no mip");
        
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("DDS image read failed for unknown reasons")
    }
    
    return Image(0,0, Image::UnsignedByte, Image::R);
}

static bool LoadImageDDSFromMemory(const uint8_t* data, size_t size, Image& Image)
{
    using namespace tinyddsloader;
    
    AssertOrErrorCall(data != nullptr && size > 0, return false, "Invalid image data")
    
    DDSFile dds;
    auto ret = dds.Load(data, size);
    
    if (ret == Result::Success && dds.GetMipCount() > 0)
    {
        const DDSFile::ImageData* mip0 = dds.GetImageData(0, 0);
        uint32_t width = mip0->m_width;
        uint32_t height = mip0->m_height;
        DDSFile::DXGIFormat format = dds.GetFormat();
        
        Image::Type componentType;
        Image::Layout componentLayout;
        Image::Encoding encoding;
        
        // Decode image types
        // Reserve decoding buffer
        switch (format)
        {
        case DDSFile::DXGIFormat::R32G32B32A32_Float:
            componentType = Image::Float;
            componentLayout = Image::RGBA;
            encoding = Image::Linear;
            break;
            
        case DDSFile::DXGIFormat::R32G32B32A32_UInt:
            componentType = Image::UnsignedInt;
            componentLayout = Image::RGBA;
            encoding = Image::Unencoded;
            break;
            
        case DDSFile::DXGIFormat::R32G32B32A32_SInt:
            componentType = Image::Int;
            componentLayout = Image::RGBA;
            encoding = Image::Unencoded;
            break;
            
        case DDSFile::DXGIFormat::R32G32B32_Float:
            componentType = Image::Float;
            componentLayout = Image::RGB;
            encoding = Image::Linear;
            break;
            
        case DDSFile::DXGIFormat::R32G32B32_UInt:
            componentType = Image::UnsignedInt;
            componentLayout = Image::RGB;
            encoding = Image::Unencoded;
            break;
            
        case DDSFile::DXGIFormat::R32G32B32_SInt:
            componentType = Image::Int;
            componentLayout = Image::RGB;
            encoding = Image::Unencoded;
            break;
            
        case DDSFile::DXGIFormat::R16G16B16A16_UNorm:
            componentType = Image::UnsignedShort;
            componentLayout = Image::RGBA;
            encoding = Image::Linear;
            break;
            
        case DDSFile::DXGIFormat::R16G16B16A16_UInt:
            componentType = Image::UnsignedShort;
            componentLayout = Image::RGBA;
            encoding = Image::Unencoded;
            break;
            
        case DDSFile::DXGIFormat::R16G16B16A16_SNorm:
            componentType = Image::Short;
            componentLayout = Image::RGBA;
            encoding = Image::Linear;
            break;
            
        case DDSFile::DXGIFormat::R16G16B16A16_SInt:
            componentType = Image::Short;
            componentLayout = Image::RGBA;
            encoding = Image::Unencoded;
            break;
            
        case DDSFile::DXGIFormat::R32G32_Float:
            componentType = Image::Float;
            componentLayout = Image::RG;
            encoding = Image::Linear;
            break;
            
        case DDSFile::DXGIFormat::R32G32_UInt:
            componentType = Image::UnsignedInt;
            componentLayout = Image::RG;
            encoding = Image::Unencoded;
            break;
            
        case DDSFile::DXGIFormat::R32G32_SInt:
            componentType = Image::Int;
            componentLayout = Image::RG;
            encoding = Image::Unencoded;
            break;
        
        case DDSFile::DXGIFormat::R8G8B8A8_UNorm:
            componentType = Image::UnsignedByte;
            componentLayout = Image::RGBA;
            encoding = Image::Linear;
            break;
            
        case DDSFile::DXGIFormat::R8G8B8A8_UNorm_SRGB:
            componentType = Image::UnsignedByte;
            componentLayout = Image::RGBA;
            encoding = Image::sRGB;
            break;
            
        case DDSFile::DXGIFormat::R8G8B8A8_UInt:
            componentType = Image::UnsignedByte;
            componentLayout = Image::RGBA;
            encoding = Image::Unencoded;
            break;
            
        case DDSFile::DXGIFormat::R8G8B8A8_SNorm:
            componentType = Image::Byte;
            componentLayout = Image::RGBA;
            encoding = Image::Linear;
            break;
            
        case DDSFile::DXGIFormat::R8G8B8A8_SInt:
            componentType = Image::Byte;
            componentLayout = Image::RGBA;
            encoding = Image::Unencoded;
            break;
            
        case DDSFile::DXGIFormat::R16G16_UNorm:
            componentType = Image::UnsignedShort;
            componentLayout = Image::RG;
            encoding = Image::Linear;
            break;
            
        case DDSFile::DXGIFormat::R16G16_UInt:
            componentType = Image::UnsignedShort;
            componentLayout = Image::RG;
            encoding = Image::Unencoded;
            break;
            
        case DDSFile::DXGIFormat::R16G16_SNorm:
            componentType = Image::Short;
            componentLayout = Image::RG;
            encoding = Image::Linear;
            break;
            
        case DDSFile::DXGIFormat::R16G16_SInt:
            componentType = Image::Short;
            componentLayout = Image::RG;
            encoding = Image::Unencoded;
            break;
            
        case DDSFile::DXGIFormat::D32_Float:
        case DDSFile::DXGIFormat::R32_Float:
            componentType = Image::Float;
            componentLayout = Image::R;
            encoding = Image::Linear;
            break;
            
        case DDSFile::DXGIFormat::R32_UInt:
            componentType = Image::UnsignedInt;
            componentLayout = Image::R;
            encoding = Image::Unencoded;
            break;
            
        case DDSFile::DXGIFormat::R32_SInt:
            componentType = Image::Int;
            componentLayout = Image::R;
            encoding = Image::Unencoded;
            break;

        case DDSFile::DXGIFormat::R8G8_UNorm:
            componentType = Image::UnsignedByte;
            componentLayout = Image::RG;
            encoding = Image::Linear;
            break;
            
        case DDSFile::DXGIFormat::R8G8_UInt:
            componentType = Image::UnsignedByte;
            componentLayout = Image::RG;
            encoding = Image::Unencoded;
            break;
            
        case DDSFile::DXGIFormat::R8G8_SNorm:
            componentType = Image::Byte;
            componentLayout = Image::RG;
            encoding = Image::Linear;
            break;
            
        case DDSFile::DXGIFormat::R8G8_SInt:
            componentType = Image::Byte;
            componentLayout = Image::RG;
            encoding = Image::Unencoded;
            break;
            
        case DDSFile::DXGIFormat::D16_UNorm:
        case DDSFile::DXGIFormat::R16_UNorm:
            componentType = Image::UnsignedShort;
            componentLayout = Image::R;
            encoding = Image::Linear;
            break;
        
        case DDSFile::DXGIFormat::R16_UInt:
            componentType = Image::UnsignedShort;
            componentLayout = Image::R;
            encoding = Image::Unencoded;
            break;
            
        case DDSFile::DXGIFormat::R16_SNorm:
            componentType = Image::Short;
            componentLayout = Image::R;
            encoding = Image::Linear;
            break;
            
        case DDSFile::DXGIFormat::R16_SInt:
            componentType = Image::Short;
            componentLayout = Image::R;
            encoding = Image::Unencoded;
            break;
            
        case DDSFile::DXGIFormat::A8_UNorm:
        case DDSFile::DXGIFormat::R8_UNorm:
            componentType = Image::UnsignedByte;
            componentLayout = Image::R;
            encoding = Image::Linear;
            break;
            
        case DDSFile::DXGIFormat::R8_UInt:
            componentType = Image::UnsignedByte;
            componentLayout = Image::R;
            encoding = Image::Unencoded;
            break;
            
        case DDSFile::DXGIFormat::R8_SNorm:
            componentType = Image::Byte;
            componentLayout = Image::R;
            encoding = Image::Linear;
            break;
            
        case DDSFile::DXGIFormat::R8_SInt:
            componentType = Image::Byte;
            componentLayout = Image::R;
            encoding = Image::Unencoded;
            break;            
            
        case DDSFile::DXGIFormat::BC1_UNorm:
        case DDSFile::DXGIFormat::BC2_UNorm:
        case DDSFile::DXGIFormat::BC3_UNorm:
        case DDSFile::DXGIFormat::BC7_UNorm:
            componentType = Image::UnsignedByte;
            componentLayout = Image::RGBA;
            encoding = Image::Linear;
            break;
            
        case DDSFile::DXGIFormat::BC1_UNorm_SRGB:
        case DDSFile::DXGIFormat::BC2_UNorm_SRGB:
        case DDSFile::DXGIFormat::BC3_UNorm_SRGB:
        case DDSFile::DXGIFormat::BC7_UNorm_SRGB:
            componentType = Image::UnsignedByte;
            componentLayout = Image::RGBA;
            encoding = Image::sRGB;
            break;

            
        case DDSFile::DXGIFormat::BC4_UNorm:
            componentType = Image::UnsignedByte;
            componentLayout = Image::R;
            encoding = Image::Linear;
            break;
            
        case DDSFile::DXGIFormat::BC4_SNorm:
            componentType = Image::UnsignedByte;
            componentLayout = Image::R;
            encoding = Image::sRGB;
            break;
            
        case DDSFile::DXGIFormat::BC5_UNorm:
            componentType = Image::UnsignedByte;
            componentLayout = Image::RG;
            encoding = Image::Linear;
            break;
            
        case DDSFile::DXGIFormat::BC5_SNorm:
            componentType = Image::Byte;
            componentLayout = Image::RG;
            encoding = Image::Linear;
            break;
        
        case DDSFile::DXGIFormat::BC6H_UF16:
            
        case DDSFile::DXGIFormat::BC6H_SF16:
            componentType = Image::Float;
            componentLayout = Image::RGB;
            encoding = Image::Linear;
            break;
            
        case DDSFile::DXGIFormat::B8G8R8X8_UNorm:
            EngineRuntimeCrash("Image buffers does not yet support padding")
            componentType = Image::UnsignedByte;
            componentLayout = Image::BGR;
            encoding = Image::Linear;
            break;
            
        case DDSFile::DXGIFormat::B8G8R8X8_UNorm_SRGB:
            EngineRuntimeCrash("Image buffers does not yet support padding")
            componentType = Image::UnsignedByte;
            componentLayout = Image::BGR;
            encoding = Image::sRGB;
            break;
        
        case DDSFile::DXGIFormat::B8G8R8A8_UNorm:
            EngineRuntimeCrash("Image buffers does not support reversed layout with alpha at the end (BGRA)")
            componentType = Image::UnsignedByte;
            componentLayout = Image::BGR;
            encoding = Image::Linear;
            break;
            // return Image(width, height, Image::UnsignedByte, Image::BGR, Image::Unencoded, mip0->m_mem);
            
        case DDSFile::DXGIFormat::B8G8R8A8_UNorm_SRGB:
            EngineRuntimeCrash("Image buffers does not support reversed layout with alpha at the end (BGRA)")
            componentType = Image::UnsignedByte;
            componentLayout = Image::BGR;
            encoding = Image::sRGB;
            break;
            
        case DDSFile::DXGIFormat::Unknown:
        case DDSFile::DXGIFormat::R32G32B32A32_Typeless:
        case DDSFile::DXGIFormat::R32G32B32_Typeless:
        case DDSFile::DXGIFormat::R16G16B16A16_Typeless:
        case DDSFile::DXGIFormat::R32G32_Typeless:
        case DDSFile::DXGIFormat::R32G8X24_Typeless:
        case DDSFile::DXGIFormat::R10G10B10A2_Typeless:
        case DDSFile::DXGIFormat::R8G8B8A8_Typeless:
        case DDSFile::DXGIFormat::R16G16_Typeless:
        case DDSFile::DXGIFormat::R32_Typeless:
        case DDSFile::DXGIFormat::R24G8_Typeless:
        case DDSFile::DXGIFormat::R8G8_Typeless:
        case DDSFile::DXGIFormat::R16_Typeless:
        case DDSFile::DXGIFormat::R8_Typeless:
        case DDSFile::DXGIFormat::BC1_Typeless:
        case DDSFile::DXGIFormat::BC2_Typeless:
        case DDSFile::DXGIFormat::BC3_Typeless:
        case DDSFile::DXGIFormat::BC4_Typeless:
        case DDSFile::DXGIFormat::BC5_Typeless:
        case DDSFile::DXGIFormat::B8G8R8A8_Typeless:
        case DDSFile::DXGIFormat::B8G8R8X8_Typeless:
        case DDSFile::DXGIFormat::BC6H_Typeless:
        case DDSFile::DXGIFormat::BC7_Typeless:
        case DDSFile::DXGIFormat::R16G16B16A16_Float:
        case DDSFile::DXGIFormat::D32_Float_S8X24_UInt:
        case DDSFile::DXGIFormat::R32_Float_X8X24_Typeless:
        case DDSFile::DXGIFormat::X32_Typeless_G8X24_UInt:
        case DDSFile::DXGIFormat::R10G10B10A2_UNorm:
        case DDSFile::DXGIFormat::R10G10B10A2_UInt:
        case DDSFile::DXGIFormat::R11G11B10_Float:
        case DDSFile::DXGIFormat::R16G16_Float:
        case DDSFile::DXGIFormat::D24_UNorm_S8_UInt:
        case DDSFile::DXGIFormat::R24_UNorm_X8_Typeless:
        case DDSFile::DXGIFormat::X24_Typeless_G8_UInt:
        case DDSFile::DXGIFormat::R16_Float:
        case DDSFile::DXGIFormat::R1_UNorm:
        case DDSFile::DXGIFormat::R9G9B9E5_SHAREDEXP:
        case DDSFile::DXGIFormat::R8G8_B8G8_UNorm:
        case DDSFile::DXGIFormat::G8R8_G8B8_UNorm:
        case DDSFile::DXGIFormat::B5G6R5_UNorm:
        case DDSFile::DXGIFormat::B5G5R5A1_UNorm: 
        case DDSFile::DXGIFormat::R10G10B10_XR_BIAS_A2_UNorm:
        case DDSFile::DXGIFormat::AYUV:
        case DDSFile::DXGIFormat::Y410:
        case DDSFile::DXGIFormat::Y416:
        case DDSFile::DXGIFormat::NV12:
        case DDSFile::DXGIFormat::P010:
        case DDSFile::DXGIFormat::P016:
        case DDSFile::DXGIFormat::YUV420_OPAQUE:
        case DDSFile::DXGIFormat::YUY2:
        case DDSFile::DXGIFormat::Y210:
        case DDSFile::DXGIFormat::Y216:
        case DDSFile::DXGIFormat::NV11:
        case DDSFile::DXGIFormat::AI44:
        case DDSFile::DXGIFormat::IA44:
        case DDSFile::DXGIFormat::P8:
        case DDSFile::DXGIFormat::A8P8:
        case DDSFile::DXGIFormat::B4G4R4A4_UNorm:
        case DDSFile::DXGIFormat::P208:
        case DDSFile::DXGIFormat::V208:
        case DDSFile::DXGIFormat::V408:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported DDS texture type")
        }
        
        AssertOrErrorCallF(width == Image.Width() && height == Image.Height(), return false, "Given Image (%ux%u) and decoded Image (%dx%d) size missmatch", Image.Width(), Image.Height(), width, height)
        AssertOrErrorCall(Image.ComponentLayout() == componentLayout, return false, "Given Image and decoded Image Channel missmatch")
        AssertOrErrorCall(Image.ComponentType() == componentType, return false, "Given Image and decoded image type mismatch")
        AssertOrErrorCall(Image.ComponentEncoding() == encoding, return false, "Given Image and decoded image type mismatch")

        // Decode the image if encoded else copy the result to the image
        switch (format)
        {            
        case DDSFile::DXGIFormat::BC1_UNorm:
        case DDSFile::DXGIFormat::BC1_UNorm_SRGB:
        {
            uint32_t pitch = width * 4;
            for (uint32_t y = 0; y < height; y += 4)
            for (uint32_t x = 0; x < width; x += 4) 
            {
                const uint8_t* block = reinterpret_cast<const uint8_t*>(mip0->m_mem) + (y / 4) * (width / 4) * 8 + (x / 4) * 8;
                uint8_t* dst = (uint8_t*)(Image.Data()) + (size_t)(y * pitch + x * 4);
                bcdec_bc1(block, dst, pitch);
            }
        }
        break;
            
        case DDSFile::DXGIFormat::BC2_UNorm:
        case DDSFile::DXGIFormat::BC2_UNorm_SRGB:
        {
            uint32_t pitch = width * 4;
            for (uint32_t y = 0; y < height; y += 4)
            for (uint32_t x = 0; x < width; x += 4) 
            {
                const uint8_t* block = reinterpret_cast<const uint8_t*>(mip0->m_mem) + (y / 4) * (width / 4) * 16 + (x / 4) * 16;
                uint8_t* dst = (uint8_t*)(Image.Data()) + (size_t)(y * pitch + x * 4);
                bcdec_bc2(block, dst, pitch);
            }
        }
        break;
            
        case DDSFile::DXGIFormat::BC3_UNorm:
        case DDSFile::DXGIFormat::BC3_UNorm_SRGB:
        {
            uint32_t pitch = width * 4;
            for (uint32_t y = 0; y < height; y += 4)
            for (uint32_t x = 0; x < width; x += 4) 
            {
                const uint8_t* block = reinterpret_cast<const uint8_t*>(mip0->m_mem) + (y / 4) * (width / 4) * 16 + (x / 4) * 16;
                uint8_t* dst = (uint8_t*)(Image.Data()) + (size_t)(y * pitch + x * 4);
                bcdec_bc3(block, dst, pitch);
            }
        }
        break;
            
        case DDSFile::DXGIFormat::BC4_UNorm:
        case DDSFile::DXGIFormat::BC4_SNorm:
        {
            uint32_t pitch = width * 1;
            for (uint32_t y = 0; y < height; y += 4)
            for (uint32_t x = 0; x < width; x += 4) 
            {
                const uint8_t* block = reinterpret_cast<const uint8_t*>(mip0->m_mem) + (y / 4) * (width / 4) * 8 + (x / 4) * 8;
                uint8_t* dst = (uint8_t*)(Image.Data()) + (size_t)(y * pitch + x * 4);
                bcdec_bc4(block, dst, pitch);
            }
        }
        break;
            
        case DDSFile::DXGIFormat::BC5_UNorm:
        case DDSFile::DXGIFormat::BC5_SNorm:
        {
            uint32_t pitch = width * 2;
            for (uint32_t y = 0; y < height; y += 4)
            for (uint32_t x = 0; x < width; x += 4) 
            {
                const uint8_t* block = reinterpret_cast<const uint8_t*>(mip0->m_mem) + (y / 4) * (width / 4) * 16 + (x / 4) * 16;
                uint8_t* dst = (uint8_t*)(Image.Data()) + (size_t)(y * pitch + x * 4);
                bcdec_bc5(block, dst, pitch);
            }
        }
        break;
        
        case DDSFile::DXGIFormat::BC6H_UF16:
        {
            int pitch = width * 4;
            for (uint32_t y = 0; y < height; y += 4)
            for (uint32_t x = 0; x < width; x += 4) 
            {
                const uint8_t* block = reinterpret_cast<const uint8_t*>(mip0->m_mem) + (y / 4) * (width / 4) * 16 + (x / 4) * 16;
                float* dst = (float*)(Image.Data()) + (size_t)(y * pitch + x * 3);
                bcdec_bc6h_float(block, dst, pitch, false);
            }
        }
        break;
            
        case DDSFile::DXGIFormat::BC6H_SF16:
        {
            int pitch = width * 4;
            for (uint32_t y = 0; y < height; y += 4)
            for (uint32_t x = 0; x < width; x += 4) 
            {
                const uint8_t* block = reinterpret_cast<const uint8_t*>(mip0->m_mem) + (y / 4) * (width / 4) * 16 + (x / 4) * 16;
                float* dst = (float*)(Image.Data()) + (size_t)(y * pitch + x * 3);
                bcdec_bc6h_float(block, dst, pitch, true);
            }
        }
        break;
            
        case DDSFile::DXGIFormat::BC7_UNorm:
        case DDSFile::DXGIFormat::BC7_UNorm_SRGB:
        {
            int pitch = width * 4;
            for (uint32_t y = 0; y < height; y += 4)
            for (uint32_t x = 0; x < width; x += 4) 
            {
                const uint8_t* block = reinterpret_cast<const uint8_t*>(mip0->m_mem) + (y / 4) * (width / 4) * 16 + (x / 4) * 16;
                uint8_t* dst = (uint8_t*)(Image.Data()) + (size_t)(y * pitch + x * 4);
                bcdec_bc7(block, dst, pitch);
            }
        }
        
        case DDSFile::DXGIFormat::R32G32B32A32_Float:            
        case DDSFile::DXGIFormat::R32G32B32A32_UInt:            
        case DDSFile::DXGIFormat::R32G32B32A32_SInt:            
        case DDSFile::DXGIFormat::R32G32B32_Float:            
        case DDSFile::DXGIFormat::R32G32B32_UInt:            
        case DDSFile::DXGIFormat::R32G32B32_SInt:            
        case DDSFile::DXGIFormat::R16G16B16A16_UNorm:            
        case DDSFile::DXGIFormat::R16G16B16A16_UInt:            
        case DDSFile::DXGIFormat::R16G16B16A16_SNorm:            
        case DDSFile::DXGIFormat::R16G16B16A16_SInt:            
        case DDSFile::DXGIFormat::R32G32_Float:            
        case DDSFile::DXGIFormat::R32G32_UInt:            
        case DDSFile::DXGIFormat::R32G32_SInt:
        case DDSFile::DXGIFormat::R8G8B8A8_UNorm:            
        case DDSFile::DXGIFormat::R8G8B8A8_UNorm_SRGB:            
        case DDSFile::DXGIFormat::R8G8B8A8_UInt:            
        case DDSFile::DXGIFormat::R8G8B8A8_SNorm:            
        case DDSFile::DXGIFormat::R8G8B8A8_SInt:            
        case DDSFile::DXGIFormat::R16G16_UNorm:            
        case DDSFile::DXGIFormat::R16G16_UInt:            
        case DDSFile::DXGIFormat::R16G16_SNorm:            
        case DDSFile::DXGIFormat::R16G16_SInt:            
        case DDSFile::DXGIFormat::D32_Float:
        case DDSFile::DXGIFormat::R32_Float:
        case DDSFile::DXGIFormat::R32_UInt:            
        case DDSFile::DXGIFormat::R32_SInt:
        case DDSFile::DXGIFormat::R8G8_UNorm:            
        case DDSFile::DXGIFormat::R8G8_UInt:            
        case DDSFile::DXGIFormat::R8G8_SNorm:            
        case DDSFile::DXGIFormat::R8G8_SInt:            
        case DDSFile::DXGIFormat::D16_UNorm:
        case DDSFile::DXGIFormat::R16_UNorm:        
        case DDSFile::DXGIFormat::R16_UInt:            
        case DDSFile::DXGIFormat::R16_SNorm:            
        case DDSFile::DXGIFormat::R16_SInt:            
        case DDSFile::DXGIFormat::A8_UNorm:
        case DDSFile::DXGIFormat::R8_UNorm:            
        case DDSFile::DXGIFormat::R8_UInt:            
        case DDSFile::DXGIFormat::R8_SNorm:            
        case DDSFile::DXGIFormat::R8_SInt:
            memcpy(Image.Data(), mip0->m_mem , Image.DataSize());
            break;
            
        case DDSFile::DXGIFormat::B8G8R8X8_UNorm:
        case DDSFile::DXGIFormat::B8G8R8X8_UNorm_SRGB:
        case DDSFile::DXGIFormat::B8G8R8A8_UNorm:
        case DDSFile::DXGIFormat::B8G8R8A8_UNorm_SRGB:
        case DDSFile::DXGIFormat::Unknown:
        case DDSFile::DXGIFormat::R32G32B32A32_Typeless:
        case DDSFile::DXGIFormat::R32G32B32_Typeless:
        case DDSFile::DXGIFormat::R16G16B16A16_Typeless:
        case DDSFile::DXGIFormat::R32G32_Typeless:
        case DDSFile::DXGIFormat::R32G8X24_Typeless:
        case DDSFile::DXGIFormat::R10G10B10A2_Typeless:
        case DDSFile::DXGIFormat::R8G8B8A8_Typeless:
        case DDSFile::DXGIFormat::R16G16_Typeless:
        case DDSFile::DXGIFormat::R32_Typeless:
        case DDSFile::DXGIFormat::R24G8_Typeless:
        case DDSFile::DXGIFormat::R8G8_Typeless:
        case DDSFile::DXGIFormat::R16_Typeless:
        case DDSFile::DXGIFormat::R8_Typeless:
        case DDSFile::DXGIFormat::BC1_Typeless:
        case DDSFile::DXGIFormat::BC2_Typeless:
        case DDSFile::DXGIFormat::BC3_Typeless:
        case DDSFile::DXGIFormat::BC4_Typeless:
        case DDSFile::DXGIFormat::BC5_Typeless:
        case DDSFile::DXGIFormat::B8G8R8A8_Typeless:
        case DDSFile::DXGIFormat::B8G8R8X8_Typeless:
        case DDSFile::DXGIFormat::BC6H_Typeless:
        case DDSFile::DXGIFormat::BC7_Typeless:
        case DDSFile::DXGIFormat::R16G16B16A16_Float:
        case DDSFile::DXGIFormat::D32_Float_S8X24_UInt:
        case DDSFile::DXGIFormat::R32_Float_X8X24_Typeless:
        case DDSFile::DXGIFormat::X32_Typeless_G8X24_UInt:
        case DDSFile::DXGIFormat::R10G10B10A2_UNorm:
        case DDSFile::DXGIFormat::R10G10B10A2_UInt:
        case DDSFile::DXGIFormat::R11G11B10_Float:
        case DDSFile::DXGIFormat::R16G16_Float:
        case DDSFile::DXGIFormat::D24_UNorm_S8_UInt:
        case DDSFile::DXGIFormat::R24_UNorm_X8_Typeless:
        case DDSFile::DXGIFormat::X24_Typeless_G8_UInt:
        case DDSFile::DXGIFormat::R16_Float:
        case DDSFile::DXGIFormat::R1_UNorm:
        case DDSFile::DXGIFormat::R9G9B9E5_SHAREDEXP:
        case DDSFile::DXGIFormat::R8G8_B8G8_UNorm:
        case DDSFile::DXGIFormat::G8R8_G8B8_UNorm:
        case DDSFile::DXGIFormat::B5G6R5_UNorm:
        case DDSFile::DXGIFormat::B5G5R5A1_UNorm: 
        case DDSFile::DXGIFormat::R10G10B10_XR_BIAS_A2_UNorm:
        case DDSFile::DXGIFormat::AYUV:
        case DDSFile::DXGIFormat::Y410:
        case DDSFile::DXGIFormat::Y416:
        case DDSFile::DXGIFormat::NV12:
        case DDSFile::DXGIFormat::P010:
        case DDSFile::DXGIFormat::P016:
        case DDSFile::DXGIFormat::YUV420_OPAQUE:
        case DDSFile::DXGIFormat::YUY2:
        case DDSFile::DXGIFormat::Y210:
        case DDSFile::DXGIFormat::Y216:
        case DDSFile::DXGIFormat::NV11:
        case DDSFile::DXGIFormat::AI44:
        case DDSFile::DXGIFormat::IA44:
        case DDSFile::DXGIFormat::P8:
        case DDSFile::DXGIFormat::A8P8:
        case DDSFile::DXGIFormat::B4G4R4A4_UNorm:
        case DDSFile::DXGIFormat::P208:
        case DDSFile::DXGIFormat::V208:
        case DDSFile::DXGIFormat::V408:
            UNREACHABLE;
            
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported texture format")
        }
        
        return true;
    }

    switch (ret) 
    {
    case tinyddsloader::ErrorFileOpen:          EngineLoggerError("Failed to load DDS image. returned result is ErrorFileOpen"); break;
    case tinyddsloader::ErrorRead:              EngineLoggerError("Failed to load DDS image. returned result is ErrorRead"); break;
    case tinyddsloader::ErrorMagicWord:         EngineLoggerError("Failed to load DDS image. returned result is ErrorMagicWord"); break;
    case tinyddsloader::ErrorSize:              EngineLoggerError("Failed to load DDS image. returned result is ErrorSize"); break;
    case tinyddsloader::ErrorVerify:            EngineLoggerError("Failed to load DDS image. returned result is ErrorVerify"); break;
    case tinyddsloader::ErrorNotSupported:      EngineLoggerError("Failed to load DDS image. returned result is ErrorNotSupported"); break;
    case tinyddsloader::ErrorInvalidData:       EngineLoggerError("Failed to load DDS image. returned result is ErrorInvalidData"); break;
        
    case tinyddsloader::Success:
    if (dds.GetMipCount() == 0)                 EngineLoggerError("Failed to load DDS image. no error but image has no mip");
        
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("DDS image read failed for unknown reasons")
    }
    
    return false;
}

static Image LoadImageDDS(const std::filesystem::path& ImagePath, Image::Type ComponentType)
{
    std::ifstream file(ImagePath, std::ios::binary | std::ios::ate);
    AssertOrErrorCallF(file.is_open(), return Image(0,0, ComponentType, Image::R),
        "Could not open file %s", ImagePath.generic_string().c_str())

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (file.read(reinterpret_cast<char*>(buffer.data()), size))
    {
        return LoadImageDDSFromMemory(buffer.data(), size);
    }

    return Image(0,0,Image::UnsignedByte, Image::R);
}

static Image LoadImageEXRFromMemory(const uint8_t* data, size_t size)
{
    int width, height;
    float* image;
    const char* err = NULL;
    
    int ret = IsEXRFromMemory(data, size);
    AssertOrErrorCallF(ret == TINYEXR_SUCCESS, Image(0,0,Image::UnsignedByte, Image::R), "File not found or given file is not a EXR format. code %d\n", ret)
    
    ret = LoadEXRFromMemory(&image, &width, &height, data, size, &err);
    
    if (ret != TINYEXR_SUCCESS) 
    {
        if (err) 
        {
            EngineLoggerErrorF("Load EXR err: %s(code %d)\n", err, ret);
        } 
        else 
        {
            EngineLoggerErrorF("Load EXR err: code = %d\n", ret);
        }
        EngineRuntimeBREAKPOINT
        
        FreeEXRErrorMessage(err);
        return Image(0,0,Image::UnsignedByte, Image::R);
    }
    
    Image Result(width, height, Image::Float, Image::RGBA, Image::Linear, image);
    
    free(image);
    
    return Result;
}

static bool LoadImageEXRFromMemory(const uint8_t* data, size_t size, Image& Image)
{
    AssertOrErrorCall(data != nullptr && size > 0, return false, "Invalid image data")
    
    int width, height;
    float* image;
    const char* err = NULL;
    
    int ret = IsEXRFromMemory(data, size);
    AssertOrErrorCallF(ret == TINYEXR_SUCCESS, return false, "File not found or given file is not a EXR format. code %d\n", ret)
    
    ret = LoadEXRFromMemory(&image, &width, &height, data, size, &err);
    
    if (ret != TINYEXR_SUCCESS) 
    {
        if (err) 
        {
            EngineLoggerErrorF("Load EXR err: %s(code %d)\n", err, ret);
        } 
        else 
        {
            EngineLoggerErrorF("Load EXR err: code = %d\n", ret);
        }
        EngineRuntimeBREAKPOINT
        
        FreeEXRErrorMessage(err);
        return false;
    }
    
    AssertOrErrorCallF(width == Image.Width() && height == Image.Height(), return false, "Given Image (%ux%u) and decoded Image (%dx%d) size missmatch", Image.Width(), Image.Height(), width, height)
    AssertOrErrorCall(Image.ComponentType() == Image::Float, return false, "Given Image and decoded image type mismatch")
    AssertOrErrorCall(Image.ComponentLayout() == Image::RGBA || Image.ComponentLayout() == Image::ABGR || Image.ComponentLayout() == Image::ARGB, return false, "Given Image and decoded Image Channel type missmatch")
    AssertOrErrorCall(Image.ComponentEncoding() == Image::Linear, return false, "Given Image and decoded image type mismatch")
    
    memcpy(Image.Data(), image, Image.DataSize());
    
    free(image);
    
    return true;
}

static Image LoadImageEXR(const std::filesystem::path& ImagePath, Image::Type ComponentType)
{
    int width, height;
    float* image;
    const char* err = NULL;
    std::string path = ImagePath.generic_string().c_str();
    
    int ret = IsEXR(path.c_str());
    AssertOrErrorCallF(ret == TINYEXR_SUCCESS, Image(0,0,Image::UnsignedByte, Image::R), "File not found or given file is not a EXR format. code %d\n", ret)
    
    ret = LoadEXR(&image, &width, &height, path.c_str(), &err);
    
    if (ret != TINYEXR_SUCCESS) 
    {
        if (err) 
        {
            EngineLoggerErrorF("Load EXR err: %s(code %d)\n", err, ret);
        } 
        else 
        {
            EngineLoggerErrorF("Load EXR err: code = %d\n", ret);
        }
        EngineRuntimeBREAKPOINT
        
        FreeEXRErrorMessage(err);
        return Image(0,0,Image::UnsignedByte, Image::R);
    }
    
    Image Result(width, height, Image::Float, Image::RGBA, Image::Linear, image);
    
    free(image);
    
    return Result;
}

static Image LoadImageSTBFromMemory(const uint8_t* data, size_t size, Image::FileType FileType)
{
    int Width = 0, Height = 0, ChannelCount = 0;
    void* Buffer;
    Image::Type componentType;
    Image::Encoding encoding;
    
    switch (FileType)
    {
    case Image::JPG:
    case Image::PNG:
    case Image::TGA:
    case Image::BMP:
        Buffer = stbi_load_from_memory(data, size, &Width, &Height, &ChannelCount, 0);
        componentType = Image::UnsignedByte;
        encoding = Image::sRGB;
        break;
        
    case Image::HDR:
        Buffer = stbi_loadf_from_memory(data, size, &Width, &Height, &ChannelCount, 0);
        componentType = Image::Float;
        encoding = Image::Linear;
        break;
        
    case Image::DDS:
    case Image::EXR:
    case Image::_Count:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported file type")
        }

    Image::Layout Layout;
    switch (ChannelCount)
    {
    case 1: Layout = Image::R;      break;
    case 2: Layout = Image::RG;     break;
    case 3: Layout = Image::RGB;    break;
    case 4: Layout = Image::RGBA;   break;

    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported channel count")
    }
    Image Image(Width, Height, componentType, Layout, encoding, Buffer);

    stbi_image_free(Buffer);

    return Image;
}

static bool LoadImageSTBFromMemory(const uint8_t* data, size_t size, Image::FileType FileType, Image& Image)
{    
    int Width = 0, Height = 0, ChannelCount = 0;
    void* Buffer;
    Image::Type componentType;
    Image::Encoding encoding;
    
    switch (FileType)
    {
    case Image::JPG:
    case Image::PNG:
    case Image::TGA:
    case Image::BMP:
        Buffer = stbi_load_from_memory(data, size, &Width, &Height, &ChannelCount, 0);
        componentType = Image::UnsignedByte;
        encoding = Image::sRGB;
        break;
        
    case Image::HDR:
        Buffer = stbi_loadf_from_memory(data, size, &Width, &Height, &ChannelCount, 0);
        componentType = Image::Float;
        encoding = Image::Linear;
        break;
        
    case Image::DDS:
    case Image::EXR:
    case Image::_Count:
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported file type")
    }

    Image::Layout Layout;
    switch (ChannelCount)
    {
    case 1: Layout = Image::R;      break;
    case 2: Layout = Image::RG;     break;
    case 3: Layout = Image::RGB;    break;
    case 4: Layout = Image::RGBA;   break;

    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported channel count")
    }
    
    AssertOrErrorCallF(Width == Image.Width() && Height == Image.Height(), goto on_failed, "Given Image (%ux%u) and decoded Image (%dx%d) size missmatch", Image.Width(), Image.Height(), Width, Height)
    AssertOrErrorCallF(ChannelCount == Image.ComponentCount(), goto on_failed, "Given Image (%u) and decoded Image (%d) Channel count missmatch", Image.ComponentCount(), ChannelCount)
    AssertOrErrorCall(Image.ComponentType() == componentType, goto on_failed, "Given Image and decoded image type mismatch")
    AssertOrErrorCall(Image.ComponentEncoding() == encoding, goto on_failed, "Given Image and decoded image type mismatch")
    
    memcpy(Image.Data(), Buffer, Image.DataSize());

    stbi_image_free(Buffer);

    return true;
    
on_failed:
    stbi_image_free(Buffer);

    return false;
}

static Image LoadImageSTB(const std::filesystem::path& ImagePath, Image::Type ComponentType)
{
    int Width = 0, Height = 0, ChannelCount = 0;
    void* Buffer;
    Image::Encoding encoding;
        
    switch (ComponentType)
    {
    case Image::UnsignedByte:
        Buffer = stbi_load(ImagePath.generic_string().c_str(), &Width, &Height, &ChannelCount, 0);
        encoding = Image::sRGB;
        break;
        
    case Image::UnsignedShort:
        Buffer = stbi_load_16(ImagePath.generic_string().c_str(), &Width, &Height, &ChannelCount, 0);
        encoding = Image::sRGB;
        break;
        
    case Image::Float:
        Buffer = stbi_loadf(ImagePath.generic_string().c_str(), &Width, &Height, &ChannelCount, 0);
        encoding = Image::Linear;
        break;

    case Image::Byte:
    case Image::Short:
    case Image::UnsignedInt:
    case Image::Int:
    case Image::Double:
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported channel type")
    }

    Image::Layout Layout;
    switch (ChannelCount)
    {
    case 1: Layout = Image::R;      break;
    case 2: Layout = Image::RG;     break;
    case 3: Layout = Image::RGB;    break;
    case 4: Layout = Image::RGBA;   break;

    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported channel count")
    }
    Image Image(Width, Height, ComponentType, Layout, encoding, Buffer);

    stbi_image_free(Buffer);

    return Image;
}

Image ImageLoad(const std::filesystem::path& ImagePath, Image::Type ComponentType)
{
    AssertOrErrorCallF(exists(ImagePath), return Image(0,0, ComponentType, Image::R),
        "Could not open image %s, No such file or directory", ImagePath.generic_string().c_str())
    
    if (ImagePath.extension().compare(".dds") == 0)
    {
        return LoadImageDDS(ImagePath, ComponentType);
    }
    else if (ImagePath.extension().compare(".exr") == 0)
    {
        return LoadImageEXR(ImagePath, ComponentType);
    }
    else
    {
        return LoadImageSTB(ImagePath, ComponentType);
    }
}

Image ImageLoadFromMemory(const uint8_t* Buffer, size_t Size, Image::FileType Type)
{
    switch (Type)
    {
    case Image::JPG:
    case Image::PNG:
    case Image::TGA:
    case Image::BMP:
    case Image::HDR:
        return LoadImageSTBFromMemory(Buffer, Size, Type);
        
    case Image::DDS:
        return LoadImageDDSFromMemory(Buffer, Size);
        
    case Image::EXR:
        return LoadImageEXRFromMemory(Buffer, Size);
        
    case Image::_Count:
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported image format")
    }
}

bool ImageLoadFromMemory(const uint8_t* Buffer, size_t Size, Image::FileType Type, Image& Image)
{
    switch (Type)
    {
    case Image::JPG:
    case Image::PNG:
    case Image::TGA:
    case Image::BMP:
    case Image::HDR:
        return LoadImageSTBFromMemory(Buffer, Size, Type, Image);
        
    case Image::DDS:
        return LoadImageDDSFromMemory(Buffer, Size, Image);
        
    case Image::EXR:
        return LoadImageEXRFromMemory(Buffer, Size, Image);
        
    case Image::_Count:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported image format")
        }
}

bool ImageStore(const std::filesystem::path& OutputPath, const Image& Image, Image::FileType Type)
{
    if (!exists(OutputPath.parent_path()))
    {   
        std::filesystem::create_directory(OutputPath.parent_path());
    }

    switch (Type)
    {
    case Image::JPG: 
        if (Image.ComponentType() == Image::UnsignedByte || Image.ComponentType() == Image::Byte)
        {
            return stbi_write_jpg(OutputPath.generic_string().c_str(), Image.Width(), Image.Height(), Image.ComponentCount(), Image.Data(), 90 /*TODO expose*/) > 0;
        }
        ENUM_OUT_OF_RANGE("JPGs only supports int and uint 8 textures")

    case Image::PNG: 
        if (Image.ComponentType() == Image::UnsignedByte || Image.ComponentType() == Image::Byte)
        {
            return stbi_write_png(OutputPath.generic_string().c_str(), Image.Width(), Image.Height(), Image.ComponentCount(), Image.Data(), Image.Width() * Image.PixelSize()) > 0;
        }
        ENUM_OUT_OF_RANGE("PNGs only supports int and uint 8 textures")

    case Image::TGA:
        if (Image.ComponentType() == Image::UnsignedByte || Image.ComponentType() == Image::Byte)
        {
            return stbi_write_tga(OutputPath.generic_string().c_str(), Image.Width(), Image.Height(), Image.ComponentCount(), Image.Data()) > 0;
        }
        ENUM_OUT_OF_RANGE("TGA only supports int and uint 8 textures")

    case Image::BMP:
        if (Image.ComponentType() == Image::UnsignedByte || Image.ComponentType() == Image::Byte)
        {
            return stbi_write_bmp(OutputPath.generic_string().c_str(), Image.Width(), Image.Height(), Image.ComponentCount(), Image.Data()) > 0;
        }
        ENUM_OUT_OF_RANGE("Bitmap only supports int and uint 8 textures")

    case Image::HDR:
        if (Image.ComponentType() == Image::Float)
        {
            return stbi_write_hdr(OutputPath.generic_string().c_str(), Image.Width(), Image.Height(), Image.ComponentCount(), (float*) Image.Data()) > 0;
        }
        ENUM_OUT_OF_RANGE("HDR only supports float textures")

    case Image::EXR:
        if (Image.ComponentType() == Image::Float)
        {
            const char* err = NULL;
            int ret = SaveEXR((float*) Image.Data(), Image.Width(), Image.Height(), Image.ComponentCount(), false, OutputPath.generic_string().c_str(), &err);
            
            if (ret != TINYEXR_SUCCESS) 
            {
                if (err) 
                {
                    EngineLoggerErrorF("Load EXR err: %s(code %d)\n", err, ret);
                } 
                else 
                {
                    EngineLoggerErrorF("Load EXR err: code = %d\n", ret);
                }
                EngineRuntimeBREAKPOINT
        
                FreeEXRErrorMessage(err);
                return false;
            }
            
            return true;
        }
        ENUM_OUT_OF_RANGE("EXR only supports float textures")
        
    case Image::DDS:
    case Image::_Count:
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported export format")
    }
}

static struct stbi_out_buffer
{
    uint8_t** OutBuffer; size_t* Size;
};

static void stb_image_write_func(void *context, void *data, int size)
{    
    stbi_out_buffer* WriteBuffer = static_cast<stbi_out_buffer*>(context);
    
    AssertOrErrorCall(size > 0, return;, "Encoded image size is negative")
    
    *(WriteBuffer->OutBuffer) = (uint8_t*)malloc(size);
    *(WriteBuffer->Size) = size;
    
    memcpy( *(WriteBuffer->OutBuffer), data, size);
}

bool ImageStoreToMemory(const Image& Image, Image::FileType Type, uint8_t** OutBuffer, size_t* Size)
{
    stbi_out_buffer WriteBuffer{.OutBuffer = OutBuffer, .Size = Size};
    
    switch (Type)
    {
    case Image::JPG: 
        if (Image.ComponentType() == Image::UnsignedByte || Image.ComponentType() == Image::Byte)
        {
            return stbi_write_jpg_to_func(&stb_image_write_func, &WriteBuffer, Image.Width(), Image.Height(), Image.ComponentCount(), Image.Data(), 90 /*TODO expose*/) > 0;
        }
        ENUM_OUT_OF_RANGE("JPGs only supports int and uint 8 textures")

    case Image::PNG: 
        if (Image.ComponentType() == Image::UnsignedByte || Image.ComponentType() == Image::Byte)
        {
            return stbi_write_png_to_func(&stb_image_write_func, &WriteBuffer, Image.Width(), Image.Height(), Image.ComponentCount(), Image.Data(), Image.Width() * Image.PixelSize()) > 0;
        }
        ENUM_OUT_OF_RANGE("PNGs only supports int and uint 8 textures")

    case Image::TGA:
        if (Image.ComponentType() == Image::UnsignedByte || Image.ComponentType() == Image::Byte)
        {
            return stbi_write_tga_to_func(&stb_image_write_func, &WriteBuffer, Image.Width(), Image.Height(), Image.ComponentCount(), Image.Data()) > 0;
        }
        ENUM_OUT_OF_RANGE("TGA only supports int and uint 8 textures")

    case Image::BMP:
        if (Image.ComponentType() == Image::UnsignedByte || Image.ComponentType() == Image::Byte)
        {
            return stbi_write_bmp_to_func(&stb_image_write_func, &WriteBuffer, Image.Width(), Image.Height(), Image.ComponentCount(), Image.Data()) > 0;
        }
        ENUM_OUT_OF_RANGE("Bitmap only supports int and uint 8 textures")

    case Image::HDR:
        if (Image.ComponentType() == Image::Float)
        {
            return stbi_write_hdr_to_func(&stb_image_write_func, &WriteBuffer, Image.Width(), Image.Height(), Image.ComponentCount(), (float*) Image.Data()) > 0;
        }
        ENUM_OUT_OF_RANGE("HDR only supports float textures")

    case Image::EXR:
        if (Image.ComponentType() == Image::Float)
        {
            const char* err = NULL;
            
            int ret = SaveEXRToMemory((float*) Image.Data(), Image.Width(), Image.Height(), Image.ComponentCount(), /* TODO expose */ false, OutBuffer, &err);
            
            if (ret < 0) 
            {
                if (err) 
                {
                    EngineLoggerErrorF("Load EXR err: %s(code %d)\n", err, ret);
                } 
                else 
                {
                    EngineLoggerErrorF("Load EXR err: code = %d\n", ret);
                }
                EngineRuntimeBREAKPOINT
        
                FreeEXRErrorMessage(err);
                return false;
            }
            *Size = ret;
            
            return true;
        }
        ENUM_OUT_OF_RANGE("EXR only supports float textures")
        
    case Image::DDS:
    case Image::_Count:
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported export format")
    }
}

ImageCube::ImageCube(uint32_t Width, uint32_t Height, uint32_t Depth, Image::Type ComponentType, Image::Layout ComponentLayout, void* Data):
    m_Width(Width), m_Height(Height),  m_Depth(Depth),
    m_ComponentType(ComponentType),
    m_ComponentLayout(ComponentLayout),
    m_Data(m_Width * m_Height > 0 ? malloc(DataSize()) : nullptr)
    {
        if (Data != nullptr && m_Data != nullptr)
        {
            memcpy(m_Data, Data, DataSize());
        }
    }

ImageCube::~ImageCube()
{
    if (m_Data == nullptr) return;

    free(m_Data);
}

ImageCube::ImageCube(const ImageCube& other):
    m_Width(other.m_Width),
    m_Height(other.m_Height),
    m_Depth(other.m_Depth),
    m_ComponentType(other.m_ComponentType),
    m_ComponentLayout(other.m_ComponentLayout),
    m_Data(m_Width * m_Height > 0 ? malloc(DataSize()) : nullptr)
{
    memcpy(m_Data, other.m_Data, DataSize()); 
}

ImageCube::ImageCube(ImageCube&& other) noexcept:
    m_Width(other.m_Width),
    m_Height(other.m_Height),
    m_Depth(other.m_Depth),
    m_ComponentType(other.m_ComponentType),
    m_ComponentLayout(other.m_ComponentLayout),
    m_Data(other.m_Data)
{
    other.m_Data = nullptr;
}

ImageCube& ImageCube::operator=(ImageCube other)
{
    using std::swap;
    swap(*this, other);
    return *this;
}

uint32_t ImageCube::ChannelSize() const
{
    switch (m_ComponentType)
    {
    case Image::UnsignedByte:
    case Image::Byte:          return 1;
        
    case Image::UnsignedShort:
    case Image::Short:         return 2;
        
    case Image::UnsignedInt:
    case Image::Int:
    case Image::Float:         return 4;
    case Image::Double:        return 8;

    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported channel type")
    }
}

uint32_t ImageCube::PixelSize() const
{
    switch (m_ComponentLayout)
    {
    case Image::R:         return ChannelSize();
    case Image::RG:        return 2 * ChannelSize();
    case Image::RGB:
    case Image::BGR:       return 3 * ChannelSize();
    case Image::RGBA:
    case Image::ARGB:
    case Image::ABGR:      return 4 * ChannelSize();

    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported channel layout")
    }
}

size_t ImageCube::DataSize() const
{
    return static_cast<size_t>(m_Width) * m_Height * m_Depth * PixelSize();
}

ImageCube ImageLoadCube(const std::filesystem::path& ImagePath, Image::Type ComponentType)
{
    if (ImagePath.extension() == ".cub" || ImagePath.extension() == ".cube")
    {
       
    }

    ENUM_OUT_OF_RANGE("Unsupported extension for cube image loading")
}

CubeLUT::CubeLUT(uint32_t Size3D, const Math::Vector3d& DomainMin, const Math::Vector3d& DomainMax,Image::Type ComponentType, Image::Layout ComponentLayout, void* Data):
    ImageCube(Size3D, Size3D, Size3D, ComponentType, ComponentLayout, Data),
    m_DomainMax(DomainMax),
    m_DomainMin(DomainMin)
{}

CubeLUT::~CubeLUT() = default;

CubeLUT::CubeLUT(const CubeLUT& other):
    ImageCube(other),
    m_DomainMax(other.m_DomainMax),
    m_DomainMin(other.m_DomainMin)
{}

CubeLUT::CubeLUT(CubeLUT&& other) noexcept:
    ImageCube(std::move(other)),
    m_DomainMax(std::move(other.m_DomainMax)),
    m_DomainMin(std::move(other.m_DomainMin))
{}

CubeLUT ImageLoadCubeLUT(const std::filesystem::path& ImagePath)
{
    std::string line;
    std::vector<float> data;
    uint32_t size = 0;
    std::ifstream file;
    Math::Vector3d domainMin{ 0.0f };
    Math::Vector3d domainMax{ 1.0f };

    AssertOrErrorCallF(exists(ImagePath), goto EmptyLut,
        "Could not open image %s, No such file or directory", ImagePath.generic_string().c_str())
    
    if (ImagePath.extension() == ".cub" || ImagePath.extension() == ".cube")
    {
        file = std::ifstream(ImagePath);

        AssertOrErrorCallF(file.is_open(), goto EmptyLut,
            "Failed to open image %s", ImagePath.generic_string().c_str())
        
        while (std::getline(file, line)) {
            // Remove comments
            if (line.empty() || line[0] == '#')
                continue;

            // Trim leading spaces
            line.erase(0, line.find_first_not_of(" \t\r\n"));

            if (line.empty() || line[0] == '#')
                continue;

            std::istringstream iss(line);
            std::string keyword;
            iss >> keyword;

            if (keyword == "TITLE") {
                // Could parse title if needed, ignore for now
                continue;
            }
            else if (keyword == "LUT_3D_SIZE") {
                iss >> size;
                data.reserve(static_cast<size_t>(size) * size * size * 3);
            }
            else if (keyword == "DOMAIN_MIN") {
                iss >> domainMin[0] >> domainMin[1] >> domainMin[2];
            }
            else if (keyword == "DOMAIN_MAX") {
                iss >> domainMax[0] >> domainMax[1] >> domainMax[2];
            }
            else {
                // Not a keyword - should be LUT data entries: 3 floats per line
                float r, g, b;
                // Try to parse RGB from this line again using the full line string
                std::istringstream colorStream(line);
                if (colorStream >> r >> g >> b) {
                    data.push_back(r);
                    data.push_back(g);
                    data.push_back(b);
                } else {
                    // Invalid line, ignore or stop loading
                    // We'll ignore here
                }
            }
        }

        AssertOrErrorCall(size > 0, goto EmptyLut, "LUT size must be greater than zero")
        AssertOrErrorCallF(data.size() / 3 == (static_cast<size_t>(size) * size * size), goto EmptyLut,
            "LUT data size mismatch. Expected %llu got %llu. Lut size must be a square", (static_cast<size_t>(size) * size * size), data.size() / 3)
        

        return CubeLUT(size, domainMin, domainMax, Image::Float, Image::RGB, data.data());

    EmptyLut:

        // TODO maybe generate the identity LUT
        return CubeLUT(0, Math::Vector3d(0), Math::Vector3d(1.0), Image::Float, Image::BGR, nullptr);
    }

    ENUM_OUT_OF_RANGE("Unsupported extension for cube image loading")
}
