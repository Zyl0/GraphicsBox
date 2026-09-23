Memory Alloc Functions
======================
[Home](../Documentation.md)

Header: ```<Memory/MemAlloc.h>```

 - [Allocate](#allocate)
 - [Reallocate](#reallocate)
 - [Free](#free)
 - [Using memory reporter](#using-memory-reporter)

## Allocate

```c++
Memory::Block Memory::Allocate(size_t size);

Memory::Block Memory::AllocateAligned(size_t Size, size_t Alignment);
```

## Reallocate

```c++
void Memory::Reallocate(Memory::Block& block, size_t NewSize)

void Memory::ReallocateAligned(Memory::Block& block, size_t NewSize, size_t Alignment);
```

## Free

```c++
void Memory::Deallocate(Memory::Block& block);

void Memory::DeallocateAligned(Memory::Block& block, size_t Alignment);
```

## Using memory reporter

You can pass a [```Memory::Reporter```](MemReporter.md) by pointer for memory tracking of allocations/reallocations/free calls.

```c++
Memory::Block Memory::Allocate(Memory::Reporter* Reporter, size_t size);

Memory::Block Memory::AllocateAligned(Memory::Reporter* Reporter, size_t Size, size_t Alignment);

void Memory::Reallocate(Memory::Reporter* Reporter, Memory::Block& block, size_t NewSize);

void Memory::ReallocateAligned(Memory::Reporter* Reporter, Memory::Block& block, size_t NewSize, size_t Alignment);

void Memory::Deallocate(Memory::Reporter* Reporter, Memory::Block& block);

void Memory::DeallocateAligned(Memory::Reporter* Reporter, Memory::Block& block, size_t Alignment);
```