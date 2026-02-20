#include "FrameBuffers.h"

#include "Shared/Assertion.h"

FrameBuffer::FrameBuffer(const ExternalAttachment& Attachments) :
    FrameBuffer(std::span<const ExternalAttachment>(&Attachments, (&Attachments)+1))
{
}

FrameBuffer::FrameBuffer(std::span<const ExternalAttachment> Attachments):
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
        AssertOrErrorCallF(m_Type == attachment.Type, return,
            "Failed to create framebuffer. Attachment type missmaitch.")

        switch (attachment.Type)
        {
        case Frame2D:
            GLCall(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + AttachmentIndex, GL_TEXTURE_2D, attachment.Handle, 0))
            break;
        case Frame3D:
            // TODO fix
            GLCall(glFramebufferTexture3D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + AttachmentIndex, GL_TEXTURE_3D, attachment.Handle, 0, 0))
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

void Bind(const FrameBuffer& FrameBuffer)
{
    glViewport(0, 0, FrameBuffer.Width(), FrameBuffer.Height());
    GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FrameBuffer.Handle()))
}

void UnBind(const FrameBuffer& FrameBuffer)
{
    GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0))
}
