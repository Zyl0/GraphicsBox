#include "Memory/VirtualMemory.h"

#include "Shared/Annotations.h"
#include "Shared/Assertion.h"

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#include <memoryapi.h>
#endif // PLATFORM_WINDOWS

#ifdef PLATFORM_LINUX
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#endif // PLATFORM_LINUX

//#define ENABLE_WATCHDOG

namespace Memory::Virtual
{
    static size_t PageSize = 4_KB;

    INLINE size_t RoundToPage(size_t Size)
    {
        // return Size & ~(PageSize - 1) + (Size & (PageSize - 1) > 0 ? PageSize : 0);
        
        return (Size + (PageSize - 1)) & ~(PageSize - 1);
    }

    void CalcPageSize()
    {
#ifdef PLATFORM_WINDOWS
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        PageSize = sysInfo.dwPageSize;
#endif // PLATFORM_WINDOWS
#ifdef PLATFORM_LINUX
        PageSize = sysconf(_SC_PAGESIZE);
#endif // PLATFORM_LINUX
    }
    
    Block Reserve(size_t Size)
    {
        CalcPageSize();
        Size = RoundToPage(Size);
        
        return Reserve(nullptr, Size);
    }

    void* Commit(const Block& Memory, size_t Offset, size_t Size)
    {
        return Commit(nullptr, Memory, Offset, Size);
    }

    void DeCommit(const Block& Memory, size_t Offset, size_t Size)
    {
        DeCommit(nullptr, Memory, Offset, Size);
    }

    void Release(Block& Memory)
    {
        Release(nullptr, Memory);
    }

    Block Reserve(Memory::Reporter* Reporter, size_t Size)
    {
        CalcPageSize();
        Size = RoundToPage(Size);
        
        if (Reporter != nullptr) Reporter->ReportIncrease(Memory::Reporter::RT_VirtualReserved, Size);
#ifdef PLATFORM_WINDOWS
        DWORD extraFlags{};
        
#ifdef ENABLE_WATCHDOG
        extraFlags |= MEM_WRITE_WATCH;
#endif //ENABLE_WATCHDOG
        
        void* ptr = VirtualAlloc(nullptr, Size, MEM_RESERVE | extraFlags, PAGE_READWRITE);
#endif // PLATFORM_WINDOWS

#ifdef PLATFORM_LINUX
        void* ptr = mmap(nullptr, Size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0); 
#endif // PLATFORM_LINUX
        
        AssertOrError(ptr != nullptr, "Could not reserve Virtual Memory")
        return {ptr, Size};
    }

    void* Commit(Memory::Reporter* Reporter, const Block& Memory, size_t Offset, size_t Size)
    {
        if (Reporter != nullptr) Reporter->ReportIncrease(Memory::Reporter::RT_VirtualCommited, Size);
        
        AssertOrError(Memory.Address != nullptr && Memory.Size > 0, "Commit: Memory is empty")
        AssertOrError(Offset + Size <= Memory.Size, "Commit: Requested memory is out of memory block bounds")
        
        void* Ptr = static_cast<uint8_t*>(Memory.Address) + Offset;
        
#ifdef PLATFORM_WINDOWS
        Ptr = VirtualAlloc(Ptr, Size, MEM_COMMIT, PAGE_READWRITE);
        AssertOrError(Ptr != nullptr, "Could not commit Virtual Memory")
#endif // PLATFORM_WINDOWS

#ifdef PLATFORM_LINUX
        int result = mprotect(Ptr, Size, PROT_READ | PROT_WRITE);
        AssertOrError(result == 0, "Could not protect memory when commiting")

        result = madvise(Ptr, Size, MADV_WILLNEED);
        AssertOrError(result == 0, "Could not madvise memory when commiting")
#endif // PLATFORM_LINUX

        return Ptr;
    }

    void DeCommit(Memory::Reporter* Reporter, const Block& Memory, size_t Offset, size_t Size)
    {
        if (Reporter != nullptr) Reporter->ReportDecrease(Memory::Reporter::RT_VirtualCommited, Size);
        
        AssertOrError(Memory.Address != nullptr && Memory.Size > 0, "DeCommit: Memory is empty")

        void* Ptr = static_cast<uint8_t*>(Memory.Address) + Offset;
        
#ifdef PLATFORM_WINDOWS
        BOOL result = VirtualFree(Ptr, Size, MEM_DECOMMIT);
        AssertOrError(result, "Could not de commit Virtual Memory")
#endif // PLATFORM_WINDOWS

#ifdef PLATFORM_LINUX
        int result = madvise(Ptr, Size, MADV_DONTNEED);
        AssertOrError(result == 0, "Could not madvise memory when commiting")
#endif // PLATFORM_LINUX;
    }

    void Release(Memory::Reporter* Reporter, Block& Memory)
    {
        if (Reporter != nullptr) Reporter->ReportDecrease(Memory::Reporter::RT_VirtualReserved, Memory.Size);
        
        AssertOrError(Memory.Address != nullptr && Memory.Size > 0, "Virtual memory release called on an empty memory")
        
#ifdef PLATFORM_WINDOWS
        BOOL result = VirtualFree(Memory.Address, 0, MEM_RELEASE);
        AssertOrError(result, "Could not release Virtual Memory")
#endif // PLATFORM_WINDOWS

#ifdef PLATFORM_LINUX
        int result = munmap(Memory.Address, Memory.Size);
        AssertOrError(result == 0, "Could not release Virtual Memory")
#endif // PLATFORM_LINUX

        Memory.Address = nullptr;
        Memory.Size = 0;
    }

    size_t GetPageSize()
    {
        return PageSize;
    }
}