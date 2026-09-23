Allocators
==========
[Home](../Documentation.md)

Header: ```<Memory/Allocators.h>```

 - [General Interface](#general-interface)
 - [System Allocator](#system)
 - [Linear Allocator](#linear)
 - [Stack Allocator](#stack)
 - [Pool Allocator](#pool)

## General Interface
Each allocator has the following symbols:
```c++
struct Allocator
{
    // Allocate some memory of a given size
    void* Allocate(size_t Size);

    // Allocate some memory of a given size respecting a given memory alignment
    void* AllocateAligned(size_t Size, size_t Alignment);

    // Reallocate a given memory from this allocator to a new size
    void* Reallocate(void* OldPtr, size_t OldSize, size_t NewSize);

    // Reallocate a given memory from this allocator to a new size respecting a given alignment
    // If the memory need to be moved, this does not deallocate previous memory, nor perform any move operation. You can safely check between the returned pointer and the old pointer to see if move and deallocation operations are needed.
    void* ReallocateAligned(void* OldPtr, size_t OldSize, size_t NewSize, size_t Alignment);

    // Release a given memory from this allocator
    void Deallocate(void* ptr, size_t Size);

    // Release a given memory from this allocator that was aligned
    void DeallocateAligned(void* ptr, size_t Size, size_t Alignment);
}
```
Except from system allocator, all allocators takes a memory block as well as it's memory origin type. If the memory is virtual, it is considered as reserved but not commited, anything else is considered regular memory.

The workflow using [regular memory](MemAlloc.md) allocation is:
```c++
Memory::Block memory = Memory::Allocate(4_KB);

// Memory usage lifetime
{
    SomeAllocator allocator{memory, Memory::AT_Physical};

    // ...
}

Memory::Deallocate(memory);
```

The workflow using [virtual memory](MemAlloc.md) allocation is:
```c++
Memory::Block virtualMemory = Memory::Virtual::Reserve(&reporter, 16_KB);

// Memory usage lifetime
{
    SomeAllocator allocator{virtualMemory, Memory::AT_Virtual};

    // ...
}

Memory::Virtual::Release(virtualMemory);
```
All allocator can take an optionnal [```Memory::Reporter```](MemReporter.md) in it's contructor parameters for memory usage tracking. 

## System
The system allocator is just an allocator that directly calls the c standard allocation functions and platform specific alligned memory allocation functions. 

## Linear
Implementation from [gingerBill - Memory Allocation Strategies - Part 2 - Linear/Arena Allocators](https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/). 

A linear allocator is a very low, constant cost allocator. It retains only the data of the latest allocation made. This means, that it can resize and free only the last allocation. Once a new allocation is made, the previously allocated data cannot be changed. Resizing memory that is not the last allocation made over this allocator will become new allocation, and so, become the last allocated memory.

## Stack
Implementation from [gingerBill - Memory Allocation Strategies - Part 3 - Stack Allocators](https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-003/). 

A stack allocator reproduces the behaviour of a stack LIFO. Allocation expects to be made in a stack to allocated and deallocate with a constant cost.

Modification. To allow for reallocation and deallocation of elements that are not at the top of the stack. The header stored in front of an allocated region stores a value that precises if the allocation has been moved or release from here. If a deallocation happens the value is just flagged, so when deallocating the region on top of the stack, if the region bellow is flagged, deallocate it as well, until the next region on top is in use. When performing a reallocation, always move the memory.

## Pool
Implementation from [gingerBill - Memory Allocation Strategies - Part 4 - Pool Allocators](https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-004/). 

A pool allocator cut the memory in chuncks that it distributes over the allocation. A free list exist along side the memory to look for free space when allocating before extening the used size of the memory pool.

Modification: Pool allocation can be bigger than a single pool cell. 

## Free List
TODO implement.