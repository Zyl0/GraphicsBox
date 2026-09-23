#pragma once

#include "Types.h"
#include "Reporter.h"

#include "Shared/Annotations.h"

namespace Memory
{
    namespace Virtual
    {
        Block Reserve(size_t Size);
        
        void* Commit(const Block& Memory, size_t Offset, size_t Size);
        
        void DeCommit(const Block& Memory, size_t Offset, size_t Size);
        
        void Release(Block& Memory);

        Block Reserve(Memory::Reporter* Reporter, size_t Size);
        
        void* Commit(Memory::Reporter* Reporter, const Block& Memory, size_t Offset, size_t Size);
        
        void DeCommit(Memory::Reporter* Reporter, const Block& Memory, size_t Offset, size_t Size);
        
        void Release(Memory::Reporter* Reporter, Block& Memory);

        size_t GetPageSize();
    }

    namespace _Virtual
    {
        INLINE size_t GetPageIndex(size_t Offset)
        {
            return Offset / Virtual::GetPageSize();
        }

        INLINE bool IsSubPageMemory(size_t Size)
        {
            return Size < Virtual::GetPageSize();
        }

        INLINE bool ShouldFallbackToPhysicalAlloc(size_t Size)
        {
            return Size < (2 * Virtual::GetPageSize());
        }

        INLINE size_t CalcPageCount(size_t Size)
        {
            size_t rest = Size % Virtual::GetPageSize() > 0 ? 1 : 0;
            return (Size / Virtual::GetPageSize()) + rest;
        }
    }
}
