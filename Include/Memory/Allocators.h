#pragma once
#include <limits>

#include "Reporter.h"
#include "Types.h"
#include "Shared/Annotations.h"

struct SystemAllocator
{
    INLINE constexpr size_t AllocationMin() const {return 1;}
    INLINE constexpr size_t AllocationMax() const {return std::numeric_limits<size_t>::max();}
    
    SystemAllocator(Memory::Reporter* Reporter = nullptr) : m_Reporter(Reporter) {}

    SystemAllocator(const SystemAllocator& Other) : m_Reporter(Other.m_Reporter) {}

    SystemAllocator(SystemAllocator&& Other) noexcept : m_Reporter(Other.m_Reporter)
    {
    }

    SystemAllocator& operator=(const SystemAllocator& Other)
    {
        if (this == &Other)
            return *this;
        m_Reporter = Other.m_Reporter;
        return *this;
    }

    SystemAllocator& operator=(SystemAllocator&& Other) noexcept
    {
        if (this == &Other)
            return *this;
        m_Reporter = Other.m_Reporter;
        return *this;
    }

    ~SystemAllocator() = default;

    void* Allocate(size_t Size) const;

    void* AllocateAligned(size_t Size, size_t Alignment) const;

    // Reallocate a given memory,
    // if the new memory needs to be moved, it is up to the caller to perform the required moves and free the previous memory.
    void* Reallocate(void* OldPtr, size_t OldSize, size_t NewSize) const;

    // Reallocate a given memory,
    // if the new memory needs to be moved, it is up to the caller to perform the required moves and free the previous memory.
    void* ReallocateAligned(void* OldPtr, size_t OldSize, size_t NewSize, size_t Alignment) const;

    void Deallocate(void* ptr, size_t Size) const;

    void DeallocateAligned(void* ptr, size_t Size, size_t Alignment) const;

    //Block ReserveRegion(size_t Size) = delete;
    //void ReleaseRegion(Block& block) = delete;

private:        
    Memory::Reporter* m_Reporter;
};

struct LinearAllocator
{
    INLINE constexpr size_t AllocationMin() const {return 1;}
    INLINE size_t AllocationMax() const {return m_Memory.Size;}
    
    LinearAllocator(Memory::Block memory, Memory::AllocType type, Memory::Reporter* Reporter = nullptr):
        m_Reporter(Reporter),
        m_Memory(memory),
        m_PreviousOffset(memory.Size),
        m_AllocType(type)
    {}

    ~LinearAllocator();

    INLINE void* Allocate(size_t Size) 
    {
        return AllocateAligned(Size, Memory::kDefaultAlignment);
    }

    void* AllocateAligned(size_t Size, size_t Alignment);

    // Reallocate a given memory,
    // if the new memory needs to be moved, it is up to the caller to perform the required moves and free the previous memory.
    INLINE void* Reallocate(void* OldPtr, size_t OldSize, size_t NewSize) 
    {
        return ReallocateAligned(OldPtr, OldSize, NewSize, Memory::kDefaultAlignment);
    }

    // Reallocate a given memory,
    // if the new memory needs to be moved, it is up to the caller to perform the required moves and free the previous memory.
    void* ReallocateAligned(void* OldPtr, size_t OldSize, size_t NewSize, size_t Alignment);

    INLINE void Deallocate(void* ptr, size_t Size) 
    {
        DeallocateAligned(ptr, Size, Memory::kDefaultAlignment);
    }

    void DeallocateAligned(void* ptr, size_t Size, size_t Alignment);
    
    // Block ReserveRegion(size_t Size);

    // void ReleaseRegion(Block& block);

private:
    Memory::Reporter* m_Reporter;
    Memory::Block m_Memory;
    size_t m_CommitedPage = 0;
    size_t m_CurrentOffset = 0;
    size_t m_PreviousOffset = 0;
    size_t m_Leakage = 0;
    const Memory::AllocType m_AllocType;
};

struct StackAllocator
{
    INLINE constexpr size_t AllocationMin() const {return 1;}
    INLINE size_t AllocationMax() const {return m_Memory.Size - sizeof(Header);}
    
    StackAllocator(Memory::Block memory, Memory::AllocType type, Memory::Reporter* Reporter = nullptr):
        m_Reporter(Reporter),
        m_Memory(memory),
        m_AllocType(type)
    {}

    ~StackAllocator();

private:
    struct Header
    {
        size_t PreviousOffset;
        uint32_t padding;
        bool HasBeenMoved = false;
    };
public:

    INLINE void* Allocate(size_t Size) 
    {
        return AllocateAligned(Size, Memory::kDefaultAlignment);
    }
    
    void* AllocateAligned(size_t Size, size_t Alignment);

    // Reallocate a given memory,
    // if the new memory needs to be moved, it is up to the caller to perform the required moves and free the previous memory.
    INLINE void* Reallocate(void* OldPtr, size_t OldSize, size_t NewSize) 
    {
        return ReallocateAligned(OldPtr, OldSize, NewSize, Memory::kDefaultAlignment);
    }

    // Reallocate a given memory,
    // if the new memory needs to be moved, it is up to the caller to perform the required moves and free the previous memory.
    void* ReallocateAligned(void* OldPtr, size_t OldSize, size_t NewSize, size_t Alignment);

    INLINE void Deallocate(void* ptr, size_t Size) 
    {
        DeallocateAligned(ptr, Size, Memory::kDefaultAlignment);
    }

    void DeallocateAligned(void* ptr, size_t Size, size_t Alignment);

    // Can only be called for virtual allocators
    // Region sizes are rounded to pages
    // this just outsource the responsability of the memory page(s) to another sub allocator
    // it does not commit it
    // Block ReserveRegion(size_t Size);

    // Not sure if I keep this one but I know that the linear allocator cannot deal well with it
    // And maybe not even the stack one as this reserve release may not happen in a LIFO way
    // void ReleaseRegion(Block& block);


private:
    static size_t CalcPaddingWithHeader(size_t Ptr, size_t Alignment);
    
    Memory::Reporter* m_Reporter;
    Memory::Block m_Memory;
    size_t m_CommitedPage = 0;
    size_t m_CurrentOffset = 0;
    size_t m_PreviousOffset = 0;
    const Memory::AllocType m_AllocType;
};

struct _PoolAllocator
{        
protected:
    struct FreeNode
    {
        FreeNode* m_Next;
        size_t m_ChunkCount;
    };
    
    INLINE constexpr size_t AllocationMin() const {return sizeof(FreeNode);}
    INLINE size_t AllocationMax() const {return m_Memory.Size;}
    
public:
    _PoolAllocator(Memory::Block memory, size_t ChunkSize, Memory::AllocType type, Memory::Reporter* Reporter = nullptr):
        m_Reporter(Reporter),
        m_Memory(memory),
        m_ChunkSize(ChunkSize),
        m_AllocType(type)
    {}
    
    ~_PoolAllocator();

    INLINE void* Allocate(size_t Size) 
    {
        return AllocateAligned(Size, Memory::kDefaultAlignment);
    }
   
    void* AllocateAligned(size_t Size, size_t Alignment);

    // Reallocate a given memory,
    // if the new memory needs to be moved, it is up to the caller to perform the required moves and free the previous memory.
    INLINE void* Reallocate(void* OldPtr, size_t OldSize, size_t NewSize) 
    {
        return ReallocateAligned(OldPtr, OldSize, NewSize, Memory::kDefaultAlignment);
    }

    // Reallocate a given memory,
    // if the new memory needs to be moved, it is up to the caller to perform the required moves and free the previous memory.
    void* ReallocateAligned(void* OldPtr, size_t OldSize, size_t NewSize, size_t Alignment);

    INLINE void Deallocate(void* ptr, size_t Size) 
    {
        DeallocateAligned(ptr, Size, Memory::kDefaultAlignment);
    }

    void DeallocateAligned(void* ptr, size_t Size, size_t Alignment);

    // Can only be called for virtual allocators
    // Region sizes are rounded to pages
    // this just outsource the responsability of the memory page(s) to another sub allocator
    // it does not commit it
    // Block ReserveRegion(size_t Size);

    // Not sure if I keep this one but I know that the linear allocator cannot deal well with it
    // And maybe not even the stack one as this reserve release may not happen in a LIFO way
    //void ReleaseRegion(Block& block);


private:
    void InsertFreeNode(size_t offset, size_t chunkCount);

    void* TryPopFreeNode(size_t Size, size_t Alignment);

    Memory::Reporter* m_Reporter;
    Memory::Block m_Memory;
    size_t m_CommitedPage = 0;
    size_t m_ChunkSize;
    size_t m_AllocatedChunkCount = 0;
    size_t m_Leakage = 0;

    FreeNode* m_FreeList = nullptr;

    const Memory::AllocType m_AllocType;
};

template <size_t ChunkSize>
struct PoolAllocator : public _PoolAllocator
{
    static_assert(ChunkSize > sizeof(_PoolAllocator::FreeNode), "ChunkSize must be big enough to fit the invalidation structure");

    PoolAllocator(Memory::Block memory, Memory::AllocType type, Memory::Reporter* Reporter = nullptr):
        _PoolAllocator(memory, ChunkSize, type, Reporter)
    {}

    ~PoolAllocator() = default;
};