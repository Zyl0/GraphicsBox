#pragma once

#include <string_view>

#include "Memory/Types.h"

namespace Memory
{
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
            
            RT_Allocated // TODO see if used
        };
        
        Reporter(std::string_view name, size_t limit = 0): m_Name(name), m_Limit(limit) {}
        
        static bool MemoryTypeIsAllocatedType(ReportType type)
        {
            return type == RT_Physical || type == RT_VirtualCommited;
        }
        
        size_t Limit() const {return m_Limit;}
        size_t Size(ReportType MemType) const;
        int64_t Available() const;

        void SetLimit(size_t Limit) {m_Limit = Limit;}
        void UnsetLimit() {m_Limit = 0;}
        
        void ReportIncrease(ReportType type, size_t Size);
        void ReportDecrease(ReportType type, size_t Size);
        
        std::string_view Name() const {return m_Name;}
        
    private:
        std::string_view m_Name;
        size_t m_Size = 0;
        size_t m_VirtualSize = 0;
        size_t m_CommitedSize = 0;
        size_t m_Limit = 0;
        size_t m_Used = 0;
    };
}