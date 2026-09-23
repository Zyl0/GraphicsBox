#include "catch2/catch_all.hpp"

#include "Memory/Reporter.h"
#include "Memory/Allocators.h"

static bool IsAligned(void* ptr, size_t alignment)
{
    return (reinterpret_cast<uintptr_t>(ptr) % alignment) == 0;
}

TEST_CASE("System Allocator Allocation")
{
    Memory::Reporter Reporter {"System Allocations"};
    
    SECTION("Can allocate within capacity")
    {
        SystemAllocator allocator(&Reporter);

        void* ptr = allocator.Allocate(256);
        REQUIRE(ptr != nullptr);

        allocator.Deallocate(ptr, 256);
    }

    SECTION("Can allocate Multiple allocations increment used size")
    {
        SystemAllocator allocator(&Reporter);

        void* ptr1 = allocator.Allocate(100);
        void* ptr2 = allocator.Allocate(200);
        void* ptr3 = allocator.Allocate(300);
        
        REQUIRE(ptr1 != nullptr);
        REQUIRE(ptr2 != nullptr);
        REQUIRE(ptr3 != nullptr);
        REQUIRE(Reporter.Size(Memory::Reporter::RT_Used) == 600);

        allocator.Deallocate(ptr1, 100);
        allocator.Deallocate(ptr2, 200);
        allocator.Deallocate(ptr3, 300);
    }
    
    SECTION("Aligned allocation")
    {
        SystemAllocator allocator(&Reporter);
        const size_t alignment = 64;
        
        void* p = allocator.AllocateAligned(256, alignment);
        REQUIRE(p != nullptr);
        REQUIRE(IsAligned(p, alignment));

        allocator.DeallocateAligned(p, 256, alignment);
    }
}

TEST_CASE("System Allocator Deallocation")
{
    Memory::Reporter Reporter {"System Allocations"};
    
    SECTION("Can free memory")
    {
        SystemAllocator allocator(&Reporter);
        
        void* ptr = allocator.Allocate(256);

        REQUIRE(ptr != nullptr);
        REQUIRE(Reporter.Size(Memory::Reporter::RT_Used) == 256);
        
        allocator.Deallocate(ptr, 256);
        
        REQUIRE(Reporter.Size(Memory::Reporter::RT_Used) == 0);
    }
}

TEST_CASE("System Allocator Reallocation")
{
    Memory::Reporter Reporter {"System Allocations"};
    
    SECTION("Can reallocate more memory")
    {
        SystemAllocator allocator(&Reporter);
        
        void* ptr = allocator.Allocate(256);

        REQUIRE(ptr != nullptr);
        REQUIRE(Reporter.Size(Memory::Reporter::RT_Used) == 256);

        void* postPtr = allocator.Reallocate(ptr, 256, 512);
        if (postPtr != ptr)
        {
            allocator.Deallocate(ptr, 256);
            ptr = postPtr;
        }

        REQUIRE(Reporter.Size(Memory::Reporter::RT_Used) == 512);

        allocator.Deallocate(ptr, 512);
    }

    SECTION("Can reallocate less memory")
    {
        SystemAllocator allocator(&Reporter);
        
        void* ptr = allocator.Allocate(256);

        REQUIRE(ptr != nullptr);
        REQUIRE(Reporter.Size(Memory::Reporter::RT_Used) == 256);

        void* postPtr = allocator.Reallocate(ptr, 256, 128);
        if (postPtr != ptr)
        {
            allocator.Deallocate(ptr, 256);
            ptr = postPtr;
        }

        REQUIRE(Reporter.Size(Memory::Reporter::RT_Used) == 128);

        allocator.Deallocate(ptr, 128);
    }

    SECTION("Reallocation does move the pointer")
    {
        SystemAllocator allocator(&Reporter);

        void* ptr1 = allocator.Allocate(256);
        void* ptr2 = allocator.Allocate(256);

        REQUIRE(ptr1 != nullptr);
        REQUIRE(ptr2 != nullptr);
        REQUIRE(Reporter.Size(Memory::Reporter::RT_Used)== 512);

        void* newPtr1 = allocator.Reallocate(ptr1, 256, 512);
        REQUIRE(ptr1 != newPtr1);

        REQUIRE(Reporter.Size(Memory::Reporter::RT_Used) == 1024);

        allocator.Deallocate(ptr1, 256);
        ptr1 = newPtr1;
        
        REQUIRE(Reporter.Size(Memory::Reporter::RT_Used) == (512 + 256));
        
        allocator.Deallocate(ptr1, 512);
        allocator.Deallocate(ptr2, 256);
    }

    SECTION("ReallocateAligned maintains alignment")
    {
        SystemAllocator allocator(&Reporter);

        constexpr size_t oldSize = 64;
        constexpr size_t newSize = 256;
        constexpr size_t alignment = 32;

        void* p = allocator.AllocateAligned(oldSize, alignment);
        REQUIRE(IsAligned(p, alignment));

        void* newP = allocator.ReallocateAligned(p, oldSize, newSize, alignment);
        REQUIRE(IsAligned(newP, alignment));

        allocator.DeallocateAligned(p, oldSize, alignment);
        allocator.DeallocateAligned(newP, newSize, alignment);
    }
}
