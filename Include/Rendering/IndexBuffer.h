#pragma once

#include "Shared/Annotations.h"
#include "GLHelper.h"

class IndexBuffer
{
public:
    IndexBuffer();
    ~IndexBuffer();

    INLINE GLuint Handle() const
    {
        return m_IndexBuffer;
    }

    void BufferData(const void* data, unsigned int count);
    
private:
    GLuint m_IndexBuffer;
};

void Bind(const IndexBuffer& VertexBuffer);
void UnBind(const IndexBuffer& VertexBuffer);