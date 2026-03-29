#pragma once

#include <cstdint>

#include "Shared/Annotations.h"
#include "GLHelper.h"

class UniformBuffer
{
public:
    UniformBuffer();
    UniformBuffer(uint32_t Size, const void* data = nullptr);
    ~UniformBuffer();

    UniformBuffer(const UniformBuffer& Other) = delete;

    UniformBuffer(UniformBuffer&& Other) noexcept: 
        m_UBO(Other.m_UBO),
        m_Size(Other.m_Size)
    {
        Other.m_UBO = 0;
        Other.m_Size = 0;
    }

    UniformBuffer& operator=(const UniformBuffer& Other) = delete;

    UniformBuffer& operator=(UniformBuffer&& Other) noexcept
    {
        if (this == &Other)
            return *this;
        m_UBO = Other.m_UBO;
        m_Size = Other.m_Size;
        
        Other.m_UBO = 0;
        Other.m_Size = 0;
        return *this;
    }

    void Data(const void* data, uint32_t size);
    void SubData(const  void* data, uint32_t size, uint32_t offset);
    
    INLINE GLuint Handle() const    { return m_UBO; }
    INLINE uint32_t Size() const    { return m_Size; }
    
private:
    GLuint m_UBO;
    uint32_t m_Size;
};

void Bind(const UniformBuffer& UniformBuffer);
void UnBind(const UniformBuffer& UniformBuffer);