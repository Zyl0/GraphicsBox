#include "Memory/Allocators.h"

#include <cstring>

#include "VirtualMemory.h"
#include "Shared/Assertion.h"


StackAllocator::~StackAllocator()
{
    if (m_CurrentOffset != 0)
    {
        if (m_PreviousOffset != 0)
        {
            size_t FreedSize = m_CurrentOffset;
            size_t CurrentOffset = m_CurrentOffset;
            size_t PreviousOffset = m_PreviousOffset;
            uintptr_t start = reinterpret_cast<uintptr_t>(m_Memory.Address);

            // Handle moved memory and padding
            while (PreviousOffset != 0)
            {
                uintptr_t currentAddress = start + PreviousOffset;
                Header* currentHeader = reinterpret_cast<Header*>(currentAddress - sizeof(Header));
                if (currentHeader->HasBeenMoved)
                {
                    FreedSize -= (CurrentOffset - PreviousOffset);
                }
                FreedSize -= currentHeader->padding;

                CurrentOffset = PreviousOffset - currentHeader->padding;
                PreviousOffset = currentHeader->PreviousOffset;
            }

            if (m_Reporter != nullptr) m_Reporter->ReportDecrease(Memory::Reporter::RT_Used, FreedSize);
        }
        else
        {
            if (m_Reporter != nullptr) m_Reporter->ReportDecrease(Memory::Reporter::RT_Used, m_CurrentOffset);
        }
    }
        
    if (m_AllocType == Memory::AllocType::AT_Virtual && m_CommitedPage > 0)
    {
        Memory::Virtual::DeCommit(m_Reporter, m_Memory, 0, m_CommitedPage * Memory::Virtual::GetPageSize());
    }
}

void* StackAllocator::AllocateAligned(size_t Size, size_t Alignment)
{
    AssertOrWarnCall(Size > 0, return nullptr, "Tried to allocated 0 bytes")

    AssertOrError(IsPowerOfTwo(Alignment), "Alignment must be a power of two") // TODO return nullptr instead of crash maube

    uintptr_t curr_addr = reinterpret_cast<uintptr_t>(m_Memory.Address) + m_CurrentOffset;
    size_t padding = CalcPaddingWithHeader(curr_addr, (uintptr_t)Alignment);

    // Return nullptr if the stack is out of memory (or handle differently)
    AssertOrErrorCall((m_CurrentOffset + padding + Size) <= m_Memory.Size, return nullptr, "Allocator is out of memory to perform this allocation")

    if (m_AllocType == Memory::AllocType::AT_Virtual)
    {
        size_t requiredPageCount = Memory::_Virtual::CalcPageCount(m_CurrentOffset + padding + Size); // Last page index + 1
        
        if (requiredPageCount > m_CommitedPage)
        {
            Memory::Virtual::Commit(m_Reporter, m_Memory, m_CommitedPage * Memory::Virtual::GetPageSize(), (requiredPageCount - m_CommitedPage) * Memory::Virtual::GetPageSize());
            m_CommitedPage += (requiredPageCount - m_CommitedPage);
        }
    }
    // else is already allocated

    if (m_Reporter != nullptr) m_Reporter->ReportIncrease(Memory::Reporter::RT_Used, Size);

    uintptr_t next_addr = curr_addr + (uintptr_t)padding;
    Header* header = reinterpret_cast<Header*>(next_addr - sizeof(Header));
    header->padding = padding;
    header->PreviousOffset = m_PreviousOffset;
    header->HasBeenMoved = false;

    m_CurrentOffset += padding;
    m_PreviousOffset = m_CurrentOffset;

    m_CurrentOffset += Size;

    return memset(reinterpret_cast<void*>(next_addr), 0, Size);
}

void* StackAllocator::ReallocateAligned(void* OldPtr, size_t OldSize, size_t NewSize, size_t Alignment)
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

    size_t min_size = OldSize < NewSize ? OldSize : NewSize;

    uintptr_t start = reinterpret_cast<uintptr_t>(m_Memory.Address);
    uintptr_t end = start + (uintptr_t)m_Memory.Size;
    uintptr_t curr_addr = reinterpret_cast<uintptr_t>(OldPtr);

    AssertOrError(start <= curr_addr && curr_addr < end, "Called Reallocate function over memory that is not part of this allocator")

    // Allow double frees
    AssertOrWarnCall(curr_addr < (start + m_CurrentOffset), return nullptr, "Double free detected when calling reallocate over memory that was part of this allocator but got previously freed")

    // If last allocated memory, resizes it instead of creating a new allocation
    if (curr_addr + OldSize == (start + m_CurrentOffset))
    {
        if (m_AllocType == Memory::AllocType::AT_Virtual)
        {
            size_t offset = curr_addr - start;
            size_t requiredPageCount = Memory::_Virtual::CalcPageCount(offset + NewSize);
    
            if (requiredPageCount > m_CommitedPage)
            {
                Memory::Virtual::Commit(m_Reporter, m_Memory, m_CommitedPage * Memory::Virtual::GetPageSize(), (requiredPageCount - m_CommitedPage) * Memory::Virtual::GetPageSize());
                m_CommitedPage += (requiredPageCount - m_CommitedPage);
            }

            if (NewSize > OldSize)
            {
                size_t diff = NewSize - OldSize;
                if (m_Reporter != nullptr) m_Reporter->ReportIncrease(Memory::Reporter::RT_Used, diff);
                m_CurrentOffset += diff;
            }
            else // if (NewSize < OldSize)
            {
                size_t diff = OldSize - NewSize;
                if (m_Reporter != nullptr) m_Reporter->ReportDecrease(Memory::Reporter::RT_Used, diff);
                m_CurrentOffset -= diff;
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
                size_t diff = NewSize - OldSize;
                if (m_Reporter != nullptr) m_Reporter->ReportIncrease(Memory::Reporter::RT_Used, diff);
                m_CurrentOffset += diff;
            }
            else // if (NewSize < OldSize)
            {
                size_t diff = OldSize - NewSize;
                if (m_Reporter != nullptr) m_Reporter->ReportDecrease(Memory::Reporter::RT_Used, diff);
                m_CurrentOffset -= diff;
            }
        }

        return OldPtr;
    }

    Header* header = reinterpret_cast<Header*>(curr_addr - sizeof(Header));
    header->HasBeenMoved = true;

    return AllocateAligned(NewSize, Alignment);
}

void StackAllocator::DeallocateAligned(void* ptr, size_t Size, size_t Alignment/* Unused */)
{
    AssertOrWarnCall(ptr != nullptr, return, "Tried to free null memory")

    uintptr_t start = reinterpret_cast<uintptr_t>(m_Memory.Address);
    uintptr_t end = start + (uintptr_t)m_Memory.Size;
    uintptr_t curr_addr = reinterpret_cast<uintptr_t>(ptr);
    AssertOrError(start <= curr_addr && curr_addr < end, "Called Reallocate function over memory that is not part of this allocator")

    // Allow double frees
    AssertOrWarnCall(curr_addr < (start + m_CurrentOffset), return, "Double free detected when calling reallocate over memory that was part of this allocator but got previously freed")
    
    Header* header = reinterpret_cast<Header*>(curr_addr - sizeof(Header));
    AssertOrError(reinterpret_cast<uintptr_t>(m_Memory.Address) + m_CurrentOffset == (curr_addr + Size) || header->HasBeenMoved, "Out of order stack allocator free")
    
    if (m_Reporter != nullptr) m_Reporter->ReportDecrease(Memory::Reporter::RT_Used, Size);

    // Calculate previous offset from the header and its address
    size_t prev_offset = (size_t)((curr_addr - static_cast<uintptr_t>(header->padding)) - start);

    if (m_AllocType == Memory::AllocType::AT_Virtual)
    {
        size_t requiredPageCount = Memory::_Virtual::CalcPageCount(m_PreviousOffset); // Last page index + 1
                
        if (requiredPageCount < m_CommitedPage)
        {
            Memory::Virtual::DeCommit(m_Reporter, m_Memory, requiredPageCount * Memory::Virtual::GetPageSize(), (m_CommitedPage - requiredPageCount) * Memory::Virtual::GetPageSize());
            m_CommitedPage -= (m_CommitedPage - requiredPageCount);
        }
    }

    if (!header->HasBeenMoved)
    {
        m_CurrentOffset = prev_offset;
        m_PreviousOffset = header->PreviousOffset;
    }
    
    if (m_CurrentOffset == 0) return;

    Header* previousHeader = reinterpret_cast<Header*>((reinterpret_cast<uintptr_t>(m_Memory.Address) + m_PreviousOffset) - sizeof(Header));
    if (previousHeader->HasBeenMoved)
    {
        DeallocateAligned(static_cast<uint8_t*>(m_Memory.Address) + m_PreviousOffset, m_CurrentOffset - m_PreviousOffset, 0);
    }
}

size_t StackAllocator::CalcPaddingWithHeader(size_t Ptr, size_t Alignment)
{
#ifdef CONFIGURATION_DEBUG
    AssertOrError(IsPowerOfTwo(Alignment), "Alignment must be a power of two")
#endif // CONFIGURATION_DEBUG

    uintptr_t p = Ptr;
    uintptr_t a = Alignment;
    uintptr_t modulo = p & (a - 1); // (p % a) as it assumes alignment is a power of two

    uintptr_t padding = 0;
    uintptr_t needed_space = sizeof(Header);

    if (modulo != 0)
    {
        // Same logic as 'AlignForward'
        padding = a - modulo;
    }

    if (padding < needed_space)
    {
        needed_space -= padding;

        if ((needed_space & (a-1)) != 0)
        {
            padding += a * (1+(needed_space/a));
        }
        else
        {
            padding += a * (needed_space/a);
        }
    }

    return padding;
}
