Memory Reporter
===============
[Home](../Documentation.md)

Header: ```<Memory/Reporter.h>```

The ```Memory::Reporter``` object is a memory usage counter that can be passed to memory allocation tools. The ```Memory::Reporter``` tracks allocated memory through malloc/realloc/free functions, virtual memory reserve/commit/decommit/release functions, and memory usage.

It is possible to set a limit to used memory reported. The reporter will throw an error if the limit is exeeded. It is possible to track from there how many memory remaining is available. This limit concerns used memory, but does not consern system allocations nor virtual allocations.

```c++
    class Reporter
    {
    public:
        enum ReportType : uint8_t
        {
            // Report the allocation of memory through systems allocator (malloc)
            RT_Physical = 0,
            
            // Report the allocation of virtual memory (reserve/commit of memory pages)
            RT_VirtualReserved,
            RT_VirtualCommited,
            
            // Report memory usage
            RT_Used,
            
            // Used to get the total allocated size
            // Do not use to report any memory increase or decrease
            RT_Allocated
        };
        
        Reporter(std::string_view name, size_t limit = 0);
        
        size_t Limit() const;
        size_t Size(ReportType MemType) const;
        int64_t Available() const;

        void SetLimit(size_t Limit);
        void UnsetLimit();
        
        void ReportIncrease(ReportType type, size_t Size);
        void ReportDecrease(ReportType type, size_t Size);
        
        std::string_view Name() const;
    };
```