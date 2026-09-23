#include "Memory/Reporter.h"

#include <limits>

#include "Shared/Assertion.h"

namespace Memory
{
    size_t Reporter::Size(ReportType MemType) const
    {
        switch(MemType)
        {
        case RT_Physical:
            return m_Size;
        case RT_VirtualReserved:
            return m_VirtualSize;
        case RT_VirtualCommited:
            return m_CommitedSize;
        case RT_Used:
            return m_Used;
        case RT_Allocated:
            return m_Size + m_CommitedSize;

        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Cannot retrieve pool size of such memory type")
        }
    }

    int64_t Reporter::Available() const
    {
        return m_Limit > 0 ? 
            (int64_t)std::min(m_Limit, m_Size + m_VirtualSize) - (int64_t)m_Used :
            (int64_t)std::min(m_Size + m_VirtualSize, (size_t)std::numeric_limits<int64_t>::max()) - (int64_t)m_Used;
    }

    void Reporter::ReportIncrease(ReportType type, size_t Size)
    {
        switch(type)
        {
        case RT_Physical:
            m_Size += Size;          break;
        case RT_VirtualReserved:
            m_VirtualSize += Size;   break;
        case RT_VirtualCommited:
            m_CommitedSize += Size;  break;
        case RT_Used:
            AssertOrErrorF((m_Used + Size) <= (m_Size + m_CommitedSize), "Not enough memory allocated in this pool (%llu / %llu bytes) to fit this memory (%llu bytes)", m_Used + Size, m_Size + m_CommitedSize, Size)
            AssertOrErrorF( m_Limit == 0 || (m_Used + Size) <= (m_Limit), "Memory allocation of (%llu bytes) is out of budget in this pool (%llu / %llu bytes [limited] )", Size, m_Used + Size, m_Limit)
            m_Used += Size;          break;
            
        case RT_Allocated:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Cannot report pool size of such memory type")
        }
    }

    void Reporter::ReportDecrease(ReportType type, size_t Size)
    {
        switch(type)
        {
        case RT_Physical:
            AssertOrErrorF(
                m_Size >= Size, 
                "Tried to free more memory (%llu bytes) than actually reported in the pool %.*s (%llu bytes)", 
                Size, m_Name.size(), m_Name.data(), m_Size)
            m_Size -= Size;          
            break;

        case RT_VirtualReserved:
            AssertOrErrorF(
                m_VirtualSize >= Size, 
                "Tried to free more virtual memory (%llu bytes) than actually reported in the pool %.*s (%llu bytes)", 
                Size, m_Name.size(), m_Name.data(), m_VirtualSize)
            AssertOrErrorF(
                m_CommitedSize <= (m_VirtualSize - Size), 
                "Tried to free virtual memory that is most likely still commited from pool %.*s", 
                m_Name.size(), m_Name.data())
            m_VirtualSize -= Size;   
            break;

        case RT_VirtualCommited:
            AssertOrErrorF(
                m_CommitedSize >= Size, 
                "Tried to de commit memory (%llu bytes) than actually reported in the pool %.*s (%llu bytes)", 
                Size, m_Name.size(), m_Name.data(), m_CommitedSize)
            m_CommitedSize -= Size;  
            break;

        case RT_Used:
            AssertOrErrorF(
                m_Used >= Size, 
                "Tried to free de commit memory (%llu bytes) than actually reported in the pool %.*s (%llu bytes)", 
                Size, m_Name.size(), m_Name.data(), m_Used)
            m_Used -= Size;
            break;
            
        case RT_Allocated:
        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Cannot report pool size of such memory type")
        }
    }
}
