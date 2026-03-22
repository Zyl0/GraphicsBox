#include "Image.h"

#include <fstream>
#include <stb_image.h>
#include <stb_image_write.h>

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

Image::Image(uint32_t Width, uint32_t Height, Type ComponentType, Layout ComponentLayout, const void* Data) :
    m_Width(Width), m_Height(Height),
    m_ComponentType(ComponentType),
    m_ComponentLayout(ComponentLayout),
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

Image ImageLoad(const std::filesystem::path& ImagePath, Image::Type ComponentType)
{
    AssertOrErrorCallF(exists(ImagePath), return Image(0,0, ComponentType, Image::R),
        "Could not open image %s, No such file or directory", ImagePath.generic_string().c_str())

    int Width = 0, Height = 0, ChannelCount = 0;
    void* Buffer;
    
    switch (ComponentType)
    {
    case Image::UnsignedByte:
        Buffer = stbi_load(ImagePath.generic_string().c_str(), &Width, &Height, &ChannelCount, 0);
        break;
        
    case Image::UnsignedShort:
        Buffer = stbi_load_16(ImagePath.generic_string().c_str(), &Width, &Height, &ChannelCount, 0);
        break;
        
    case Image::Float:
        Buffer = stbi_loadf(ImagePath.generic_string().c_str(), &Width, &Height, &ChannelCount, 0);
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
    Image Image(Width, Height, ComponentType, Layout, Buffer);

    stbi_image_free(Buffer);

    return Image;
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

        AssertOrErrorCallF(size > 0, goto EmptyLut, "LUT size must be greater than zero")
        AssertOrErrorCallF(data.size() / 3 == (static_cast<size_t>(size) * size * size), goto EmptyLut,
            "LUT data size mismatch. Expected %llu got %llu. Lut size must be a square", (static_cast<size_t>(size) * size * size), data.size() / 3)
        

        return CubeLUT(size, domainMin, domainMax, Image::Float, Image::RGB, data.data());

    EmptyLut:

        // TODO maybe generate the identity LUT
        return CubeLUT(0, Math::Vector3d(0), Math::Vector3d(1.0), Image::Float, Image::BGR, nullptr);
    }

    ENUM_OUT_OF_RANGE("Unsupported extension for cube image loading")
}
