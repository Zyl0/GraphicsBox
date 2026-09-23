Allocators
==========
[Home](../Documentation.md)

Header: ```<Modeling/Mesh.h>```

## General Interface
Each allocator has the following symbols:
```c++
struct Allocator
{
    // Memory pool the allocator report its allocations to
    using Pool = //...

    // Allocate some memory of a given size
    void* Allocate(size_t Size);

    // Allocate some memory of a given size respecting a given memory alignment
    void* AllocateAligned(size_t Size, size_t Alignment);

    // Reallocate a given memory from this allocator to a new size
    void* Reallocate(void* OldPtr, size_t OldSize, size_t NewSize);

    // Reallocate a given memory from this allocator to a new size respecting a given alignment
    void* ReallocateAligned(void* OldPtr, size_t OldSize, size_t NewSize, size_t Alignment);

    // Release a given memory from this allocator
    void Deallocate(void* ptr, size_t Size);

    // Release a given memory from this allocator that was aligned
    void DeallocateAligned(void* ptr, size_t Size, size_t Alignment);

    void DeallocateAll(); // Optionnal
}
```

## System
The system allocator is just an allocator that directly calls the platform specific alligned memory allocation functions. The ```void DeallocateAll();``` function is disabled.

## Linear
Implementation from [gingerBill](https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/)'s page over Arena/Linear alloactors. A linear allocator is a very low, constant cost allocator. It retains only the data of the latest allocation made. This means, that it can resize and free only the last allocation. Once a new allocation is made, the previously allocated data cannot be changed. Resizing memory that is not the last allocation made over this allocator will become new allocation, and so, become the last allocated memory.

## Stack
## Pool
## Free List
