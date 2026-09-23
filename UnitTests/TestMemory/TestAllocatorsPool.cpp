#include "catch2/catch_all.hpp"

#include "Memory/Reporter.h"
#include "Memory/Allocators.h"
#include "Memory/MemAlloc.h"
#include "Memory/VirtualMemory.h"

static constexpr size_t kStressTestLenght = 16;
static constexpr size_t kPoolSize = 256;

static bool IsAligned(void* ptr, size_t alignment)
{
    return (reinterpret_cast<uintptr_t>(ptr) % alignment) == 0;
}

TEST_CASE("Pool Allocator Allocation")
{
    Memory::Reporter reporter = {"Pool allocation"};
    Memory::Block memory = Memory::Allocate(&reporter, 4_KB);
    Memory::Block virtualMemory = Memory::Virtual::Reserve(&reporter, 16_KB);
    
    WHEN("Allocator is working on physical memory")
    {
        SECTION("Can allocate within capacity")
        {
            PoolAllocator<kPoolSize> allocator{memory, Memory::AT_Physical, &reporter};

            void* ptr = allocator.Allocate(256);
            REQUIRE(ptr != nullptr);
        }

        SECTION("Can allocate Multiple allocations increment used size")
        {
            PoolAllocator<kPoolSize> allocator{memory, Memory::AT_Physical, &reporter};

            void* ptr1 = allocator.Allocate(100);
            void* ptr2 = allocator.Allocate(200);
            void* ptr3 = allocator.Allocate(300);
        
            REQUIRE(ptr1 != nullptr);
            REQUIRE(ptr2 != nullptr);
            REQUIRE(ptr3 != nullptr);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 600);
        }

        SECTION("Cannot allocate bigger than capacity")
        {
            PoolAllocator<kPoolSize> allocator{memory, Memory::AT_Physical, &reporter};

            void* ptr = allocator.Allocate(4097);
            REQUIRE(ptr == nullptr);
        }

        SECTION("Cannot allocate out of bounds")
        {
            PoolAllocator<kPoolSize> allocator{memory, Memory::AT_Physical, &reporter};

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
            PoolAllocator<kPoolSize> allocator{memory, Memory::AT_Physical, &reporter};
            const size_t alignment = 64;
        
            void* p = allocator.AllocateAligned(256, alignment);
            REQUIRE(p != nullptr);
            REQUIRE(IsAligned(p, alignment));
        }

        SECTION("Multiple Aligned allocation")
        {
            PoolAllocator<kPoolSize> allocator{memory, Memory::AT_Physical, &reporter};

            for (size_t i = 0; i < 4; i++)
            {
                {
                    const size_t alignment = 64;
        
                    void* p = allocator.AllocateAligned(64, alignment);
                    REQUIRE(p != nullptr);
                    REQUIRE(IsAligned(p, alignment));
                }
                
                {
                    const size_t alignment = 16;
        
                    void* p = allocator.AllocateAligned(16, alignment);
                    REQUIRE(p != nullptr);
                    REQUIRE(IsAligned(p, alignment));
                }

                {
                    const size_t alignment = 128;
        
                    void* p = allocator.AllocateAligned(128, alignment);
                    REQUIRE(p != nullptr);
                    REQUIRE(IsAligned(p, alignment));
                }
            }
        }
    }

    WHEN("Allocator is working on virtual memory")
    {
        SECTION("Can allocate within capacity")
        {
            PoolAllocator<kPoolSize> allocator{virtualMemory, Memory::AT_Virtual, &reporter};

            void* ptr = allocator.Allocate(256);
            REQUIRE(ptr != nullptr);
        }

        SECTION("Can allocate Multiple allocations increment used size")
        {
            PoolAllocator<kPoolSize> allocator{virtualMemory, Memory::AT_Virtual, &reporter};

            void* ptr1 = allocator.Allocate(100);
            void* ptr2 = allocator.Allocate(200);
            void* ptr3 = allocator.Allocate(300);
        
            REQUIRE(ptr1 != nullptr);
            REQUIRE(ptr2 != nullptr);
            REQUIRE(ptr3 != nullptr);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 600);
        }

        SECTION("Cannot allocate bigger than capacity")
        {
            PoolAllocator<kPoolSize> allocator{virtualMemory, Memory::AT_Virtual, &reporter};

            void* ptr = allocator.Allocate(16_KB + 16);
            REQUIRE(ptr == nullptr);
        }
        
        SECTION("Cannot allocate out of bounds")
        {
            PoolAllocator<kPoolSize> allocator{virtualMemory, Memory::AT_Virtual, &reporter};

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
            PoolAllocator<kPoolSize> allocator{virtualMemory, Memory::AT_Virtual, &reporter};
            const size_t alignment = 64;
        
            void* p = allocator.AllocateAligned(256, alignment);
            REQUIRE(p != nullptr);
            REQUIRE(IsAligned(p, alignment));
        }

        SECTION("Multiple Aligned allocation")
        {
            PoolAllocator<kPoolSize> allocator{virtualMemory, Memory::AT_Virtual, &reporter};

            for (size_t i = 0; i < 4; i++)
            {
                {
                    const size_t alignment = 64;
        
                    void* p = allocator.AllocateAligned(64, alignment);
                    REQUIRE(p != nullptr);
                    REQUIRE(IsAligned(p, alignment));
                }
                
                {
                    const size_t alignment = 16;
        
                    void* p = allocator.AllocateAligned(16, alignment);
                    REQUIRE(p != nullptr);
                    REQUIRE(IsAligned(p, alignment));
                }

                {
                    const size_t alignment = 128;
        
                    void* p = allocator.AllocateAligned(128, alignment);
                    REQUIRE(p != nullptr);
                    REQUIRE(IsAligned(p, alignment));
                }
            }
        }
    }
    
    Memory::Deallocate(&reporter, memory);
    Memory::Virtual::Release(&reporter, virtualMemory);
    
    REQUIRE(reporter.Size(Memory::Reporter::RT_Allocated) == 0);
}

TEST_CASE("Pool Allocator Deallocation")
{
    Memory::Reporter reporter = {"Pool allocation"};
    Memory::Block memory = Memory::Allocate(&reporter, 4_KB);
    Memory::Block virtualMemory = Memory::Virtual::Reserve(&reporter, 16_KB);
    
    WHEN("Allocator is working on physical memory")
    {
        SECTION("Can free memory")
        {
            PoolAllocator<kPoolSize> allocator{memory, Memory::AT_Physical, &reporter};
        
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
            PoolAllocator<kPoolSize> allocator{virtualMemory, Memory::AT_Virtual, &reporter};
        
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

TEST_CASE("Pool Allocator Reallocation")
{
    Memory::Reporter reporter = {"Pool allocation"};
    Memory::Block memory = Memory::Allocate(&reporter, 4_KB);
    Memory::Block virtualMemory = Memory::Virtual::Reserve(&reporter, 16_KB);
    
    WHEN("Allocator is working on physical memory")
    {
        SECTION("Can reallocate more memory")
        {
            PoolAllocator<kPoolSize> allocator{memory, Memory::AT_Physical, &reporter};
            
            void* ptr = allocator.Allocate(256);

            REQUIRE(ptr != nullptr);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 256);

            allocator.Reallocate(ptr, 256, 512);

            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 512);
        }

        SECTION("Can reallocate less memory")
        {
            PoolAllocator<kPoolSize> allocator{memory, Memory::AT_Physical, &reporter};
            
            void* ptr = allocator.Allocate(256);

            REQUIRE(ptr != nullptr);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 256);

            allocator.Reallocate(ptr, 256, 128);

            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 128);
        }

        SECTION("Reallocation the last allocation does not move the pointer")
        {
            PoolAllocator<kPoolSize> allocator{memory, Memory::AT_Physical, &reporter};
            
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
            PoolAllocator<kPoolSize> allocator{memory, Memory::AT_Physical, &reporter};

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
            PoolAllocator<kPoolSize> allocator{memory, Memory::AT_Physical, &reporter};

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
            PoolAllocator<kPoolSize> allocator{virtualMemory, Memory::AT_Virtual, &reporter};
            
            void* ptr = allocator.Allocate(256);

            REQUIRE(ptr != nullptr);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 256);

            allocator.Reallocate(ptr, 256, 512);

            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 512);
        }

        SECTION("Can reallocate less memory")
        {
            PoolAllocator<kPoolSize> allocator{virtualMemory, Memory::AT_Virtual, &reporter};
            
            void* ptr = allocator.Allocate(256);

            REQUIRE(ptr != nullptr);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 256);

            allocator.Reallocate(ptr, 256, 128);

            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 128);
        }

        SECTION("Reallocation the last allocation does not move the pointer")
        {
            PoolAllocator<kPoolSize> allocator{virtualMemory, Memory::AT_Virtual, &reporter};
            
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
            PoolAllocator<kPoolSize> allocator{virtualMemory, Memory::AT_Virtual, &reporter};

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
            PoolAllocator<kPoolSize> allocator{virtualMemory, Memory::AT_Virtual, &reporter};

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

TEST_CASE("Pool Allocator Stress test")
{
    Memory::Reporter reporter = {"Pool allocation"};
    Memory::Block memory = Memory::Allocate(&reporter, 4_KB);
    Memory::Block virtualMemory = Memory::Virtual::Reserve(&reporter, 16_KB);
    
    WHEN("Allocator is working on physical memory")
    {
        SECTION("Can modify multiple times the same memory")
        {
            PoolAllocator<kPoolSize> allocator{memory, Memory::AT_Physical, &reporter};

            void* ptr = allocator.Allocate(256);
            REQUIRE(ptr != nullptr);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 256);

            for (size_t i = 0; i < 256; i++)
            {
                {
                    void* nptr = allocator.Reallocate(ptr, 256, 512);
                    if (nptr != ptr) allocator.Deallocate(ptr, 256);
                    ptr = nptr;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 512);
                }

                {
                    void* nptr = allocator.Reallocate(ptr, 512, 16);
                    if (nptr != ptr) allocator.Deallocate(ptr, 512);
                    ptr = nptr;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 16);
                }

                {
                    void* nptr = allocator.Reallocate(ptr, 16, 100);
                    if (nptr != ptr) allocator.Deallocate(ptr, 16);
                    ptr = nptr;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 100);
                }

                {
                    void* nptr = allocator.Reallocate(ptr, 100, 9);
                    if (nptr != ptr) allocator.Deallocate(ptr, 100);
                    ptr = nptr;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 9);
                }

                {
                    void* nptr = allocator.Reallocate(ptr, 9, 2563);
                    if (nptr != ptr) allocator.Deallocate(ptr, 9);
                    ptr = nptr;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 2563);
                }
            
                {
                    void* nptr = allocator.Reallocate(ptr, 2563, 256);
                    if (nptr != ptr) allocator.Deallocate(ptr, 2563);
                    ptr = nptr;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 256);
                }
            }

            allocator.Deallocate(ptr, 256);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 0);
        }

        SECTION("Can modify multiple times different memories")
        {
            PoolAllocator<kPoolSize> allocator{memory, Memory::AT_Physical, &reporter};

            void* ptr1 = allocator.Allocate(256);
            void* ptr2 = allocator.Allocate(256);
            void* ptr3 = allocator.Allocate(256);
            REQUIRE(ptr1 != nullptr);
            REQUIRE(ptr2 != nullptr);
            REQUIRE(ptr3 != nullptr);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (256 * 3));

            for (size_t i = 0; i < 256; i++)
            {
                // 1
                {
                    void* nptr1 = allocator.Reallocate(ptr1, 256, 512);
                    if (nptr1 != ptr1) allocator.Deallocate(ptr1, 256);
                    ptr1 = nptr1;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (256 * 2 + 512));
                    void* nptr2 = allocator.Reallocate(ptr2, 256, 512);
                    if (nptr2 != ptr2) allocator.Deallocate(ptr2, 256);
                    ptr2 = nptr2;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (512 * 2 + 256));
                    void* nptr3 = allocator.Reallocate(ptr3, 256, 512);
                    if (nptr3 != ptr3) allocator.Deallocate(ptr3, 256);
                    ptr3 = nptr3;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (512 * 3));
                }

                // 2
                {
                    void* nptr1 = allocator.Reallocate(ptr1, 512, 16);
                    if (nptr1 != ptr1) allocator.Deallocate(ptr1, 512);
                    ptr1 = nptr1;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (512 * 2 + 16));
                    void* nptr2 = allocator.Reallocate(ptr2, 512, 16);
                    if (nptr2 != ptr2) allocator.Deallocate(ptr2, 512);
                    ptr2 = nptr2;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (512 + 16 * 2));
                }

                // 3
                {
                    void* nptr1 = allocator.Reallocate(ptr1, 16, 100);
                    if (nptr1 != ptr1) allocator.Deallocate(ptr1, 16);
                    ptr1 = nptr1;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (512 + 16 + 100));
                    void* nptr2 = allocator.Reallocate(ptr2, 16, 100);
                    if (nptr2 != ptr2) allocator.Deallocate(ptr2, 16);
                    ptr2 = nptr2;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (512 + 100 * 2));
                }

                // 2 + 3
                {
                    void* nptr3 = allocator.Reallocate(ptr3, 512, 16);
                    if (nptr3 != ptr3) allocator.Deallocate(ptr3, 512);
                    ptr3 = nptr3;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (100 * 2 + 16));
                    nptr3 = allocator.Reallocate(ptr3, 16, 100);
                    if (nptr3 != ptr3) allocator.Deallocate(ptr3, 16);
                    ptr3 = nptr3;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (100 * 3));
                }

                // 4 + 5
                {
                    void* nptr1 = allocator.Reallocate(ptr1, 100, 9);
                    if (nptr1 != ptr1) allocator.Deallocate(ptr1, 100);
                    ptr1 = nptr1;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (100 * 2 + 9));
                    nptr1 = allocator.Reallocate(ptr1, 9, 256);
                    if (nptr1 != ptr1) allocator.Deallocate(ptr1, 9);
                    ptr1 = nptr1;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (256 + 100 * 2));
                }

                // 4
                {
                    void* nptr2 = allocator.Reallocate(ptr2, 100, 9);
                    if (nptr2 != ptr2) allocator.Deallocate(ptr2, 100);
                    ptr2 = nptr2;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (256 + 100 + 9));
                    void* nptr3 = allocator.Reallocate(ptr3, 100, 9);
                    if (nptr3 != ptr3) allocator.Deallocate(ptr3, 100);
                    ptr3 = nptr3;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (256 + 9 * 2));
                }

                // 5
                {
                    void* nptr2 = allocator.Reallocate(ptr2, 9, 256);
                    if (nptr2 != ptr2) allocator.Deallocate(ptr2, 9);
                    ptr2 = nptr2;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (256 * 2 + 9));
                    void* nptr3 = allocator.Reallocate(ptr3, 9, 256);
                    if (nptr3 != ptr3) allocator.Deallocate(ptr3, 9);
                    ptr3 = nptr3;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (256 * 3));
                }
            }

            allocator.Deallocate(ptr1, 256);
            allocator.Deallocate(ptr2, 256);
            allocator.Deallocate(ptr3, 256);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 0);
        }
    }
    
    WHEN("Allocator is working on virtual memory")
    {
        SECTION("Can modify multiple times the same memory")
        {
            PoolAllocator<kPoolSize> allocator{virtualMemory, Memory::AT_Virtual, &reporter};

            void* ptr = allocator.Allocate(256);
            REQUIRE(ptr != nullptr);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 256);

            for (size_t i = 0; i < kStressTestLenght; i++)
            {
                {
                    void* nptr = allocator.Reallocate(ptr, 256, 512);
                    if (nptr != ptr) allocator.Deallocate(ptr, 256);
                    ptr = nptr;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 512);
                }

                {
                    void* nptr = allocator.Reallocate(ptr, 512, 16);
                    if (nptr != ptr) allocator.Deallocate(ptr, 512);
                    ptr = nptr;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 16);
                }

                {
                    void* nptr = allocator.Reallocate(ptr, 16, 100);
                    if (nptr != ptr) allocator.Deallocate(ptr, 16);
                    ptr = nptr;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 100);
                }

                {
                    void* nptr = allocator.Reallocate(ptr, 100, 9);
                    if (nptr != ptr) allocator.Deallocate(ptr, 100);
                    ptr = nptr;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 9);
                }

                {
                    void* nptr = allocator.Reallocate(ptr, 9, 2563);
                    if (nptr != ptr) allocator.Deallocate(ptr, 9);
                    ptr = nptr;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 2563);
                }
            
                {
                    void* nptr = allocator.Reallocate(ptr, 2563, 256);
                    if (nptr != ptr) allocator.Deallocate(ptr, 2563);
                    ptr = nptr;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 256);
                }
            }

            allocator.Deallocate(ptr, 256);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 0);
        }

        SECTION("Can modify multiple times different memories")
        {
            PoolAllocator<kPoolSize> allocator{virtualMemory, Memory::AT_Virtual, &reporter};

            void* ptr1 = allocator.Allocate(256);
            void* ptr2 = allocator.Allocate(256);
            void* ptr3 = allocator.Allocate(256);
            REQUIRE(ptr1 != nullptr);
            REQUIRE(ptr2 != nullptr);
            REQUIRE(ptr3 != nullptr);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (256 * 3));

            for (size_t i = 0; i < kStressTestLenght; i++)
            {
                // 1
                {
                    void* nptr1 = allocator.Reallocate(ptr1, 256, 512);
                    if (nptr1 != ptr1) allocator.Deallocate(ptr1, 256);
                    ptr1 = nptr1;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (256 * 2 + 512));
                    void* nptr2 = allocator.Reallocate(ptr2, 256, 512);
                    if (nptr2 != ptr2) allocator.Deallocate(ptr2, 256);
                    ptr2 = nptr2;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (512 * 2 + 256));
                    void* nptr3 = allocator.Reallocate(ptr3, 256, 512);
                    if (nptr3 != ptr3) allocator.Deallocate(ptr3, 256);
                    ptr3 = nptr3;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (512 * 3));
                }

                // 2
                {
                    void* nptr1 = allocator.Reallocate(ptr1, 512, 16);
                    if (nptr1 != ptr1) allocator.Deallocate(ptr1, 512);
                    ptr1 = nptr1;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (512 * 2 + 16));
                    void* nptr2 = allocator.Reallocate(ptr2, 512, 16);
                    if (nptr2 != ptr2) allocator.Deallocate(ptr2, 512);
                    ptr2 = nptr2;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (512 + 16 * 2));
                }

                // 3
                {
                    void* nptr1 = allocator.Reallocate(ptr1, 16, 100);
                    if (nptr1 != ptr1) allocator.Deallocate(ptr1, 16);
                    ptr1 = nptr1;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (512 + 16 + 100));
                    void* nptr2 = allocator.Reallocate(ptr2, 16, 100);
                    if (nptr2 != ptr2) allocator.Deallocate(ptr2, 16);
                    ptr2 = nptr2;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (512 + 100 * 2));
                }

                // 2 + 3
                {
                    void* nptr3 = allocator.Reallocate(ptr3, 512, 16);
                    if (nptr3 != ptr3) allocator.Deallocate(ptr3, 512);
                    ptr3 = nptr3;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (100 * 2 + 16));
                    nptr3 = allocator.Reallocate(ptr3, 16, 100);
                    if (nptr3 != ptr3) allocator.Deallocate(ptr3, 16);
                    ptr3 = nptr3;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (100 * 3));
                }

                // 4 + 5
                {
                    void* nptr1 = allocator.Reallocate(ptr1, 100, 9);
                    if (nptr1 != ptr1) allocator.Deallocate(ptr1, 100);
                    ptr1 = nptr1;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (100 * 2 + 9));
                    nptr1 = allocator.Reallocate(ptr1, 9, 256);
                    if (nptr1 != ptr1) allocator.Deallocate(ptr1, 9);
                    ptr1 = nptr1;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (256 + 100 * 2));
                }

                // 4
                {
                    void* nptr2 = allocator.Reallocate(ptr2, 100, 9);
                    if (nptr2 != ptr2) allocator.Deallocate(ptr2, 100);
                    ptr2 = nptr2;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (256 + 100 + 9));
                    void* nptr3 = allocator.Reallocate(ptr3, 100, 9);
                    if (nptr3 != ptr3) allocator.Deallocate(ptr3, 100);
                    ptr3 = nptr3;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (256 + 9 * 2));
                }

                // 5
                {
                    void* nptr2 = allocator.Reallocate(ptr2, 9, 256);
                    if (nptr2 != ptr2) allocator.Deallocate(ptr2, 9);
                    ptr2 = nptr2;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (256 * 2 + 9));
                    void* nptr3 = allocator.Reallocate(ptr3, 9, 256);
                    if (nptr3 != ptr3) allocator.Deallocate(ptr3, 9);
                    ptr3 = nptr3;
                    REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == (256 * 3));
                }
            }

            allocator.Deallocate(ptr1, 256);
            allocator.Deallocate(ptr2, 256);
            allocator.Deallocate(ptr3, 256);
            REQUIRE(reporter.Size(Memory::Reporter::RT_Used) == 0);
        }
    }

    Memory::Deallocate(&reporter, memory);
    Memory::Virtual::Release(&reporter, virtualMemory);
    
    REQUIRE(reporter.Size(Memory::Reporter::RT_Allocated) == 0);
}