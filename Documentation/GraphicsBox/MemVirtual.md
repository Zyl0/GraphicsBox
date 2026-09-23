Virtual Memory
==============
[Home](../Documentation.md)

Header: ```<Memory/VirtualMemory.h>```

 - [Reserve](#reserve)
 - [Commit](#commit)
 - [DeCommit](#decommit)
 - [Release](#release)
 - [Using memory reporter](#using-memory-reporter)

## Reserve

```c++
Memory::Block Memory::Virtual::Reserve(size_t Size);
```

## Commit

```c++
void* Memory::Virtual::Commit(const Memory::Block& Memory, size_t Offset, size_t Size);
```

## DeCommit

```c++
void Memory::Virtual::DeCommit(const Memory::Block& Memory, size_t Offset, size_t Size);
```

## Release

```c++
void Memory::Virtual::Release(Memory::Block& Memory);
```

## Using memory reporter

You can pass a [```Memory::Reporter```](MemReporter.md) by pointer for memory tracking of reserve/commit/decommit/release calls.

```c++
Memory::Block Memory::Virtual::Reserve(Memory::Reporter* Reporter, size_t Size);

void* Memory::Virtual::Commit(Memory::Reporter* Reporter, const Memory::Block& Memory, size_t Offset, size_t Size);

void Memory::Virtual::DeCommit(Memory::Reporter* Reporter, const Memory::Block& Memory, size_t Offset, size_t Size);

void Memory::Virtual::Release(Memory::Reporter* Reporter, Memory::Block& Memory);
```