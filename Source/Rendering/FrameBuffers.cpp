#include "FrameBuffers.h"

#include <vector>

#include "Shared/Assertion.h"

FrameBuffer::Attachment::Attachment(const Texture2D& Texture, const FrameBuffer::ClearColor& Color, uint8_t TargetMip):
    width(Texture.Width()), height(Texture.Height()), depth(1),
    Handle(Texture.Handle()),
    ClearColor(Color),
    Type(Frame2D), TargetMip(TargetMip)
{}

FrameBuffer::Attachment::Attachment(const Texture3D& Texture, const FrameBuffer::ClearColor& Color, uint8_t TargetMip):
    width(Texture.Width()), height(Texture.Height()), depth(Texture.Depth()),
    Handle(Texture.Handle()),
    ClearColor(Color),
    Type(Frame3D), TargetMip(TargetMip)
{}

FrameBuffer::Attachment::Attachment(const TextureCube& Texture, TextureCube::Face Face, const FrameBuffer::ClearColor& Color, uint8_t TargetMip):
    width(Texture.Width()), height(Texture.Height()), depth(1),
    Handle(Texture.Handle()),
    ClearColor(Color), TargetMip(TargetMip)
{
    switch (Face)
    {
    case TextureCube::Right:        Type = FrameCubemapRight; break;
    case TextureCube::Left:         Type = FrameCubemapLeft; break;
    case TextureCube::Up:           Type = FrameCubemapUp; break;
    case TextureCube::Down:         Type = FrameCubemapDown; break;
    case TextureCube::Back:         Type = FrameCubemapBack; break;
    case TextureCube::Front:        Type = FrameCubemapFront; break;
        
    case TextureCube::_Count:
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported face type")
    }
}

FrameBuffer::Attachment::Attachment(const TextureCubeView& Texture, TextureCube::Face Face, const FrameBuffer::ClearColor& Color, uint8_t TargetMip):
    width(Texture.Width()), height(Texture.Height()), depth(1),
    Handle(Texture.Handle()),
    ClearColor(Color), TargetMip(TargetMip)
{
    switch (Face)
    {
    case TextureCube::Right:        Type = FrameCubemapRight; break;
    case TextureCube::Left:         Type = FrameCubemapLeft; break;
    case TextureCube::Up:           Type = FrameCubemapUp; break;
    case TextureCube::Down:         Type = FrameCubemapDown; break;
    case TextureCube::Back:         Type = FrameCubemapBack; break;
    case TextureCube::Front:        Type = FrameCubemapFront; break;
        
    case TextureCube::_Count:
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported face type")
    }
}

FrameBuffer::Attachment::Attachment(uint32_t width, uint32_t height, uint32_t depth, const FrameBuffer::ClearColor& Color, uint8_t TargetMip):
    width(width), height(height), depth(depth),
    Handle(0),
    ClearColor(Color),
    Type(Unset), TargetMip(TargetMip)
{
}

FrameBuffer::RetargetAttachment::RetargetAttachment(const Texture2D& Texture, uint8_t TargetMip):
    Handle(Texture.Handle()),
    Type(Frame2D), TargetMip(TargetMip)
{
}

FrameBuffer::RetargetAttachment::RetargetAttachment(const Texture3D& Texture, uint8_t TargetMip):
    Handle(Texture.Handle()),
    Type(Frame3D), TargetMip(TargetMip)
{
}

FrameBuffer::RetargetAttachment::RetargetAttachment(const TextureCube& Texture, TextureCube::Face Face, uint8_t TargetMip):
    Handle(Texture.Handle()), TargetMip(TargetMip)
{
    switch (Face)
    {
    case TextureCube::Right:        Type = FrameCubemapRight; break;
    case TextureCube::Left:         Type = FrameCubemapLeft; break;
    case TextureCube::Up:           Type = FrameCubemapUp; break;
    case TextureCube::Down:         Type = FrameCubemapDown; break;
    case TextureCube::Back:         Type = FrameCubemapBack; break;
    case TextureCube::Front:        Type = FrameCubemapFront; break;
        
    case TextureCube::_Count:
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported face type")
    }
}

FrameBuffer::RetargetAttachment::RetargetAttachment(const TextureCubeView& Texture, TextureCube::Face Face, uint8_t TargetMip):
    Handle(Texture.Handle()), TargetMip(TargetMip)
{
    switch (Face)
    {
    case TextureCube::Right:        Type = FrameCubemapRight; break;
    case TextureCube::Left:         Type = FrameCubemapLeft; break;
    case TextureCube::Up:           Type = FrameCubemapUp; break;
    case TextureCube::Down:         Type = FrameCubemapDown; break;
    case TextureCube::Back:         Type = FrameCubemapBack; break;
    case TextureCube::Front:        Type = FrameCubemapFront; break;
        
    case TextureCube::_Count:
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported face type")
    }
}

FrameBuffer::FrameBuffer(const Attachment& Attachments) :
    FrameBuffer(std::span<const Attachment>(&Attachments, (&Attachments)+1))
{
}

FrameBuffer::FrameBuffer(std::span<const Attachment> Attachments):
    m_Width(0), m_Height(0), m_Depth(0), m_ColorAttachmentCount(0)
{
    GLCall(glGenFramebuffers(1, &m_FrameBuffer))
    Bind(*this);
    
    AssertOrErrorCall(!Attachments.empty(), return, "Failed to create framebuffer. No attachment specified")

    m_Width = Attachments[0].width;
    m_Height = Attachments[0].height;
    m_Depth = Attachments[0].depth;
    m_Type = Attachments[0].Type;

    std::vector<GLenum> AttachmentsIndexes; AttachmentsIndexes.reserve(Attachments.size());
    uint8_t AttachmentIndex = 0;
    for (const auto& attachment : Attachments)
    {
        AssertOrErrorCallF(attachment.width == m_Width && attachment.height == m_Height && attachment.depth == m_Depth, return,
            "Failed to create framebuffer. Attachment size missmaitch. Expected %ux%ux%u, got %ux%ux%u",
            m_Width, m_Height, m_Depth, attachment.width, attachment.height, attachment.depth)
        AssertOrErrorCall(m_Type == attachment.Type, return,
            "Failed to create framebuffer. Attachment type missmaitch.")

        switch (attachment.Type)
        {
        case Frame2D:
            GLCall(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + AttachmentIndex, GL_TEXTURE_2D, attachment.Handle, attachment.TargetMip))
            break;
        case Frame3D:
            // TODO fix
            GLCall(glFramebufferTexture3D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + AttachmentIndex, GL_TEXTURE_3D, attachment.Handle, attachment.TargetMip, 0))
            break;
            
        case FrameCubemapRight:
            GLCall(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + AttachmentIndex, GL_TEXTURE_CUBE_MAP_POSITIVE_X , attachment.Handle, attachment.TargetMip))
            break;
            
        case FrameCubemapLeft:
            GLCall(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + AttachmentIndex, GL_TEXTURE_CUBE_MAP_NEGATIVE_X , attachment.Handle, attachment.TargetMip))
            break;
            
        case FrameCubemapUp:
            GLCall(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + AttachmentIndex, GL_TEXTURE_CUBE_MAP_POSITIVE_Y , attachment.Handle, attachment.TargetMip))
            break;
            
        case FrameCubemapDown:
            GLCall(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + AttachmentIndex, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y , attachment.Handle, attachment.TargetMip))
            break;
            
        case FrameCubemapBack:
            GLCall(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + AttachmentIndex, GL_TEXTURE_CUBE_MAP_POSITIVE_Z , attachment.Handle, attachment.TargetMip))
            break;
            
        case FrameCubemapFront:
            GLCall(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + AttachmentIndex, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z , attachment.Handle, attachment.TargetMip))
            break;
            
        case Unset:
            break;
            
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported texture type")
        }

        AttachmentsIndexes.push_back(GL_COLOR_ATTACHMENT0 + AttachmentIndex);

        AttachmentIndex++;
    }

    m_ColorAttachmentCount = Attachments.size();
    
    GLCall(glDrawBuffers(ColorAttachmentCount(), AttachmentsIndexes.data());)
    // Clear();

    UnBind(*this);
}

FrameBuffer::~FrameBuffer()
{
    GLCall(glDeleteBuffers(1, &m_FrameBuffer))
}

void FrameBuffer::Clear()
{
    for(unsigned int i = 0; i < m_ColorAttachmentCount; i++)
    {
        glClearBufferfv(GL_COLOR, i, &(m_ClearColors[i]).x);
    }
}

void FrameBuffer::Resize(uint32_t width, uint32_t height, uint32_t depth)
{
    m_Width = width;
    m_Height = height;
    m_Depth = depth;
}

void FrameBuffer::Retarget(const RetargetAttachment& Attachment)
{
    Retarget(std::span(&Attachment, (&Attachment)+1));
}

void FrameBuffer::Retarget(std::span<const RetargetAttachment> Attachments)
{
    Bind(*this);
    
    AssertOrErrorCall(!Attachments.empty(), return, "Failed to create framebuffer. No attachment specified")

    std::vector<GLenum> AttachmentsIndexes; AttachmentsIndexes.reserve(Attachments.size());
    uint8_t AttachmentIndex = 0;
    for (const auto& attachment : Attachments)
    {
        switch (attachment.Type)
        {
        case Frame2D:
            GLCall(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + AttachmentIndex, GL_TEXTURE_2D, attachment.Handle, attachment.TargetMip))
            break;
        case Frame3D:
            // TODO fix
            GLCall(glFramebufferTexture3D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + AttachmentIndex, GL_TEXTURE_3D, attachment.Handle, attachment.TargetMip, 0))
            break;
            
        case FrameCubemapRight:
            GLCall(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + AttachmentIndex, GL_TEXTURE_CUBE_MAP_POSITIVE_X , attachment.Handle, attachment.TargetMip))
            break;
            
        case FrameCubemapLeft:
            GLCall(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + AttachmentIndex, GL_TEXTURE_CUBE_MAP_NEGATIVE_X , attachment.Handle, attachment.TargetMip))
            break;
            
        case FrameCubemapUp:
            GLCall(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + AttachmentIndex, GL_TEXTURE_CUBE_MAP_POSITIVE_Y , attachment.Handle, attachment.TargetMip))
            break;
            
        case FrameCubemapDown:
            GLCall(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + AttachmentIndex, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y , attachment.Handle, attachment.TargetMip))
            break;
            
        case FrameCubemapBack:
            GLCall(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + AttachmentIndex, GL_TEXTURE_CUBE_MAP_POSITIVE_Z , attachment.Handle, attachment.TargetMip))
            break;
            
        case FrameCubemapFront:
            GLCall(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + AttachmentIndex, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z , attachment.Handle, attachment.TargetMip))
            
            break;
            
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported texture type")
        }

        AttachmentsIndexes.push_back(GL_COLOR_ATTACHMENT0 + AttachmentIndex);

        AttachmentIndex++;
    }

    m_ColorAttachmentCount = Attachments.size();
    
    GLCall(glDrawBuffers(ColorAttachmentCount(), AttachmentsIndexes.data());)
    // Clear();

    UnBind(*this);
}

void Bind(const FrameBuffer& FrameBuffer)
{
    glViewport(0, 0, FrameBuffer.Width(), FrameBuffer.Height());
    GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FrameBuffer.Handle()))
}

void UnBind(const FrameBuffer& FrameBuffer)
{
    GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0))
}
