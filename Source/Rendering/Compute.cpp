#include "Compute.h"

void UnbindTextureStorage(const Pipeline& Program, uint32_t BufferBindingSlot)
{
    GLCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BufferBindingSlot, 0))
}

void BindTextureStorage(const Pipeline& Program, uint8_t TextureBindingSlot, const Texture2D& texture, TextureAccessMode AccessMode, uint8_t MipMapLevel)
{
    switch (AccessMode)
    {
    case TextureAccessMode::TA_ReadOnly:
        GLCall(glBindImageTexture(TextureBindingSlot, texture.Handle(), MipMapLevel, GL_FALSE, 0, GL_READ_ONLY, ToGPUTextureType(texture.ComponentType(), texture.ComponentLayout())))
            break;
    case TextureAccessMode::TA_WriteOnly:
        GLCall(glBindImageTexture(TextureBindingSlot, texture.Handle(), MipMapLevel, GL_FALSE, 0, GL_WRITE_ONLY, ToGPUTextureType(texture.ComponentType(), texture.ComponentLayout())))
            break;
    case TextureAccessMode::TA_ReadWrite:
        GLCall(glBindImageTexture(TextureBindingSlot, texture.Handle(), MipMapLevel, GL_FALSE, 0, GL_READ_WRITE, ToGPUTextureType(texture.ComponentType(), texture.ComponentLayout())))
            break;
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported texture storage access mode")
    }
}

void UnbindTextureStorage(const Pipeline& Program, uint8_t TextureBindingSlot)
{
    glBindImageTexture(TextureBindingSlot, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_NONE);
}

void ComputeDispatch(const Pipeline& Program, uint16_t ThreadBlockCountX, uint16_t ThreadBlockCountY, uint16_t ThreadBlockCountZ)
{
    if (Program.Type() != Pipeline::Compute) return;

    GLCall(glDispatchCompute(ThreadBlockCountX, ThreadBlockCountY, ThreadBlockCountZ));
}

void ComputeBarrier(BarrierType Barrier)
{
    GLenum glBarrier = 0;
    if (Barrier & BarrierType::BT_StorageTextureAccess)
        glBarrier |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
    if (Barrier & BarrierType::BT_TextureUpdate)
        glBarrier |= GL_TEXTURE_UPDATE_BARRIER_BIT;
    if (Barrier & BarrierType::BT_AtomicCounter)
        glBarrier |= GL_ATOMIC_COUNTER_BARRIER_BIT;
    if (Barrier & BarrierType::BT_ShaderStorage)
        glBarrier |= GL_SHADER_STORAGE_BARRIER_BIT;

    glMemoryBarrier(glBarrier);
}
