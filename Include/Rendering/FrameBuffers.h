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
        Frame3D,
        FrameCubemapRight,
        FrameCubemapLeft,
        FrameCubemapUp,
        FrameCubemapDown,
        FrameCubemapBack,
        FrameCubemapFront,
        Unset
    };

    struct Attachment
    {
        uint32_t width; uint32_t height; uint32_t depth;
        GLuint Handle;
        ClearColor ClearColor;
        FrameType Type;
        uint8_t TargetMip;

        Attachment(const Texture2D& Texture, const FrameBuffer::ClearColor& Color, uint8_t TargetMip = 0);

        Attachment(const Texture3D& Texture, const FrameBuffer::ClearColor& Color, uint8_t TargetMip = 0);
        
        Attachment(const TextureCube& Texture, TextureCube::Face Face, const FrameBuffer::ClearColor& Color, uint8_t TargetMip = 0);
        
        Attachment(const TextureCubeView& Texture, TextureCube::Face Face, const FrameBuffer::ClearColor& Color, uint8_t TargetMip = 0);
        
        Attachment(uint32_t width, uint32_t height, uint32_t depth, const FrameBuffer::ClearColor& Color, uint8_t TargetMip = 0);
    };
    
    struct RetargetAttachment
    {
        GLuint Handle;
        FrameType Type;
        uint8_t TargetMip;

        RetargetAttachment(const Texture2D& Texture, uint8_t TargetMip = 0);

        RetargetAttachment(const Texture3D& Texture, uint8_t TargetMip = 0);
        
        RetargetAttachment(const TextureCube& Texture, TextureCube::Face Face, uint8_t TargetMip = 0);
        
        RetargetAttachment(const TextureCubeView& Texture, TextureCube::Face Face, uint8_t TargetMip = 0);
    };
    
    FrameBuffer(const Attachment& Attachments);
    FrameBuffer(std::span<const Attachment> Attachments);
    ~FrameBuffer();

    void Clear();
    void Resize(uint32_t width = 1, uint32_t height = 1, uint32_t depth = 1);
    
    void Retarget(const RetargetAttachment& Attachment);
    void Retarget(std::span<const RetargetAttachment> Attachments);
    
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