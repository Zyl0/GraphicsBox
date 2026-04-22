#pragma once

#include "Pipelines.h"
#include "StorageBuffer.h"
#include "Textures.h"

INLINE void BindBufferStorage(const Pipeline& Program, uint32_t BufferBindingSlot, const StorageBuffer& StorageBuffer) {Bind(StorageBuffer, BufferBindingSlot);}

INLINE void UnbindTextureStorage(const Pipeline& Program, uint32_t BufferBindingSlot);

enum TextureAccessMode : uint8_t
{
    TA_ReadOnly,
    TA_WriteOnly,
    TA_ReadWrite
};

void BindTextureStorage(const Pipeline& Program, uint8_t TextureBindingSlot, const Texture2D& texture, TextureAccessMode AccessMode, uint8_t MipMapLevel = 0);

void UnbindTextureStorage(const Pipeline& Program, uint8_t TextureBindingSlot);

void ComputeDispatch(const Pipeline& Program, uint16_t ThreadBlockCountX = 1, uint16_t ThreadBlockCountY = 1, uint16_t ThreadBlockCountZ = 1);

enum BarrierType : uint8_t
{
    // todo include more from there https://registry.khronos.org/OpenGL-Refpages/gl4/html/glMemoryBarrier.xhtml

    BT_None,

    BT_StorageTextureAccess = 1,
    BT_TextureUpdate = 2,
    BT_AtomicCounter = 4,
    BT_ShaderStorage = 8
};

INLINE enum BarrierType operator | (enum BarrierType A, enum BarrierType B) { return static_cast<BarrierType>(static_cast<uint8_t>(A) | static_cast<uint8_t>(B)); }
INLINE enum BarrierType operator & (enum BarrierType A, enum BarrierType B) { return static_cast<BarrierType>(static_cast<uint8_t>(A) & static_cast<uint8_t>(B)); }

void ComputeBarrier(BarrierType Barrier);