#pragma once

#include <cstdint>
#include <array>
#include <span>

#include "Math/Vector.h"
#include "GLHelper.h"
#include "Textures.h"

class FrameBuffer
{
public:
    using ClearColor = Math::Vector4f;
    static constexpr size_t kMaxColorAttachments = 4;

    enum FrameType :uint8_t
    {
        Frame2D,
        Frame3D
    };

    struct ExternalAttachment
    {
        uint32_t width; uint32_t height; uint32_t depth;
        GLuint Handle;
        ClearColor ClearColor;
        FrameType Type;

        ExternalAttachment(const Texture2D& Texture, const FrameBuffer::ClearColor& Color):
            width(Texture.Width()), height(Texture.Height()), depth(1),
            Handle(Texture.Handle()),
            ClearColor(Color),
            Type(Frame2D)
        {}

        ExternalAttachment(const Texture3D& Texture, const FrameBuffer::ClearColor& Color):
            width(Texture.Width()), height(Texture.Height()), depth(Texture.Depth()),
            Handle(Texture.Handle()),
            ClearColor(Color),
            Type(Frame3D)
        {}
    };
    
    FrameBuffer(const ExternalAttachment& Attachments);
    FrameBuffer(std::span<const ExternalAttachment> Attachments);
    ~FrameBuffer();

    void Clear();
    void Resize(uint32_t width = 1, uint32_t height = 1, uint32_t depth = 1);
    
    INLINE GLuint Handle() const {return m_FrameBuffer;}
    INLINE uint32_t Width() const {return m_Width;}
    INLINE uint32_t Height() const {return m_Height;}
    INLINE uint32_t Depth() const {return m_Depth;}
    INLINE uint8_t ColorAttachmentCount() const {return m_ColorAttachmentCount;}
    
private:
    uint32_t m_Width, m_Height, m_Depth;
    uint8_t m_ColorAttachmentCount;
    FrameType m_Type;

    std::array<ClearColor, kMaxColorAttachments> m_ClearColors;
    
    GLuint m_FrameBuffer;
};

void Bind(const FrameBuffer& FrameBuffer);
void UnBind(const FrameBuffer& FrameBuffer);