#include "Memory/Allocators.h"

#include <limits>

#include "VirtualMemory.h"
#include "Shared/Assertion.h"

_PoolAllocator::~_PoolAllocator()
{
    size_t CurrentOffset = m_AllocatedChunkCount * m_ChunkSize;
    if (CurrentOffset == 0) return;

    size_t FreedSize = CurrentOffset - m_Leakage;
    FreeNode* node = m_FreeList;

    // Handle moved memory
    while (node != nullptr)
    {
        FreedSize -= node->m_ChunkCount * m_ChunkSize;
        node = node->m_Next;
    }
        
    if (m_Reporter != nullptr) m_Reporter->ReportDecrease(Memory::Reporter::RT_Used, FreedSize);
        
    if (m_AllocType == Memory::AllocType::AT_Virtual && m_CommitedPage > 0)
    {
        Memory::Virtual::DeCommit(m_Reporter, m_Memory, 0, m_CommitedPage * Memory::Virtual::GetPageSize());
    }
}

void* _PoolAllocator::AllocateAligned(size_t Size, size_t Alignment)
{
    AssertOrWarnCall(Size > 0, return nullptr, "Tried to allocated 0 bytes")
    AssertOrError(Size >= sizeof(_PoolAllocator::FreeNode), "Pool allocators requires to allocate at least 8 bytes")
    AssertOrWarnCallF(Size > (m_ChunkSize / 2), , "Pool allocation over less (%llu bytes) than half of it's chunk size (%llu bytes)",
        Size, m_ChunkSize)

    AssertOrError(IsPowerOfTwo(Alignment), "Alignment must be a power of two")

    // if (Size <= m_ChunkSize)
    {
        void* ptr = TryPopFreeNode(Size, Alignment);
        if (ptr != nullptr) return ptr;
    }

    size_t toAllocOffset = m_ChunkSize * m_AllocatedChunkCount;

    uintptr_t start = reinterpret_cast<uintptr_t>(m_Memory.Address);
    uintptr_t ptr = start + toAllocOffset;

    uintptr_t alignedPtr = Memory::AlignForward(ptr, Alignment);
    size_t toAllocSubOffset = alignedPtr - ptr;

    size_t RequiredSize = Size + toAllocSubOffset;
    size_t toAllocChunkCount = RequiredSize / m_ChunkSize + (RequiredSize % m_ChunkSize ? 1 : 0);

    AssertOrErrorCall((m_AllocatedChunkCount + toAllocChunkCount) * m_ChunkSize <= m_Memory.Size, return nullptr, "Pool allocator has no free memory")

    m_AllocatedChunkCount += toAllocChunkCount;
    size_t NewLeakage =  (toAllocChunkCount * m_ChunkSize) - Size;
    AssertOrError(m_Leakage < (std::numeric_limits<size_t>::max() - NewLeakage), "Leakage overflow")
    m_Leakage += NewLeakage;

    if (m_AllocType == Memory::AllocType::AT_Virtual)
    {
        size_t requiredPageCount = Memory::_Virtual::CalcPageCount(toAllocOffset + (toAllocChunkCount * m_ChunkSize)); // Last page index + 1
        
        if (requiredPageCount > m_CommitedPage)
        {
            Memory::Virtual::Commit(m_Reporter, m_Memory, m_CommitedPage * Memory::Virtual::GetPageSize(), (requiredPageCount - m_CommitedPage) * Memory::Virtual::GetPageSize());
            m_CommitedPage += (requiredPageCount - m_CommitedPage);
        }
    }
    // else is already allocated

    if (m_Reporter != nullptr) m_Reporter->ReportIncrease(Memory::Reporter::RT_Used, Size);

    return memset(reinterpret_cast<void*>(alignedPtr), 0, toAllocChunkCount * m_ChunkSize);
}

void* _PoolAllocator::ReallocateAligned(void* OldPtr, size_t OldSize, size_t NewSize, size_t Alignment)
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

    void* start = m_Memory.Address;
    void* end = static_cast<uint8_t*>(m_Memory.Address) + m_Memory.Size;
    AssertOrError(start <= OldPtr && OldPtr < end, "Called Reallocate function over memory that is not part of this allocator")

    // De align memory        
    uintptr_t at = reinterpret_cast<uintptr_t>(OldPtr) - reinterpret_cast<uintptr_t>(m_Memory.Address);
    size_t chunkIndex = at / m_ChunkSize;
    size_t offset = at % m_ChunkSize;
    uintptr_t atDeAligned = at - offset;
    size_t AllocSize = OldSize + offset;
    size_t NewAllocSize = NewSize + offset;

    size_t ChunkCount = AllocSize / m_ChunkSize + (AllocSize % m_ChunkSize ? 1 : 0);
    size_t NewChunkCount = NewAllocSize / m_ChunkSize + (NewAllocSize % m_ChunkSize ? 1 : 0);

    size_t oldLeakage = (ChunkCount * m_ChunkSize) - OldSize; // oldLeakage
    size_t NewLeakage =  (NewChunkCount * m_ChunkSize) - NewSize;
    AssertOrError(m_Leakage < (std::numeric_limits<size_t>::max() - NewLeakage), "Leakage overflow")
    m_Leakage += NewLeakage; // newLeakage

    if (ChunkCount == NewChunkCount)
    {
        AssertOrError(m_Leakage >= oldLeakage, "Leakage overflow")
        m_Leakage -= oldLeakage;

        if (NewSize < OldSize)
        {
            if (m_Reporter != nullptr) m_Reporter->ReportDecrease(Memory::Reporter::RT_Used, OldSize - NewSize);
        }
        else // NewSize > OldSize
        {
            if (m_Reporter != nullptr) m_Reporter->ReportIncrease(Memory::Reporter::RT_Used, NewSize - OldSize);
        }
        
        return OldPtr;
    }

    if (NewChunkCount < ChunkCount)
    {
        AssertOrError(m_Leakage >= oldLeakage, "Leakage overflow")
        m_Leakage -= oldLeakage;
        
        if (m_Reporter != nullptr) m_Reporter->ReportDecrease(Memory::Reporter::RT_Used, OldSize - NewSize);
        size_t DeltaChunk = ChunkCount - NewChunkCount;

        if (chunkIndex + ChunkCount == m_AllocatedChunkCount)
        {
            if (m_AllocType == Memory::AllocType::AT_Virtual)
            {
                size_t requiredPageCount = Memory::_Virtual::CalcPageCount((m_AllocatedChunkCount - DeltaChunk) * m_ChunkSize); // Last page index + 1
        
                if (requiredPageCount < m_CommitedPage)
                {
                    Memory::Virtual::DeCommit(m_Reporter, m_Memory, requiredPageCount * Memory::Virtual::GetPageSize(), (m_CommitedPage - requiredPageCount) * Memory::Virtual::GetPageSize());
                    m_CommitedPage -= (m_CommitedPage - requiredPageCount);
                }
            }
            
            m_AllocatedChunkCount -= DeltaChunk;
        }
        else
        {
            InsertFreeNode(atDeAligned + (NewChunkCount * m_ChunkSize), DeltaChunk);
        }            
        
        return OldPtr;
    }

    // if (NewChunkCount > ChunkCount)

    // check if last allocated memory
    if (chunkIndex + ChunkCount == m_AllocatedChunkCount)
    {
        AssertOrError(m_Leakage >= oldLeakage, "Leakage overflow")
        m_Leakage -= oldLeakage;
        
        size_t DeltaChunk = NewChunkCount - ChunkCount;

        AssertOrError((m_AllocatedChunkCount + DeltaChunk) * m_ChunkSize <= m_Memory.Size, "Pool allocator has no free memory")

        m_AllocatedChunkCount += DeltaChunk;

        if (m_AllocType == Memory::AllocType::AT_Virtual)
        {
            size_t requiredPageCount = Memory::_Virtual::CalcPageCount((m_AllocatedChunkCount + DeltaChunk) * m_ChunkSize); // Last page index + 1
        
            if (requiredPageCount > m_CommitedPage)
            {
                Memory::Virtual::Commit(m_Reporter, m_Memory, m_CommitedPage * Memory::Virtual::GetPageSize(), (requiredPageCount - m_CommitedPage) * Memory::Virtual::GetPageSize());
                m_CommitedPage += (requiredPageCount - m_CommitedPage);
            }
        }

        if (m_Reporter != nullptr) m_Reporter->ReportIncrease(Memory::Reporter::RT_Used, NewSize - OldSize);

        return OldPtr;
    }

    return AllocateAligned(NewSize, Alignment);
}

void _PoolAllocator::DeallocateAligned(void* ptr, size_t Size, size_t Alignment)
{
    AssertOrWarnCall(ptr != nullptr, return, "Tried to free null memory")

    void* start = m_Memory.Address;
    void* end = static_cast<uint8_t*>(m_Memory.Address) + m_Memory.Size;

    AssertOrError(start <= ptr && ptr < end, "Called Reallocate function over memory that is not part of this allocator")

    if (m_Reporter != nullptr) m_Reporter->ReportDecrease(Memory::Reporter::RT_Used, Size);

    // TODO free memory at the end of the allocator when possible

    // De align memory
    uintptr_t at = reinterpret_cast<uintptr_t>(ptr) - reinterpret_cast<uintptr_t>(m_Memory.Address);
    size_t offset = at % m_ChunkSize;
    size_t chunkIndex = at / m_ChunkSize;
    Size = Size + offset;
    size_t chunkCount = (Size / m_ChunkSize) + (Size % m_ChunkSize > 0 ? 1 : 0);

    size_t oldLeakage = (chunkCount * m_ChunkSize) - (Size - offset);
    AssertOrError(m_Leakage >= oldLeakage, "Leakage overflow")
    m_Leakage -= oldLeakage;

    InsertFreeNode(at - offset, chunkCount);
}

void _PoolAllocator::InsertFreeNode(size_t offset, size_t chunkCount)
{
    void* ptr = static_cast<uint8_t*>(m_Memory.Address) + offset;
    void* start = m_Memory.Address;
    void* end = static_cast<uint8_t*>(m_Memory.Address) + m_Memory.Size;
    
    FreeNode* asNode = static_cast<FreeNode*>(ptr);
    asNode->m_Next = nullptr;
    asNode->m_ChunkCount = chunkCount;

    FreeNode* list = m_FreeList;
    if (list == nullptr)
    {
        m_FreeList = asNode;
        return;
    }

    uintptr_t to_free_reg_start = reinterpret_cast<uintptr_t>(ptr);
    uintptr_t to_free_reg_end = to_free_reg_start + chunkCount * m_ChunkSize;

    if (reinterpret_cast<uintptr_t>(list) == to_free_reg_end)
    {
        AssertOrError(list->m_Next == nullptr || (start <= list->m_Next && list->m_Next < end), "Out of bound free list next pointer")
        
        m_FreeList = asNode;
        m_FreeList->m_Next = list->m_Next;
        m_FreeList->m_ChunkCount += list->m_ChunkCount;

        return;
    }

    if (reinterpret_cast<uintptr_t>(list) > to_free_reg_end)
    {
        AssertOrError(list == nullptr || start <= list && list < end, "Out of bound free list next pointer")
        
        m_FreeList = asNode;
        m_FreeList->m_Next = list;

        return;
    }

    // Ordered insertion in an non empty list with merging of adjacent free regions
    while (list != nullptr)
    {
        uintptr_t curr_reg_start = reinterpret_cast<uintptr_t>(list);
        uintptr_t curr_reg_end = curr_reg_start + list->m_ChunkCount * m_ChunkSize;

        // merge previous region if adjacent
        if (curr_reg_end == to_free_reg_start)
        {
            list->m_ChunkCount += chunkCount;
            curr_reg_end = curr_reg_start + list->m_ChunkCount * m_ChunkSize;

            if (list->m_Next != nullptr)
            {
                FreeNode* next = list->m_Next;

                uintptr_t next_reg_start = reinterpret_cast<uintptr_t>(next);

                // merge next region if adjacent
                if (next_reg_start == curr_reg_end)
                {
                    AssertOrError(next->m_Next == nullptr || (start <= next->m_Next && next->m_Next < end), "Out of bound free list next pointer")
                    list->m_Next = next->m_Next;
                    list->m_ChunkCount += next->m_ChunkCount;
                }
            }
            return;
        }

        if (list->m_Next != nullptr)
        {
            FreeNode* next = list->m_Next;

            uintptr_t next_reg_start = reinterpret_cast<uintptr_t>(next);

            // merge next region if adjacent
            if (next_reg_start == to_free_reg_end)
            {
                AssertOrError(asNode == nullptr || (start <= asNode && asNode < end), "Out of bound free list next pointer")
                AssertOrError(next->m_Next == nullptr || (start <= next->m_Next && next->m_Next < end), "Out of bound free list next pointer")
                
                list->m_Next = asNode;
                asNode->m_Next = next->m_Next;
                asNode->m_ChunkCount += next->m_ChunkCount;

                return;
            }

            // place region freed if next one is a higher pointer in memory
            if (next_reg_start > to_free_reg_end)
            {
                AssertOrError(asNode == nullptr || (start <= asNode && asNode < end), "Out of bound free list next pointer")
                AssertOrError(next == nullptr || (start <= next && next < end), "Out of bound free list next pointer")
                
                list->m_Next = asNode;
                asNode->m_Next = next;

                return;
            }
        }
        else
        // place region freed as last
        {
            AssertOrError(asNode == nullptr || (start <= asNode && asNode < end), "Out of bound free list next pointer")
            list->m_Next = asNode;

            return;
        }
        
        list = list->m_Next;
    }
}

void* _PoolAllocator::TryPopFreeNode(size_t Size, size_t Alignment)
{
    void* start = m_Memory.Address;
    void* end = static_cast<uint8_t*>(m_Memory.Address) + m_Memory.Size;
    
    FreeNode* node = m_FreeList;
    FreeNode* previousNode = nullptr;

    // Find fitted memory chunk
    while (node != nullptr)
    {
        uintptr_t addr = reinterpret_cast<uintptr_t>(node);
        uintptr_t alignedAddr = Memory::AlignForward(addr, Alignment);

        size_t offset = alignedAddr - addr;

        if ((Size + offset) <= (node->m_ChunkCount * m_ChunkSize)) break;

        previousNode = node;
        node = node->m_Next;
    }

    if (node != nullptr)
    {
        uintptr_t addr = reinterpret_cast<uintptr_t>(node);
        uintptr_t alignedAddr = Memory::AlignForward(addr, Alignment);

        size_t offset = alignedAddr - addr;
        size_t effectiveSize = Size + offset;

        size_t chunkCount = (effectiveSize / m_ChunkSize) + (effectiveSize % m_ChunkSize > 0 ? 1 : 0);
        if (chunkCount == node->m_ChunkCount)
        {
            if (previousNode == nullptr)
            {
                m_FreeList = m_FreeList->m_Next;
            }
            else
            {
                AssertOrError(m_FreeList->m_Next == nullptr || (start <= m_FreeList->m_Next && m_FreeList->m_Next < end), "Out of bound free list next pointer")
                previousNode->m_Next = m_FreeList->m_Next;
            }
        }
        else // chunkCount < node->m_ChunkCount
        {
            AssertOrError(node->m_Next == nullptr || (start <= node->m_Next && node->m_Next < end), "Out of bound free list next pointer")
            
            FreeNode* newNode = reinterpret_cast<FreeNode*>(reinterpret_cast<uint8_t*>(node) + chunkCount * m_ChunkSize);
            newNode->m_ChunkCount = node->m_ChunkCount - chunkCount;
            newNode->m_Next = node->m_Next;

            if (previousNode == nullptr)
            {
                m_FreeList = newNode;
            }
            else
            {
                AssertOrError(newNode == nullptr || (start <= newNode && newNode < end), "Out of bound free list next pointer")
                previousNode->m_Next = newNode;
            }
        }

        size_t NewLeakage =  (chunkCount * m_ChunkSize) - Size;
        AssertOrError(m_Leakage < (std::numeric_limits<size_t>::max() - NewLeakage), "Leakage overflow")
        m_Leakage += NewLeakage;
        if (m_Reporter != nullptr) m_Reporter->ReportIncrease(Memory::Reporter::RT_Used, Size);

        return memset(node, 0, Size);
    }

    return nullptr;
}
