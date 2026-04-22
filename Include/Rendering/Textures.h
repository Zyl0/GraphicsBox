#pragma once

#include <span>

#include "GLHelper.h"

#include "Shared/Annotations.h"
#include "Image/Image.h"

namespace Texture
{
    enum Type : uint8_t
    {
        UnsignedByte = 0,
        Byte,
        UnsignedShort,
        Short,
        UnsignedInt,
        Int,
        Half,
        Float,

        Packed_R3_G3_B2,
        Packed_RGB5_A1,
        Packed_RGB10_A2,
        Packed_A2_RGB10,
        Packed_R11F_G11F_B10F,
        Packed_RGB9_E9,
        Packed_D24_S8,
        Packed_D32F_S8_E24
    };

    enum Layout : uint8_t
    {
        R,
        RG,
        RGB,
        BGR,
        RGBA,
        ARGB,
        ABGR,
        D,
        S,
        DS
    };
    
    static Texture::Type ToTextureType(Image::Type type);
    static Texture::Layout ToTextureLayout(Image::Layout type);
    static Texture::Layout LayoutFromPackedType(Texture::Type type);
}

GLenum ToGLTextureType(Texture::Type type, Texture::Layout layout);
GLenum ToGLTextureLayout(Texture::Type type, Texture::Layout layout);
GLenum ToGPUTextureType(Texture::Type type, Texture::Layout layout);

class Texture2D
{
public:
    Texture2D(uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout, uint8_t SampleCount = 0);
    Texture2D(const Image& Image, bool UseMips = true);
    Texture2D(uint32_t width, uint32_t height, Image::Type type, Image::Layout layout, const void* ImageData, size_t ImageSize, bool UseMips = true);
    ~Texture2D();

    void Data(uint32_t width, uint32_t height);
    void Data(uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout, uint8_t SampleCount = 0);
    void Data(const Image& Image, bool UseMips = true);
    void Data(Image::Type type, Image::Layout layout, const void* ImageData, size_t ImageSize, bool UseMips = true);
    void Data(uint8_t SampleCount);
    void Export(Image& Export);

    INLINE GLuint Handle() const                    { return m_Texture;}
    INLINE uint32_t Width() const                   { return m_Width;}
    INLINE uint32_t Height() const                  { return m_Height;}
    INLINE uint32_t MipCount() const                { return m_MipCount;}
    INLINE Texture::Type ComponentType() const      { return m_Type;}
    INLINE Texture::Layout ComponentLayout() const  { return m_Layout;}
    INLINE uint8_t SampleCount() const             { return m_SampleCount;}
    INLINE GLenum GLType() const                    { return ToGLTextureType(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout); }
    INLINE GLenum GLFormat() const                  { return ToGLTextureLayout(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout); }
    INLINE GLenum GPUType() const                   { return ToGPUTextureType(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout); }
        
private:
    GLuint m_Texture;

    uint32_t m_Width;
    uint32_t m_Height;
    uint32_t m_MipCount;

    Texture::Type m_Type;
    Texture::Layout m_Layout;
    uint8_t m_SampleCount = 1;
};

void Bind(const Texture2D& texture);
void UnBind(const Texture2D& texture);

class WriteOnlyTexture2D
{
public:
    WriteOnlyTexture2D(uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout, uint8_t SampleCount = 0);
    ~WriteOnlyTexture2D();
    
    void Data(uint32_t width, uint32_t height);
    void Data(uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout, uint8_t SampleCount = 0);
    void Data(uint8_t SampleCount);
    
    INLINE GLuint Handle() const                    { return m_Texture;}
    INLINE uint32_t Width() const                   { return m_Width;}
    INLINE uint32_t Height() const                  { return m_Height;}
    INLINE Texture::Type ComponentType() const      { return m_Type;}
    INLINE Texture::Layout ComponentLayout() const  { return m_Layout;}
    INLINE uint8_t SampleCount() const              { return m_SampleCount;}
    INLINE GLenum GLType() const                    { return ToGLTextureType(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout); }
    INLINE GLenum GLFormat() const                  { return ToGLTextureLayout(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout); }
    INLINE GLenum GPUType() const                   { return ToGPUTextureType(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout); }
        
private:
    GLuint m_Texture;

    uint32_t m_Width;
    uint32_t m_Height;

    Texture::Type m_Type;
    Texture::Layout m_Layout;
    uint8_t m_SampleCount = 1;
};

void Bind(const WriteOnlyTexture2D& texture);
void UnBind(const WriteOnlyTexture2D& texture);

class Texture2DArray
{
public:
    Texture2DArray(uint32_t width, uint32_t height, uint32_t count, Texture::Type type, Texture::Layout layout, bool UseMips = false);
    ~Texture2DArray();

    void Data(uint32_t count, bool move = false);
    void Data(uint32_t width, uint32_t height);
    void Data(uint32_t width, uint32_t height, uint32_t count, Texture::Type type, Texture::Layout layout, bool UseMips = false);
    void SubData(uint32_t index, const Image& Image);
    void SubData(uint32_t index, Image::Type type, Image::Layout layout, const void* ImageData, size_t ImageSize);
    void UpdateMips();
    void ExportSub(uint32_t index, Image& Export) const;

    INLINE GLuint Handle() const                    { return m_Texture;}
    INLINE uint32_t Width() const                   { return m_Width;}
    INLINE uint32_t Height() const                  { return m_Height;}
    INLINE uint32_t Count() const                   { return m_LayerCount;}
    INLINE uint32_t MipCount() const                { return m_MipCount;}
    INLINE Texture::Type ComponentType() const      { return m_Type;}
    INLINE Texture::Layout ComponentLayout() const  { return m_Layout;}
    INLINE GLenum GLType() const                    { return ToGLTextureType(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout); }
    INLINE GLenum GLFormat() const                  { return ToGLTextureLayout(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout); }
    INLINE GLenum GPUType() const                   { return ToGPUTextureType(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout); }
        
private:
    GLuint m_Texture;

    uint32_t m_Width;
    uint32_t m_Height;
    uint32_t m_LayerCount;
    uint32_t m_MipCount;

    Texture::Type m_Type;
    Texture::Layout m_Layout;
};

void Bind(const Texture2DArray& texture);
void UnBind(const Texture2DArray& texture);

class Texture3D
{
public:
    Texture3D(uint32_t width, uint32_t height, uint32_t depth, Texture::Type type, Texture::Layout layout);
    Texture3D(const ImageCube& Image, bool UseMips = true);
    ~Texture3D();

    void Data(uint32_t width, uint32_t height, uint32_t depth);
    void Data(uint32_t width, uint32_t height, uint32_t depth, Texture::Type type, Texture::Layout layout);
    void Data(const ImageCube& Image, bool UseMips = true);
    void Export(ImageCube& Export);

    INLINE GLuint Handle() const                    { return m_Texture; }
    INLINE uint32_t Width() const                   { return m_Width;}
    INLINE uint32_t Height() const                  { return m_Height;}
    INLINE uint32_t Depth() const                   { return m_Depth;}
    INLINE uint32_t MipCount() const                { return m_MipCount;}
    INLINE Texture::Type ComponentType() const      { return m_Type;}
    INLINE Texture::Layout ComponentLayout() const  { return m_Layout;}
    INLINE GLenum GLType() const { return ToGLTextureType(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout); }
    INLINE GLenum GLFormat() const { return ToGLTextureLayout(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout); }
    INLINE GLenum GPUType() const { return ToGPUTextureType(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout); }

private:
    GLuint m_Texture;

    uint32_t m_Width;
    uint32_t m_Height;
    uint32_t m_Depth;
    uint32_t m_MipCount;

    Texture::Type m_Type;
    Texture::Layout m_Layout;
};

void Bind(const Texture3D& texture);
void UnBind(const Texture3D& texture);

class TextureCube
{
public:
    enum Face : uint8_t
    {
        Right = 0,
        Left,
        Up,
        Down,
        Back,
        Front,
        _Count
    };

    using FacePair = std::pair<Face, const Image&>;
    
    TextureCube(uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout, bool UseMips = false);
    TextureCube(std::span<const FacePair> Faces, bool UseMips = true);
    ~TextureCube();

    void Data(std::span<const FacePair> Faces);

    INLINE GLuint Handle() const                    { return m_Texture; }
    INLINE uint32_t Width() const                   { return m_Width;}
    INLINE uint32_t Height() const                  { return m_Height;}
    INLINE bool HasMips() const                     { return m_UseMips && m_MipCount > 0; }
    INLINE uint32_t MipCount() const                { return m_MipCount;}
    INLINE Texture::Type ComponentType() const      { return m_Type;}
    INLINE Texture::Layout ComponentLayout() const  { return m_Layout;}
    INLINE GLenum GLType() const { return ToGLTextureType(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout); }
    INLINE GLenum GLFormat() const { return ToGLTextureLayout(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout); }
    INLINE GLenum GPUType() const { return ToGPUTextureType(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout); }

private:
    GLuint m_Texture;

    uint32_t m_Width;
    uint32_t m_Height;
    uint32_t m_MipCount;

    Texture::Type m_Type;
    Texture::Layout m_Layout;
    bool m_UseMips;
};

void Bind(const TextureCube& texture);
void UnBind(const TextureCube& texture);

class TextureCubeView
{
public:
    TextureCubeView(const TextureCube& texture, uint32_t MipLevel, uint32_t MipCount = 1);
    ~TextureCubeView();
    
    // Retarget view
    void Data(const TextureCube& texture, uint32_t MipLevel, uint32_t MipCount = 1);
    
    INLINE GLuint Handle() const                    { return m_Texture; }
    INLINE uint32_t Width() const                   { return m_Width;}
    INLINE uint32_t Height() const                  { return m_Height;}
    INLINE bool HasMips() const                     { return m_MipCount > 1; }
    INLINE uint32_t BaseMip() const                 { return m_BaseMip;}
    INLINE uint32_t MipCount() const                { return m_MipCount;}
    INLINE Texture::Type ComponentType() const      { return m_Type;}
    INLINE Texture::Layout ComponentLayout() const  { return m_Layout;}
    INLINE GLenum GLType() const { return ToGLTextureType(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout); }
    INLINE GLenum GLFormat() const { return ToGLTextureLayout(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout); }
    INLINE GLenum GPUType() const { return ToGPUTextureType(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout); }
    
private:
    uint32_t m_Width;
    uint32_t m_Height;
    uint32_t m_BaseMip;
    uint32_t m_MipCount;

    Texture::Type m_Type;
    Texture::Layout m_Layout;
    GLuint m_Texture;
};

void Bind(const TextureCubeView& texture);
void UnBind(const TextureCubeView& texture);