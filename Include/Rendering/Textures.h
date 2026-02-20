#pragma once

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
    Texture2D(uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout);
    Texture2D(const Image& Image, bool UseMips = true);
    ~Texture2D();

    void Data(uint32_t width, uint32_t height);
    void Data(uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout);
    void Data(const Image& Image, bool UseMips = true);
    void Export(Image& Export);

    INLINE GLuint Handle() const                    { return m_Texture;}
    INLINE uint32_t Width() const                   { return m_Width;}
    INLINE uint32_t Height() const                  { return m_Height;}
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
    uint32_t m_MipCount;

    Texture::Type m_Type;
    Texture::Layout m_Layout;
};

void Bind(const Texture2D& texture);
void UnBind(const Texture2D& texture);

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