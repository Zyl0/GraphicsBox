#include "Memory/Types.h"
#include "Memory/MemAlloc.h"

#include "Shared/Assertion.h"

namespace Memory
{    
    Block Allocate(Reporter* Reporter, size_t size)
    {
        if (Reporter != nullptr) Reporter->ReportIncrease(Memory::Reporter::RT_Physical, size);

        Block block;
        block.Address = malloc(size);
        block.Size = size;
        
        AssertOrError(block.Address != nullptr, "Block allocation failed")
        
        return block;
    }
    
    Block AllocateAligned(Reporter* Reporter, size_t Size, size_t Alignment)
    {
        Size = Memory::AlignedSize(0, Size, Alignment);
        
        if (Reporter != nullptr) Reporter->ReportIncrease(Memory::Reporter::RT_Physical, Size);
        
        void* ptr;
        PLATFORM_ALIGNED_MALLOC(void, ptr, Alignment, Size)
        
        Block block;
        block.Address = ptr;
        block.Size = Size;
        
        AssertOrError(block.Address != nullptr, "Block allocation failed")
        
        return block;
    }
    
    void Reallocate(Reporter* Reporter, Block& block, size_t NewSize)
    {
        if (block.Address == nullptr || block.Size == 0)
        {
            block = Allocate(Reporter, NewSize);
            return;
        }
        
        if (block.Size == NewSize) return;

        if (Reporter != nullptr && NewSize > block.Size)
        {
            Reporter->ReportIncrease(Memory::Reporter::RT_Physical, NewSize - block.Size);
        }
        if (Reporter != nullptr && NewSize < block.Size)
        {
            Reporter->ReportDecrease(Memory::Reporter::RT_Physical, block.Size - NewSize);
        }
        
        block.Address = realloc(block.Address, NewSize);
        block.Size = NewSize;
        
        AssertOrError(block.Address != nullptr, "Block reallocation failed")
    }
    
    void ReallocateAligned(Reporter* Reporter, Block& block, size_t NewSize, size_t Alignment)
    {
        if (block.Address == nullptr || block.Size == 0)
        {
            block = Allocate(Reporter, NewSize);
            return;
        }
        
        if (block.Size == NewSize) return;

        if (Reporter != nullptr && NewSize > block.Size)
        {
            Reporter->ReportIncrease(Memory::Reporter::RT_Physical, NewSize - block.Size);
        }
        if (Reporter != nullptr && NewSize < block.Size)
        {
            Reporter->ReportDecrease(Memory::Reporter::RT_Physical, block.Size - NewSize);
        }
        
        block.Address = PLATFORM_ALIGNED_REALLOC(block.Address, Alignment, NewSize)
        block.Size = NewSize;
        
        AssertOrError(block.Address != nullptr, "Block reallocation failed")
    }
    
    void Deallocate(Reporter* Reporter, Block& block)
    {
        if (block.Address == nullptr) return;
    
        if (Reporter != nullptr) Reporter->ReportDecrease(Memory::Reporter::RT_Physical, block.Size);
        std::free(block.Address);
        
        block.Address = nullptr;
        block.Size = 0;
    }
    
    void DeallocateAligned(Reporter* Reporter, Block& block, size_t Alignment)
    {
        if (block.Address == nullptr) return;
        
        // block.Size = AlignedSize(0, block.Size, Alignment);
        
        if (Reporter != nullptr) Reporter->ReportDecrease(Memory::Reporter::RT_Physical, block.Size);
        PLATFORM_ALIGNED_FREE(block.Address)
        
        block.Address = nullptr;
        block.Size = 0;
    }

    uintptr_t AlignForward(uintptr_t ptr, size_t align)
    {
        if (align == 0) return ptr;

        AssertOrError(IsPowerOfTwo(align), "AlignForward: alignment must be a power of two")

        uintptr_t p = ptr;
        uintptr_t a = (uintptr_t)align;
    
        // Same as (p % a) but faster as 'a' is a power of two
        uintptr_t modulo = p & (a - 1);

        if (modulo != 0)
        {
            // If 'p' address is not aligned, push the address to the
            // next value which is aligned
            p += a - modulo;
        }
    
        return p;
    }

    size_t AlignedSize(uintptr_t ptr, size_t size, size_t Alignment)
    {
        AssertOrError(IsPowerOfTwo(Alignment), "AlignedSize: alignment must be a power of two")

        if (ptr != 0)
        {
            size_t AlignedEnd = Memory::AlignForward(ptr + size, Alignment);
            return AlignedEnd - ptr;
        }
        else
        {
            return Memory::AlignForward(size, Alignment);
        }
    }
}