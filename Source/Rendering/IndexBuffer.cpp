#include "IndexBuffer.h"

IndexBuffer::IndexBuffer()
{
    GLCall(glGenBuffers(1, &m_IndexBuffer))
}

IndexBuffer::~IndexBuffer()
{
    GLCall(glDeleteBuffers(1, &m_IndexBuffer))
}

void IndexBuffer::BufferData(IndexType type, const void* data, unsigned int count)
{
    m_IndexType = type;
    
    Bind(*this);
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER,  count * sizeof(unsigned int), data, GL_STATIC_DRAW))
    UnBind(*this);
}

GLenum ToGLIndexType(IndexBuffer::IndexType Type)
{
    switch (Type)
    {
    case IndexBuffer::Byte:             return GL_BYTE;
    case IndexBuffer::UnsignedByte:     return GL_UNSIGNED_BYTE;
    case IndexBuffer::Short:            return GL_SHORT;
    case IndexBuffer::UnsignedShort:    return GL_UNSIGNED_SHORT;
    case IndexBuffer::Int:              return GL_INT;
    case IndexBuffer::UnsignedInt:      return GL_UNSIGNED_INT;
        
    case IndexBuffer::_Count:
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported Index type")
    }
}

GLsizei ToGLIndexSize(IndexBuffer::IndexType Type)
{
    switch (Type)
    {
    case IndexBuffer::Byte:
    case IndexBuffer::UnsignedByte:     return 1;
    case IndexBuffer::Short:
    case IndexBuffer::UnsignedShort:    return 2;
    case IndexBuffer::Int:
    case IndexBuffer::UnsignedInt:      return 4;
        
    case IndexBuffer::_Count:
    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported Index type")
    }
}

void Bind(const IndexBuffer& VertexBuffer)
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VertexBuffer.Handle()))
}

void UnBind(const IndexBuffer& VertexBuffer)
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0))
}