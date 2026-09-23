#include "catch2/catch_all.hpp"

#include "Memory/Reporter.h"
#include "Memory/Allocators.h"
#include "Memory/MemAlloc.h"
#include "Memory/VirtualMemory.h"

static bool IsAligned(void* ptr, size_t alignment)
{
    return (reinterpret_cast<uintptr_t>(ptr) % alignment) == 0;
}

TEST_CASE("Linear Allocator Allocation")
{
    Memory::Reporter reporter = {"Linear allocation"};
    Memory::Block memory = Memory::Allocate(&reporter, 4_KB);
    Memory::Block virtualMemory = Memory::Virtual::Reserve(&reporter, 16_KB);
    
    WHEN("Allocator is working on physical memory")
    {
        SECTION("Can allocate within capacity")
        {
            LinearAllocator allocator{memory, Memory::AT_Physical, &reporter};

            void* ptr = allocator.Allocate(256);
            REQUIRE(ptr != nullptr);
        }

        SECTION("Can allocate Multiple allocations increment used size")
        {
            LinearAllocator allocator{memory, Memory::AT_Physical, &reporter};

            void* ptr1 = allocator.Allocate(100);
            void* ptr2 = allocator.Allocate(200);
            void* ptr3 = allocator.Allocate(300);
            
            REQUIRE(ptr1 != nullptr);
            REQUIRE(ptr2 != nullptr);
            REQUIRE(ptr3 != nullptr);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 600);
        }

        SECTION("Allocations are contiguous")
        {
            LinearAllocator allocator{memory, Memory::AT_Physical, &reporter};

            void* ptr1 = allocator.Allocate(128);
            void* ptr2 = allocator.Allocate(128);
            
            uintptr_t addr1 = reinterpret_cast<uintptr_t>(ptr1);
            uintptr_t addr2 = reinterpret_cast<uintptr_t>(ptr2);
            
            REQUIRE(addr2 == addr1 + 128);
        }

        SECTION("Cannot allocate bigger than capacity")
        {
            LinearAllocator allocator{memory, Memory::AT_Physical, &reporter};

            void* ptr = allocator.Allocate(4097);
            REQUIRE(ptr == nullptr);
        }
        
        SECTION("Cannot allocate out of bounds")
        {
            LinearAllocator allocator{memory, Memory::AT_Physical, &reporter};

            void* ptr1 = allocator.Allocate(1024);
            void* ptr2 = allocator.Allocate(1024);
            void* ptr3 = allocator.Allocate(1024);
            void* ptr4 = allocator.Allocate(1024);
            void* ptr5 = allocator.Allocate(1024);

            REQUIRE(ptr1 != nullptr);
            REQUIRE(ptr2 != nullptr);
            REQUIRE(ptr3 != nullptr);
            REQUIRE(ptr4 != nullptr);
            REQUIRE(ptr5 == nullptr);
        }

        SECTION("Aligned allocation")
        {
            LinearAllocator allocator{memory, Memory::AT_Physical, &reporter};
            const size_t alignment = 64;
            
            void* p = allocator.AllocateAligned(256, alignment);
            REQUIRE(p != nullptr);
            REQUIRE(IsAligned(p, alignment));
        }

        SECTION("Aligned allocations with different alignments are not contiguous")
        {
            LinearAllocator allocator{memory, Memory::AT_Physical, &reporter};

            void* ptr0 = allocator.AllocateAligned(128, 128);
            void* ptr1 = allocator.AllocateAligned(16, 16);
            void* ptr2 = allocator.AllocateAligned(128, 128);
            
            uintptr_t addr1 = reinterpret_cast<uintptr_t>(ptr1);
            uintptr_t addr2 = reinterpret_cast<uintptr_t>(ptr2);
            
            REQUIRE(addr2 != addr1 + 16);
        }

        SECTION("Aligned allocations with same alignments but size that does not match the alignment are not contiguous")
        {
            LinearAllocator allocator{memory, Memory::AT_Physical, &reporter};

            void* ptr0 = allocator.AllocateAligned(128, 128);
            void* ptr1 = allocator.AllocateAligned(100, 16);
            void* ptr2 = allocator.AllocateAligned(100, 32);
            
            uintptr_t addr1 = reinterpret_cast<uintptr_t>(ptr1);
            uintptr_t addr2 = reinterpret_cast<uintptr_t>(ptr2);
            
            REQUIRE(addr2 != addr1 + 100);
        }
    }

    WHEN("Allocator is working on virtual memory")
    {
        SECTION("Can allocate within capacity")
        {
            LinearAllocator allocator{virtualMemory, Memory::AT_Virtual, &reporter};

            void* ptr = allocator.Allocate(256);
            REQUIRE(ptr != nullptr);
        }

        SECTION("Can allocate Multiple allocations increment used size")
        {
            LinearAllocator allocator{virtualMemory, Memory::AT_Virtual, &reporter};

            void* ptr1 = allocator.Allocate(100);
            void* ptr2 = allocator.Allocate(200);
            void* ptr3 = allocator.Allocate(300);
            
            REQUIRE(ptr1 != nullptr);
            REQUIRE(ptr2 != nullptr);
            REQUIRE(ptr3 != nullptr);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 600);
        }

        SECTION("Allocations are contiguous")
        {
            LinearAllocator allocator{virtualMemory, Memory::AT_Virtual, &reporter};

            void* ptr1 = allocator.Allocate(128);
            void* ptr2 = allocator.Allocate(128);
            
            uintptr_t addr1 = reinterpret_cast<uintptr_t>(ptr1);
            uintptr_t addr2 = reinterpret_cast<uintptr_t>(ptr2);
            
            REQUIRE(addr2 == addr1 + 128);
        }

        SECTION("Cannot allocate bigger than capacity")
        {
            LinearAllocator allocator{virtualMemory, Memory::AT_Virtual, &reporter};

            void* ptr = allocator.Allocate(16_KB + 16);
            REQUIRE(ptr == nullptr);
        }
        
        SECTION("Cannot allocate out of bounds")
        {
            LinearAllocator allocator{virtualMemory, Memory::AT_Virtual, &reporter};

            void* ptr1 = allocator.Allocate(4_KB);
            void* ptr2 = allocator.Allocate(4_KB);
            void* ptr3 = allocator.Allocate(4_KB);
            void* ptr4 = allocator.Allocate(4_KB);
            void* ptr5 = allocator.Allocate(1024);

            REQUIRE(ptr1 != nullptr);
            REQUIRE(ptr2 != nullptr);
            REQUIRE(ptr3 != nullptr);
            REQUIRE(ptr4 != nullptr);
            REQUIRE(ptr5 == nullptr);
        }

        SECTION("Aligned allocation")
        {
            LinearAllocator allocator{virtualMemory, Memory::AT_Virtual, &reporter};
            const size_t alignment = 64;
            
            void* p = allocator.AllocateAligned(256, alignment);
            REQUIRE(p != nullptr);
            REQUIRE(IsAligned(p, alignment));
        }

        SECTION("Aligned allocations with different alignments are not contiguous")
        {
            LinearAllocator allocator{virtualMemory, Memory::AT_Virtual, &reporter};

            void* ptr1 = allocator.AllocateAligned(16, 16);
            void* ptr2 = allocator.AllocateAligned(128, 128);
            
            uintptr_t addr1 = reinterpret_cast<uintptr_t>(ptr1);
            uintptr_t addr2 = reinterpret_cast<uintptr_t>(ptr2);
            
            REQUIRE(addr2 != addr1 + 16);
        }

        SECTION("Aligned allocations with same alignments but size that does not match the alignment are not contiguous")
        {
            LinearAllocator allocator{virtualMemory, Memory::AT_Virtual, &reporter};

            void* ptr1 = allocator.AllocateAligned(100, 16);
            void* ptr2 = allocator.AllocateAligned(100, 32);
            
            uintptr_t addr1 = reinterpret_cast<uintptr_t>(ptr1);
            uintptr_t addr2 = reinterpret_cast<uintptr_t>(ptr2);
            
            REQUIRE(addr2 != addr1 + 100);
        }
    }

    Memory::Deallocate(&reporter, memory);
    Memory::Virtual::Release(&reporter, virtualMemory);
    
    REQUIRE(reporter.Size(Memory::Reporter::RT_Allocated) == 0);
}

TEST_CASE("Linear Allocator Deallocation")
{
    Memory::Reporter reporter = {"Linear allocation"};
    Memory::Block memory = Memory::Allocate(&reporter, 4_KB);
    Memory::Block virtualMemory = Memory::Virtual::Reserve(&reporter, 16_KB);
    
    WHEN("Allocator is working on physical memory")
    {
        SECTION("Can free memory")
        {
            LinearAllocator allocator{memory, Memory::AT_Physical, &reporter};
        
            void* ptr = allocator.Allocate(256);

            REQUIRE(ptr != nullptr);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 256);
        
            allocator.Deallocate(ptr, 256);
        
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 0);
        }
    }

    WHEN("Allocator is working on virtual memory")
    {
        SECTION("Can free memory")
        {
            LinearAllocator allocator{virtualMemory, Memory::AT_Virtual, &reporter};
        
            void* ptr = allocator.Allocate(256);

            REQUIRE(ptr != nullptr);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 256);
        
            allocator.Deallocate(ptr, 256);
        
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 0);
        }
    }

    Memory::Deallocate(&reporter, memory);
    Memory::Virtual::Release(&reporter, virtualMemory);
    
    REQUIRE(reporter.Size(Memory::Reporter::RT_Allocated) == 0);
}

TEST_CASE("Linear Allocator Reallocation")
{
    Memory::Reporter reporter = {"Linear allocation"};
    Memory::Block memory = Memory::Allocate(&reporter, 4_KB);
    Memory::Block virtualMemory = Memory::Virtual::Reserve(&reporter, 16_KB);
    
    WHEN("Allocator is working on physical memory")
    {
        SECTION("Can reallocate more memory")
        {
            LinearAllocator allocator{memory, Memory::AT_Physical, &reporter};
            
            void* ptr = allocator.Allocate(256);

            REQUIRE(ptr != nullptr);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 256);

            allocator.Reallocate(ptr, 256, 512);

            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 512);
        }

        SECTION("Can reallocate less memory")
        {
            LinearAllocator allocator{memory, Memory::AT_Physical, &reporter};
            
            void* ptr = allocator.Allocate(256);

            REQUIRE(ptr != nullptr);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 256);

            allocator.Reallocate(ptr, 256, 128);

            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 128);
        }

        SECTION("Reallocation the last allocation does not move the pointer")
        {
            LinearAllocator allocator{memory, Memory::AT_Physical, &reporter};
            
            void* ptr1 = allocator.Allocate(256);
            void* ptr2 = allocator.Allocate(256);

            REQUIRE(ptr1 != nullptr);
            REQUIRE(ptr2 != nullptr);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 512);

            void* newPtr2 = allocator.Reallocate(ptr2, 256, 512);
            REQUIRE(ptr2 == newPtr2);

            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (512 + 256));
        }

        SECTION("Reallocation the any allocation but last does move the pointer")
        {
            LinearAllocator allocator{memory, Memory::AT_Physical, &reporter};

            void* ptr1 = allocator.Allocate(256);
            void* ptr2 = allocator.Allocate(256);

            REQUIRE(ptr1 != nullptr);
            REQUIRE(ptr2 != nullptr);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used)== 512);

            void* newPtr1 = allocator.Reallocate(ptr1, 256, 512);
            REQUIRE(ptr1 != newPtr1);

            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 1024);

            allocator.Deallocate(ptr1, 256);
            
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (512 + 256));
        }

        SECTION("ReallocateAligned maintains alignment")
        {
            LinearAllocator allocator{memory, Memory::AT_Physical, &reporter};

            constexpr size_t oldSize = 64;
            constexpr size_t newSize = 256;
            constexpr size_t alignment = 32;

            void* p = allocator.AllocateAligned(oldSize, alignment);
            REQUIRE(IsAligned(p, alignment));

            void* newP = allocator.ReallocateAligned(p, oldSize, newSize, alignment);
            REQUIRE(IsAligned(newP, alignment));

            allocator.DeallocateAligned(newP, newSize, alignment);
        }
    }

    WHEN("Allocator is working on virtual memory")
    {
        SECTION("Can reallocate more memory")
        {
            LinearAllocator allocator{virtualMemory, Memory::AT_Virtual, &reporter};
            
            void* ptr = allocator.Allocate(256);

            REQUIRE(ptr != nullptr);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 256);

            allocator.Reallocate(ptr, 256, 512);

            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 512);
        }

        SECTION("Can reallocate less memory")
        {
            LinearAllocator allocator{virtualMemory, Memory::AT_Virtual, &reporter};
            
            void* ptr = allocator.Allocate(256);

            REQUIRE(ptr != nullptr);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 256);

            allocator.Reallocate(ptr, 256, 128);

            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 128);
        }

        SECTION("Reallocation the last allocation does not move the pointer")
        {
            LinearAllocator allocator{virtualMemory, Memory::AT_Virtual, &reporter};
            
            void* ptr1 = allocator.Allocate(256);
            void* ptr2 = allocator.Allocate(256);

            REQUIRE(ptr1 != nullptr);
            REQUIRE(ptr2 != nullptr);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 512);

            void* newPtr2 = allocator.Reallocate(ptr2, 256, 512);
            REQUIRE(ptr2 == newPtr2);

            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (512 + 256));
        }

        SECTION("Reallocation the any allocation but last does move the pointer")
        {
            LinearAllocator allocator{virtualMemory, Memory::AT_Virtual, &reporter};

            void* ptr1 = allocator.Allocate(256);
            void* ptr2 = allocator.Allocate(256);

            REQUIRE(ptr1 != nullptr);
            REQUIRE(ptr2 != nullptr);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used)== 512);

            void* newPtr1 = allocator.Reallocate(ptr1, 256, 512);
            REQUIRE(ptr1 != newPtr1);

            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 1024);

            allocator.Deallocate(ptr1, 256);
            
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (512 + 256));
        }

        SECTION("ReallocateAligned maintains alignment")
        {
            LinearAllocator allocator{virtualMemory, Memory::AT_Virtual, &reporter};

            constexpr size_t oldSize = 64;
            constexpr size_t newSize = 256;
            constexpr size_t alignment = 32;

            void* p = allocator.AllocateAligned(oldSize, alignment);
            REQUIRE(IsAligned(p, alignment));

            void* newP = allocator.ReallocateAligned(p, oldSize, newSize, alignment);
            REQUIRE(IsAligned(newP, alignment));

            allocator.DeallocateAligned(newP, newSize, alignment);
        }
    }

    Memory::Deallocate(&reporter, memory);
    Memory::Virtual::Release(&reporter, virtualMemory);
    
    REQUIRE(reporter.Size(Memory::Reporter::RT_Allocated) == 0);
}