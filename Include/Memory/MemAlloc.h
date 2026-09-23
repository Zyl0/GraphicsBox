#pragma once

#include "Memory/Types.h"
#include "Memory/Reporter.h"

namespace Memory
{    
    Block Allocate(Reporter* Reporter, size_t size);
    
    Block AllocateAligned(Reporter* Reporter, size_t Size, size_t Alignment);
    
    void Reallocate(Reporter* Reporter, Block& block, size_t NewSize);
    
    void ReallocateAligned(Reporter* Reporter, Block& block, size_t NewSize, size_t Alignment);
    
    void Deallocate(Reporter* Reporter, Block& block);
    
    void DeallocateAligned(Reporter* Reporter, Block& block, size_t Alignment);
    
    INLINE Block Allocate(size_t size)
    {
        return Allocate(nullptr, size);
    }
    
    INLINE Block AllocateAligned(size_t Size, size_t Alignment)
    {
        return AllocateAligned(nullptr, Size, Alignment);
    }
    
    INLINE void Reallocate(Block& block, size_t NewSize)
    {
        Reallocate(nullptr, block, NewSize);
    }
    
    INLINE void ReallocateAligned(Block& block, size_t NewSize, size_t Alignment)
    {
        ReallocateAligned(nullptr, block, NewSize, Alignment);
    }
    
    INLINE void Deallocate(Block& block)
    {
        Deallocate(nullptr, block);
    }
    
    INLINE void DeallocateAligned(Block& block, size_t Alignment)
    {
        DeallocateAligned(nullptr, block, Alignment);
    }
}
