#pragma once

#include <cstdint>
#include <filesystem>
#include <utility>

#include "Math/Vector.h"

struct Image
{
    enum Type : uint8_t
    {
        UnsignedByte = 0,
        Byte = 1,
        UnsignedShort = 2,
        Short = 3,
        UnsignedInt = 4,
        Int = 5,
        Float = 6,
        Double = 7,
    };

    enum Layout : uint8_t
    {
        R,
        RG,
        RGB,
        BGR,
        RGBA,
        ARGB,
        ABGR
    };

    enum FileType : uint8_t
    {
        JPG, PNG, TGA, BMP, HDR, _Count
    };

    Image(uint32_t Width, uint32_t Height, Type ComponentType, Layout ComponentLayout, void* Data = nullptr);
    ~Image();

    Image(const Image& other);

    Image(Image&& other) noexcept;

    Image& operator=(Image other);

    uint32_t Width() const { return m_Width; }
    uint32_t Height() const { return m_Height; }
    Type ComponentType() const { return m_ComponentType; }
    Layout ComponentLayout() const { return m_ComponentLayout; }

    uint32_t ChannelSize() const;
    uint32_t PixelSize() const;
    uint32_t ComponentCount() const;
    size_t DataSize() const;
    void* Data() const { return m_Data; }

    friend void swap(Image& first, Image& second) noexcept
    {
        using std::swap;
        swap(first.m_Width, second.m_Width);
        swap(first.m_Height, second.m_Height);
        swap(first.m_ComponentType, second.m_ComponentType);
        swap(first.m_ComponentLayout, second.m_ComponentLayout);
        swap(first.m_Data, second.m_Data);
    }

private:
    uint32_t m_Width, m_Height;
    Type m_ComponentType;
    Layout m_ComponentLayout;
    void* m_Data;
};

Image ImageLoad(const std::filesystem::path& ImagePath, Image::Type ComponentType);

bool ImageStore(const std::filesystem::path& OutputPath, const Image& Image, Image::FileType Type);

struct ImageCube
{
    ImageCube(uint32_t Width, uint32_t Height, uint32_t Depth, Image::Type ComponentType, Image::Layout ComponentLayout, void* Data = nullptr);
    ~ImageCube();

    ImageCube(const ImageCube& other);

    ImageCube(ImageCube&& other) noexcept;

    ImageCube& operator=(ImageCube other);

    uint32_t Width() const { return m_Width; }
    uint32_t Height() const { return m_Height; }
    uint32_t Depth() const { return m_Depth; }
    Image::Type ComponentType() const { return m_ComponentType; }
    Image::Layout ComponentLayout() const { return m_ComponentLayout; }

    uint32_t ChannelSize() const;
    uint32_t PixelSize() const;
    size_t DataSize() const;
    void* Data() const { return m_Data; }

    friend void swap(ImageCube& first, ImageCube& second) noexcept
    {
        using std::swap;
        swap(first.m_Width, second.m_Width);
        swap(first.m_Height, second.m_Height);
        swap(first.m_Depth, second.m_Depth);
        swap(first.m_ComponentType, second.m_ComponentType);
        swap(first.m_ComponentLayout, second.m_ComponentLayout);
        swap(first.m_Data, second.m_Data);
    }

private:
    uint32_t m_Width, m_Height, m_Depth;
    Image::Type m_ComponentType;
    Image::Layout m_ComponentLayout;
    void* m_Data;
};

ImageCube ImageLoadCube(const std::filesystem::path& ImagePath, Image::Type ComponentType);

struct CubeLUT : public ImageCube
{
    CubeLUT(uint32_t Size3D, const Math::Vector3d& DomainMin, const Math::Vector3d& DomainMax, Image::Type ComponentType, Image::Layout ComponentLayout, void* Data = nullptr);
    ~CubeLUT();

    CubeLUT(const CubeLUT& other);

    CubeLUT(CubeLUT&& other) noexcept;

    CubeLUT& operator=(CubeLUT other)
    {
        using std::swap;
        swap(*this, other);
        return *this;
    }

    const Math::Vector3d& DomainMax() const {return m_DomainMax;}
    const Math::Vector3d& DomainMin() const {return m_DomainMin;}

    friend void swap(CubeLUT& first, CubeLUT& second) noexcept
    {
        using std::swap;
        swap((ImageCube&)first, (ImageCube&)second);
        swap(first.m_DomainMax, second.m_DomainMax);
        swap(first.m_DomainMin, second.m_DomainMin);
    }
private:
    Math::Vector3d m_DomainMax = {1.0f};
    Math::Vector3d m_DomainMin = {1.0f};
};

CubeLUT ImageLoadCubeLUT(const std::filesystem::path& ImagePath);