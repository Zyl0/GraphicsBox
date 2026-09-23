#include "Memory/Allocators.h"

#include "VirtualMemory.h"
#include "Shared/Assertion.h"

LinearAllocator::~LinearAllocator()
{
    if (m_CurrentOffset != 0 && m_Reporter != nullptr) m_Reporter->ReportDecrease(Memory::Reporter::RT_Used, m_CurrentOffset - m_Leakage);
        
    if (m_AllocType == Memory::AllocType::AT_Virtual && m_CommitedPage > 0)
    {
        Memory::Virtual::DeCommit(m_Reporter, m_Memory, 0, m_CommitedPage * Memory::Virtual::GetPageSize());
    }
}

void* LinearAllocator::AllocateAligned(size_t Size, size_t Alignment)
{
    AssertOrWarnCall(Size > 0, return nullptr, "Tried to allocated 0 bytes")

    AssertOrError(IsPowerOfTwo(Alignment), "Alignment must be a power of two")

    // Align 'm_CurrentOffset' forward to the specified alignment
    uintptr_t curr_ptr = reinterpret_cast<uintptr_t>(m_Memory.Address) + m_CurrentOffset;
    uintptr_t offset = Alignment > 1 ? Memory::AlignForward(curr_ptr, Alignment) : curr_ptr;
    m_Leakage += offset - curr_ptr; // avoid counting padding to alignment
    offset -= reinterpret_cast<uintptr_t>(m_Memory.Address); // Change to relative offset

    // Check to see if the backing memory has space left
    // Return nullptr if the arena is out of memory (or handle differently)
    AssertOrErrorCall((offset + Size) <= m_Memory.Size, return nullptr, "Allocator is out of memory to perform this allocation")

    if (m_AllocType == Memory::AllocType::AT_Virtual)
    {
        size_t requiredPageCount = Memory::_Virtual::CalcPageCount(offset + Size); // Last page index + 1
        
        if (requiredPageCount > m_CommitedPage)
        {
            Memory::Virtual::Commit(m_Reporter, m_Memory, m_CommitedPage * Memory::Virtual::GetPageSize(), (requiredPageCount - m_CommitedPage) * Memory::Virtual::GetPageSize());
            m_CommitedPage += (requiredPageCount - m_CommitedPage);
        }
    }
    // else is already allocated

    if (m_Reporter != nullptr) m_Reporter->ReportIncrease(Memory::Reporter::RT_Used, Size);
    
    void *ptr = static_cast<uint8_t*>(m_Memory.Address) + offset;
    
    m_PreviousOffset = offset;
    m_CurrentOffset = offset + Size;

    // Zero new memory by default
    memset(ptr, 0, Size);
    return ptr;
}

void* LinearAllocator::ReallocateAligned(void* OldPtr, size_t OldSize, size_t NewSize, size_t Alignment)
{
    if (OldPtr == nullptr || OldSize == 0)
    {
        return AllocateAligned(NewSize, Alignment);
    }

    if (NewSize == 0)
    {
        DeallocateAligned(OldPtr, NewSize, Alignment);
        return nullptr;
    }

    if (OldSize == NewSize)
    {
        return OldPtr;
    }

    AssertOrError(IsPowerOfTwo(Alignment), "Alignment must be a power of two")
    
    uint8_t* old_mem = static_cast<uint8_t*>(OldPtr);
    uint8_t* memory = static_cast<uint8_t*>(m_Memory.Address);
    
    AssertOrError(m_Memory.Address <= old_mem && (old_mem + OldSize) < (static_cast<uint8_t*>(m_Memory.Address) + m_Memory.Size), "Called Reallocate function over memory that is not part of this allocator")

    // Check if is last allocation
    if (static_cast<uint8_t*>(m_Memory.Address) + m_PreviousOffset == old_mem)
    {
        if (m_AllocType == Memory::AllocType::AT_Virtual)
        {
            size_t offset = old_mem - memory;
            size_t requiredPageCount = Memory::_Virtual::CalcPageCount(offset + NewSize);
    
            if (requiredPageCount > m_CommitedPage)
            {
                Memory::Virtual::Commit(m_Reporter, m_Memory, m_CommitedPage * Memory::Virtual::GetPageSize(), (requiredPageCount - m_CommitedPage) * Memory::Virtual::GetPageSize());
                m_CommitedPage += (requiredPageCount - m_CommitedPage);
            }

            if (NewSize > OldSize)
            {
                if (m_Reporter != nullptr) m_Reporter->ReportIncrease(Memory::Reporter::RT_Used, NewSize - OldSize);
            }
            else // if (NewSize < OldSize)
            {
                if (m_Reporter != nullptr) m_Reporter->ReportDecrease(Memory::Reporter::RT_Used, OldSize - NewSize);
            }
            if (requiredPageCount < m_CommitedPage)
            {
                Memory::Virtual::DeCommit(m_Reporter, m_Memory, (m_CommitedPage - requiredPageCount) * Memory::Virtual::GetPageSize(), (m_CommitedPage - (m_CommitedPage - requiredPageCount)) * Memory::Virtual::GetPageSize());
                m_CommitedPage -= (m_CommitedPage - requiredPageCount);
            }
        }
        else
        {
            if (NewSize > OldSize)
            {
                if (m_Reporter != nullptr) m_Reporter->ReportIncrease(Memory::Reporter::RT_Used, NewSize - OldSize);
            }
            else // if (NewSize < OldSize)
            {
                if (m_Reporter != nullptr) m_Reporter->ReportDecrease(Memory::Reporter::RT_Used, OldSize - NewSize);
            }
        }
        if (NewSize > OldSize)
        {
            m_CurrentOffset += (NewSize - OldSize);
        }
        else // if (NewSize < OldSize)
        {
            m_CurrentOffset -= (OldSize - NewSize);
        }
        return OldPtr;
    }
    else
    {
        return AllocateAligned(NewSize, Alignment);
    }
}

void LinearAllocator::DeallocateAligned(void* ptr, size_t Size, size_t Alignment)
{
    AssertOrWarnCall(ptr != nullptr, return, "Tried to free null memory")
    
    uint8_t* old_mem = static_cast<uint8_t*>(ptr);
    
    AssertOrError(m_Memory.Address <= old_mem && (old_mem + Size) <= (static_cast<uint8_t*>(m_Memory.Address) + m_Memory.Size), "Called Free function over memory that is not part of this allocator")

    //if (m_AllocType == AllocType::Virtual && m_CurrentOffset != m_PreviousOffset && static_cast<uint8_t*>(m_Memory.Address) + m_PreviousOffset == old_mem)
    if (m_CurrentOffset != m_PreviousOffset)
    {
        if (m_Reporter != nullptr) m_Reporter->ReportDecrease(Memory::Reporter::RT_Used, Size);
        
        if (static_cast<uint8_t*>(m_Memory.Address) + m_CurrentOffset == (old_mem + Size))
        {
            if (m_AllocType == Memory::AllocType::AT_Virtual)
            {
                uint8_t* memory = static_cast<uint8_t*>(m_Memory.Address);
                
                size_t offset = old_mem - memory;
                size_t requiredPageCount = Memory::_Virtual::CalcPageCount(offset); // Last page index + 1
                
                if (requiredPageCount < m_CommitedPage)
                {
                    Memory::Virtual::DeCommit(m_Reporter, m_Memory, requiredPageCount * Memory::Virtual::GetPageSize(), (m_CommitedPage - requiredPageCount) * Memory::Virtual::GetPageSize());
                    m_CommitedPage -= (m_CommitedPage - requiredPageCount);
                }
            }
        
            m_CurrentOffset = m_PreviousOffset;
        }
        else
        {
            m_Leakage += Size;
            EngineLoggerWarnF("Linear allocator memory leak of %llu bytes detected", Size);
        }
    }
    else
    {
        m_Leakage += Size;
        EngineLoggerWarnF("Linear allocator memory leak of %llu bytes detected", Size);
    }
}