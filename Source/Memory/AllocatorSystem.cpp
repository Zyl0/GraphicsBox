#include "Memory/Allocators.h"

#include "Shared/Assertion.h"

// Disabled for now
// Allocators for the realloc function are expected not to release memory from the previous pointer of a realloc call
// Therefore std realloc cannot really be used
//#define ALLOW_SYS_MEM_MOVES

void* SystemAllocator::Allocate(size_t Size) const
{
    if (m_Reporter != nullptr) m_Reporter->ReportIncrease(Memory::Reporter::RT_Physical, Size);
    if (m_Reporter != nullptr) m_Reporter->ReportIncrease(Memory::Reporter::RT_Used, Size);
    return malloc(Size);
}

void* SystemAllocator::AllocateAligned(size_t Size, size_t Alignment) const
{
    Size = Memory::AlignedSize(0, Size, Alignment);
        
    if (m_Reporter != nullptr) m_Reporter->ReportIncrease(Memory::Reporter::RT_Physical, Size);
    if (m_Reporter != nullptr) m_Reporter->ReportIncrease(Memory::Reporter::RT_Used, Size);
        
    void* ptr;
    PLATFORM_ALIGNED_MALLOC(void, ptr, Alignment, Size)
    return ptr;
}

void* SystemAllocator::Reallocate(void* OldPtr, size_t OldSize, size_t NewSize) const
{
    if (OldPtr == nullptr || OldSize == 0)
    {
        return Allocate(NewSize);
    }
        
    if (OldSize == NewSize) return OldPtr;
    
#ifdef ALLOW_SYS_MEM_MOVES
    if (m_Reporter != nullptr && NewSize > OldSize)
    {
        m_Reporter->ReportIncrease(Memory::Reporter::RT_Physical, NewSize - OldSize);
        m_Reporter->ReportIncrease(Memory::Reporter::RT_Used, NewSize - OldSize);
    }
    if (m_Reporter != nullptr && NewSize < OldSize)
    {
        m_Reporter->ReportDecrease(Memory::Reporter::RT_Used, OldSize - NewSize);
        m_Reporter->ReportDecrease(Memory::Reporter::RT_Physical, OldSize - NewSize);
    }
    
    void* NewPtr = realloc(OldPtr, NewSize);
    AssertOrError(NewPtr != nullptr, "Reallocation failed")

    return NewPtr;
#else // ALLOW_SYS_MEM_MOVES
    return Allocate(NewSize);
#endif // !ALLOW_SYS_MEM_MOVES
}

void* SystemAllocator::ReallocateAligned(void* OldPtr, size_t OldSize, size_t NewSize, size_t Alignment) const
{
    if (OldPtr == nullptr || OldSize == 0)
    {
        return AllocateAligned(NewSize, Alignment);
    }

    if (OldSize == NewSize) return OldPtr;
    
#ifdef ALLOW_SYS_MEM_MOVES
    if (m_Reporter != nullptr && NewSize > OldSize)
    {
        m_Reporter->ReportIncrease(Memory::Reporter::RT_Physical, NewSize - OldSize);
        m_Reporter->ReportIncrease(Memory::Reporter::RT_Used, NewSize - OldSize);
    }
    if (m_Reporter != nullptr && NewSize < OldSize)
    {
        m_Reporter->ReportDecrease(Memory::Reporter::RT_Used, OldSize - NewSize);
        m_Reporter->ReportDecrease(Memory::Reporter::RT_Physical, OldSize - NewSize);
    }
    
    void* NewPtr = PLATFORM_ALIGNED_REALLOC(OldPtr, Alignment, NewSize)
    AssertOrError(NewPtr != nullptr, "Reallocation failed")
        
    return NewPtr;
#else // ALLOW_SYS_MEM_MOVES
    return AllocateAligned(NewSize, Alignment);
#endif // !ALLOW_SYS_MEM_MOVES
}

void SystemAllocator::Deallocate(void* ptr, size_t Size) const
{
    if (ptr == nullptr) return;
    
    if (m_Reporter != nullptr) m_Reporter->ReportDecrease(Memory::Reporter::RT_Used, Size);
    if (m_Reporter != nullptr) m_Reporter->ReportDecrease(Memory::Reporter::RT_Physical, Size);
    std::free(ptr);
}

void SystemAllocator::DeallocateAligned(void* ptr, size_t Size, size_t Alignment) const
{
    if (ptr == nullptr) return;
        
    Size = Memory::AlignedSize(0, Size, Alignment);
        
    if (m_Reporter != nullptr) m_Reporter->ReportDecrease(Memory::Reporter::RT_Used, Size);
    if (m_Reporter != nullptr) m_Reporter->ReportDecrease(Memory::Reporter::RT_Physical, Size);
    PLATFORM_ALIGNED_FREE(ptr)
}