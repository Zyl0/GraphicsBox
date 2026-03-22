#include "Textures.h"

#include "Shared/Annotations.h"

GLenum ToGLTextureType(Texture::Type type, Texture::Layout layout)
{    
    switch (type)
    {
    case Texture::UnsignedByte:           return GL_UNSIGNED_BYTE;
    case Texture::Byte:                   return GL_BYTE;
    case Texture::UnsignedShort:          return GL_UNSIGNED_SHORT;
    case Texture::Short:                  return GL_SHORT;
    case Texture::UnsignedInt:            return GL_UNSIGNED_INT;
    case Texture::Int:                    return GL_INT;
    case Texture::Float:                  return GL_FLOAT;
    case Texture::Half:                   return GL_HALF_FLOAT;

        
    case Texture::Packed_R3_G3_B2:        return GL_UNSIGNED_BYTE_3_3_2;
    case Texture::Packed_RGB5_A1:         return GL_UNSIGNED_SHORT_5_5_5_1;
    case Texture::Packed_RGB10_A2:        return GL_UNSIGNED_INT_10_10_10_2;
    case Texture::Packed_A2_RGB10:        return GL_UNSIGNED_INT_2_10_10_10_REV;
    case Texture::Packed_R11F_G11F_B10F:  return GL_FLOAT;
    case Texture::Packed_RGB9_E9:         return GL_UNSIGNED_INT_5_9_9_9_REV;
    case Texture::Packed_D24_S8:          return GL_DEPTH24_STENCIL8;
    case Texture::Packed_D32F_S8_E24:     return GL_UNSIGNED_INT_24_8;
        
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported opengl type")
    }
}

GLenum ToGLTextureLayout(Texture::Type type, Texture::Layout layout)
{
    if (layout == Texture::D)
    {
        return GL_DEPTH_COMPONENT;
    }
    if (layout == Texture::S)
    {
        return GL_STENCIL_INDEX;
    }

    if (layout == Texture::DS)
    {
        return GL_DEPTH_STENCIL;
    }

    switch (type)
    {
    case Texture::UnsignedByte:
    case Texture::Byte:
    case Texture::UnsignedShort:
    case Texture::Short:
    case Texture::UnsignedInt:
    case Texture::Int:
        switch (layout)
        {
        case Texture::R:        return GL_RED_INTEGER;
        case Texture::RG:       return GL_RED_INTEGER;
        case Texture::RGB:      return GL_RED_INTEGER;
        case Texture::BGR:      return GL_RED_INTEGER;
        case Texture::RGBA:     return GL_RED_INTEGER;
            
        case Texture::ARGB:
        case Texture::ABGR:
        case Texture::D:
        case Texture::S:
        case Texture::DS:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported opengl layout")
        }
        
    case Texture::Half:
    case Texture::Float:
        switch (layout)
        {
        case Texture::R:        return GL_RED;
        case Texture::RG:       return GL_RG;
        case Texture::RGB:      return GL_RGB;
        case Texture::BGR:      return GL_BGR;
        case Texture::RGBA:     return GL_RGBA;
                
        case Texture::ARGB:
        case Texture::ABGR:
        case Texture::D:
        case Texture::S:
        case Texture::DS:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported opengl layout")
        }
        
    case Texture::Packed_R3_G3_B2:          return GL_RGB_INTEGER;
    case Texture::Packed_RGB5_A1:
    case Texture::Packed_RGB10_A2:
    case Texture::Packed_A2_RGB10:          return GL_RGBA;
    case Texture::Packed_R11F_G11F_B10F:    return GL_RGB;
    case Texture::Packed_RGB9_E9:           return GL_RGBA_INTEGER;
    case Texture::Packed_D24_S8:
    case Texture::Packed_D32F_S8_E24:       return GL_DEPTH_STENCIL;

    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported opengl type")
    }    
}

GLenum ToGPUTextureType(Texture::Type type, Texture::Layout layout)
{
    if (layout == Texture::D)
    {
        return GL_DEPTH_COMPONENT;
    }
    if (layout == Texture::S)
    {
        return GL_STENCIL_INDEX8;
    }

    if (layout == Texture::DS)
    {
        if (type == Texture::Packed_D24_S8)
        {
            return GL_DEPTH24_STENCIL8;
        }
        if (type == Texture::Packed_D32F_S8_E24)
        {
            return GL_DEPTH32F_STENCIL8;
        }
        
        ENUM_OUT_OF_RANGE("Unsupported depth stencil type")
    }
    
    switch (type)
    {
    case Texture::UnsignedByte:
        switch (layout)
        {
        case Texture::R:        return GL_R8UI;
        case Texture::RG:       return GL_RG8UI;
        case Texture::RGB:
        case Texture::BGR:      return GL_RGB8UI;
        case Texture::RGBA:
        case Texture::ARGB:     return GL_RGBA8UI;
        case Texture::ABGR:     return GL_RGBA8UI;
                            
        case Texture::D:
        case Texture::S:
        case Texture::DS:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported opengl layout")
        }
        
    case Texture::Byte:
        switch (layout)
        {
        case Texture::R:        return GL_R8I;
        case Texture::RG:       return GL_RG8I;
        case Texture::RGB:
        case Texture::BGR:      return GL_RGB8I;
        case Texture::RGBA:
        case Texture::ARGB:     return GL_RGBA8I;
        case Texture::ABGR:     return GL_RGBA8I;
                        
        case Texture::D:
        case Texture::S:
        case Texture::DS:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported opengl layout")
        }
        
    case Texture::UnsignedShort:
        switch (layout)
        {
        case Texture::R:        return GL_R16UI;
        case Texture::RG:       return GL_RG16UI;
        case Texture::RGB:
        case Texture::BGR:      return GL_RGB16UI;
        case Texture::RGBA:
        case Texture::ARGB:     return GL_RGBA16UI;
        case Texture::ABGR:     return GL_RGBA16UI;
                        
        case Texture::D:
        case Texture::S:
        case Texture::DS:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported opengl layout")
        }
        
    case Texture::Short:
        switch (layout)
        {
        case Texture::R:        return GL_R16I;
        case Texture::RG:       return GL_RG16I;
        case Texture::RGB:
        case Texture::BGR:      return GL_RGB16I;
        case Texture::RGBA:
        case Texture::ARGB:     return GL_RGBA16I;
        case Texture::ABGR:     return GL_RGBA16I;
                        
        case Texture::D:
        case Texture::S:
        case Texture::DS:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported opengl layout")
        }
        
    case Texture::UnsignedInt:
        switch (layout)
        {
        case Texture::R:        return GL_R32UI;
        case Texture::RG:       return GL_RG32UI;
        case Texture::RGB:
        case Texture::BGR:      return GL_RGB32UI;
        case Texture::RGBA:
        case Texture::ARGB:     return GL_RGBA32UI;
        case Texture::ABGR:     return GL_RGBA32UI;
                        
        case Texture::D:
        case Texture::S:
        case Texture::DS:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported opengl layout")
        }
        
    case Texture::Int:
        switch (layout)
        {
        case Texture::R:        return GL_R32I;
        case Texture::RG:       return GL_RG32I;
        case Texture::RGB:
        case Texture::BGR:      return GL_RGB32I;
        case Texture::RGBA:
        case Texture::ARGB:     return GL_RGBA32I;
        case Texture::ABGR:     return GL_RGBA32I;
                        
        case Texture::D:
        case Texture::S:
        case Texture::DS:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported opengl layout")
        }
    case Texture::Float:
        switch (layout)
        {
        case Texture::R:        return GL_R32F;
        case Texture::RG:       return GL_RG32F;
        case Texture::RGB:
        case Texture::BGR:      return GL_RGB32F;
        case Texture::RGBA:
        case Texture::ARGB:     return GL_RGBA32F;
        case Texture::ABGR:     return GL_RGBA32F;
                        
        case Texture::D:
        case Texture::S:
        case Texture::DS:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported opengl layout")
        }
        
    case Texture::Half:
        switch (layout)
        {
        case Texture::R:        return GL_R16F;
        case Texture::RG:       return GL_RG16F;
        case Texture::RGB:
        case Texture::BGR:      return GL_RGB16F;
        case Texture::RGBA:
        case Texture::ARGB:     return GL_RGBA16F;
        case Texture::ABGR:     return GL_RGBA16F;
                    
        case Texture::D:
        case Texture::S:
        case Texture::DS:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported opengl layout")
        }
        
    case Texture::Packed_R3_G3_B2:        return GL_R3_G3_B2;
    case Texture::Packed_RGB5_A1:         return GL_RGB5_A1;
    case Texture::Packed_RGB10_A2:        return GL_RGB10_A2;
    case Texture::Packed_A2_RGB10:        return  GL_RGB10_A2;
    case Texture::Packed_R11F_G11F_B10F:  return GL_R11F_G11F_B10F;
    case Texture::Packed_RGB9_E9:         return GL_RGB9_E5;
    case Texture::Packed_D24_S8:          return GL_DEPTH24_STENCIL8;
    case Texture::Packed_D32F_S8_E24:     return GL_DEPTH32F_STENCIL8;
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported packed type")
    }
}

Texture::Type Texture::ToTextureType(Image::Type type)
{
    switch (type)
    {
    case Image::UnsignedByte:       return Texture::UnsignedByte;
    case Image::Byte:               return Texture::Byte;
    case Image::UnsignedShort:      return Texture::UnsignedShort;
    case Image::Short:              return Texture::Short;
    case Image::UnsignedInt:        return Texture::UnsignedInt;
    case Image::Int:                return Texture::Int;
    case Image::Float:              return Texture::Float;
        
    case Image::Double:
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported texture type in GPU")
    }
}

Texture::Layout Texture::ToTextureLayout(Image::Layout type)
{
    switch (type)
    {
    case Image::R:              return Texture::R;
    case Image::RG:             return Texture::RG;
    case Image::RGB:            return Texture::RGB;
    case Image::BGR:            return Texture::BGR;
    case Image::RGBA:           return Texture::RGBA;
    case Image::ARGB:           return Texture::ARGB;
    case Image::ABGR:           return Texture::ABGR;

    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported texture layout in GPU")
    }
}

Texture::Layout Texture::LayoutFromPackedType(Texture::Type type)
{
    switch (type)
    {
    case Packed_R3_G3_B2:       return RGB;
    case Packed_RGB5_A1:
    case Packed_RGB10_A2:       return RGBA;
    case Packed_R11F_G11F_B10F:
    case Packed_RGB9_E9:        return RGB;
    case Packed_D24_S8:
    case Packed_D32F_S8_E24:    return DS;
    
    case UnsignedByte:
    case Byte:
    case UnsignedShort:
    case Short:
    case UnsignedInt:
    case Int:
    case Half:
    case Float:
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Given Type is not a packed type")
    }
}

Texture2D::Texture2D(uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout):
    m_Width(width), m_Height(height),
    m_MipCount(0), m_Type(type),
    m_Layout(layout)
{
    GLCall(glGenTextures(1, &m_Texture))

    Bind(*this);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_MipCount);

    GLCall(glTexImage2D(GL_TEXTURE_2D, 0,
        ToGPUTextureType(type, layout), Width(), Height(), 0,
        ToGLTextureLayout(type, layout), ToGLTextureType(type, layout), nullptr))

    if (m_MipCount > 0)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    UnBind(*this);
}

Texture2D::Texture2D(const Image& Image, bool UseMips):
    m_Width(Image.Width()), m_Height(Image.Height()),
    m_MipCount(UseMips ? miplevels(m_Width, m_Height) : 0), m_Type(Texture::ToTextureType(Image.ComponentType())),
    m_Layout(Texture::ToTextureLayout(Image.ComponentLayout()))
{
    GLCall(glGenTextures(1, &m_Texture))

    Bind(*this);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_MipCount);

    GLCall(glTexImage2D(GL_TEXTURE_2D, 0,
        ToGPUTextureType(m_Type, m_Layout), Width(), Height(), 0,
        ToGLTextureLayout(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout), Image.Data()))

    if (m_MipCount > 0)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    UnBind(*this);
}

Texture2D::Texture2D(uint32_t width, uint32_t height, Image::Type type, Image::Layout layout, const void* ImageData, size_t ImageSize, bool UseMips):
    m_Width(width), m_Height(height),
    m_MipCount(UseMips ? miplevels(m_Width, m_Height) : 0), m_Type(Texture::ToTextureType(type)),
    m_Layout(Texture::ToTextureLayout(layout))
{
    GLCall(glGenTextures(1, &m_Texture))
    
    AssertOrErrorCall(m_Width * m_Height * Image::PixelSize(type, layout) == ImageSize, return;, "Texture and image data size missmatch")

    Bind(*this);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_MipCount);

    GLCall(glTexImage2D(GL_TEXTURE_2D, 0,
        ToGPUTextureType(m_Type, m_Layout), Width(), Height(), 0,
        ToGLTextureLayout(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout), ImageData))

    if (m_MipCount > 0)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    UnBind(*this);
}

Texture2D::~Texture2D()
{
    GLCall(glDeleteTextures(1, &m_Texture))
}

void Texture2D::Data(uint32_t width, uint32_t height)
{
    if (width == Width() && height == Height()) return;

    Data(width, height, m_Type, m_Layout);
}

void Texture2D::Data(uint32_t width, uint32_t height, Texture::Type type, Texture::Layout layout)
{
    m_Type = type;
    m_Layout = layout;
    m_Width = width;
    m_Height = height;
    m_MipCount = 0;
    
    Bind(*this);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_MipCount);

    GLCall(glTexImage2D(GL_TEXTURE_2D, 0,
        ToGPUTextureType(m_Type, m_Layout), Width(), Height(), 0,
        ToGLTextureLayout(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout), nullptr))

    if (m_MipCount > 0)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    UnBind(*this);
}

void Texture2D::Data(const Image& Image, bool UseMips)
{
    m_Type = Texture::ToTextureType(Image.ComponentType());
    m_Layout = Texture::ToTextureLayout(Image.ComponentLayout());
    m_Width = Image.Width();
    m_Height = Image.Height();
    m_MipCount = UseMips ? miplevels(m_Width, m_Height) : 0;

    Bind(*this);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_MipCount);

    GLCall(glTexImage2D(GL_TEXTURE_2D, 0,
        ToGPUTextureType(m_Type, m_Layout), Width(), Height(), 0,
        ToGLTextureLayout(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout), Image.Data()))

    if (m_MipCount > 0)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    UnBind(*this);
}

void Texture2D::Data( Image::Type type, Image::Layout layout, const void* ImageData, size_t ImageSize, bool UseMips)
{
    m_Type = Texture::ToTextureType(type);
    m_Layout = Texture::ToTextureLayout(layout);
    
    AssertOrErrorCall(m_Width * m_Height * Image::PixelSize(type, layout) == ImageSize, return;, "Texture and image data size missmatch")
    m_MipCount = UseMips ? miplevels(m_Width, m_Height) : 0;

    Bind(*this);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, m_MipCount);

    GLCall(glTexImage2D(GL_TEXTURE_2D, 0,
        ToGPUTextureType(m_Type, m_Layout), Width(), Height(), 0,
        ToGLTextureLayout(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout), ImageData))

    if (m_MipCount > 0)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    UnBind(*this);
}

void Texture2D::Export(Image& Export)
{
    AssertOrErrorCall(ComponentLayout() == Texture::ToTextureLayout(Export.ComponentLayout()), return;, "Output image Format doesn't match GPU image Format");
    bool IsFormatCompatible = false;
    IsFormatCompatible |= ComponentType() == Texture::ToTextureType(Export.ComponentType());
    IsFormatCompatible |= ComponentType() == Texture::Half && Texture::ToTextureType(Export.ComponentType()) == Texture::Float;
    AssertOrErrorCall(IsFormatCompatible, return;, "Output image Type doesn't match GPU image Type")
    AssertOrErrorCallF(Width() == Export.Width() && Height() == Export.Height(), return;, "Export texture resolution missmatch. GPU: %ux%u Export image: %ux%u", Width(), Height(), Export.Width(), Export.Height())

    Bind(*this);
    GLCall(glGetnTexImage(GL_TEXTURE_2D , 0,
        ToGLTextureLayout(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout),
        static_cast<GLsizei>(Export.DataSize()), Export.Data()));
    UnBind(*this);
}

void Bind(const Texture2D& texture)
{
    GLCall(glBindTexture(GL_TEXTURE_2D, texture.Handle()))
}

void UnBind(const Texture2D& texture)
{
    GLCall(glBindTexture(GL_TEXTURE_2D, 0))
}

Texture3D::Texture3D(uint32_t width, uint32_t height, uint32_t depth, Texture::Type type, Texture::Layout layout):
    m_Width(width), m_Height(height), m_Depth(depth),
    m_MipCount(0), m_Type(type),
    m_Layout(layout)
{
    GLCall(glGenTextures(1, &m_Texture))

    Bind(*this);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, m_MipCount);

    GLCall(glTexImage3D(GL_TEXTURE_3D, 0,
        ToGPUTextureType(m_Type, m_Layout), Width(), Height(), Depth(), 0,
        ToGLTextureLayout(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout), nullptr))

    if (m_MipCount > 0)
    {
        glGenerateMipmap(GL_TEXTURE_3D);
    }

    UnBind(*this);
}

Texture3D::Texture3D(const ImageCube& Image, bool UseMips):
    m_Width(Image.Width()), m_Height(Image.Height()), m_Depth(Image.Depth()),
    m_MipCount(UseMips ? miplevels(m_Width, m_Height, m_Depth) : 0), m_Type(Texture::ToTextureType(Image.ComponentType())),
    m_Layout(Texture::ToTextureLayout(Image.ComponentLayout()))
{
    GLCall(glGenTextures(1, &m_Texture))

    Bind(*this);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, m_MipCount);

    GLCall(glTexImage3D(GL_TEXTURE_3D, 0,
        ToGPUTextureType(m_Type, m_Layout), Width(), Height(), Depth(), 0,
        ToGLTextureLayout(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout), Image.Data()))

    if (m_MipCount > 0)
    {
        glGenerateMipmap(GL_TEXTURE_3D);
    }

    UnBind(*this);
}

Texture3D::~Texture3D()
{
    GLCall(glDeleteTextures(1, &m_Texture))
}

void Texture3D::Data(uint32_t width, uint32_t height, uint32_t depth)
{
    Data(width, height, depth, m_Type, m_Layout);
}

void Texture3D::Data(uint32_t width, uint32_t height, uint32_t depth, Texture::Type type, Texture::Layout layout)
{
    m_Type = type;
    m_Layout = layout;
    m_Width = width;
    m_Height = height;
    m_Depth = depth;
    m_MipCount = 0;
    
    Bind(*this);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, m_MipCount);

    GLCall(glTexImage3D(GL_TEXTURE_3D, 0,
        ToGPUTextureType(m_Type, m_Layout), Width(), Height(), Depth(), 0,
        ToGLTextureLayout(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout), nullptr))

    if (m_MipCount > 0)
    {
        glGenerateMipmap(GL_TEXTURE_3D);
    }

    UnBind(*this);
}

void Texture3D::Data(const ImageCube& Image, bool UseMips)
{
    m_Type = Texture::ToTextureType(Image.ComponentType());
    m_Layout = Texture::ToTextureLayout(Image.ComponentLayout());
    m_Width = Image.Width();
    m_Height = Image.Height();
    m_Depth = Image.Depth();
    m_MipCount = UseMips ? miplevels(m_Width, m_Height, m_Depth) : 0;

    Bind(*this);

    // Texture parameters
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, m_MipCount);

    GLCall(glTexImage3D(GL_TEXTURE_3D, 0,
        ToGPUTextureType(m_Type, m_Layout), Width(), Height(), Depth(), 0,
        ToGLTextureLayout(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout), Image.Data()))

    if (m_MipCount > 0)
    {
        glGenerateMipmap(GL_TEXTURE_3D);
    }

    UnBind(*this);
}

void Texture3D::Export(ImageCube& Export)
{
    AssertOrErrorCall(ComponentLayout() == Texture::ToTextureLayout(Export.ComponentLayout()), return;, "Output image Format doesn't match GPU image Format");
    AssertOrErrorCall(ComponentType() == Texture::ToTextureType(Export.ComponentType()), return;, "Output image Type doesn't match GPU image Type")
    AssertOrErrorCallF(Width() == Export.Width() && Height() == Export.Height() && Depth() == Export.Depth(), return;, "Export texture resolution missmatch. GPU: %ux%ux%u Export image: %ux%ux%u", Width(), Height(), Depth(), Export.Width(), Export.Height(), Export.Depth())

    Bind(*this);
    GLCall(glGetnTexImage(GL_TEXTURE_3D , 0,
        ToGLTextureLayout(m_Type, m_Layout), ToGLTextureType(m_Type, m_Layout),
        static_cast<GLsizei>(Export.DataSize()), Export.Data()));
    UnBind(*this);
}

void Bind(const Texture3D& texture)
{
    GLCall(glBindTexture(GL_TEXTURE_3D, texture.Handle()))
}

void UnBind(const Texture3D& texture)
{
    GLCall(glBindTexture(GL_TEXTURE_3D, 0))
}
