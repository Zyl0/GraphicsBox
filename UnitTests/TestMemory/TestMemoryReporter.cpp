#include "catch2/catch_all.hpp"

#include "Memory/Reporter.h"

TEST_CASE("Memory Reporter ")
{
    SECTION("Construction")
    {
        Memory::Reporter MemoryReporter{"Test reporter"};
        REQUIRE(MemoryReporter.Name() == "Test reporter");
        REQUIRE(MemoryReporter.Limit() == 0);
        REQUIRE(MemoryReporter.Size(Memory::Reporter::RT_Physical) == 0);
        REQUIRE(MemoryReporter.Size(Memory::Reporter::RT_VirtualReserved) == 0);
        REQUIRE(MemoryReporter.Size(Memory::Reporter::RT_VirtualCommited) == 0);
        REQUIRE(MemoryReporter.Size(Memory::Reporter::RT_Used) == 0);
        REQUIRE(MemoryReporter.Size(Memory::Reporter::RT_Allocated) == 0);
    
        Memory::Reporter MemoryReporterBound{"Test reporter With Bounds", 1_KB};
        REQUIRE(MemoryReporter.Name() == "Test reporter With Bounds");
        REQUIRE(MemoryReporter.Limit() == 1_KB);
        REQUIRE(MemoryReporter.Size(Memory::Reporter::RT_Physical) == 0);
        REQUIRE(MemoryReporter.Size(Memory::Reporter::RT_VirtualReserved) == 0);
        REQUIRE(MemoryReporter.Size(Memory::Reporter::RT_VirtualCommited) == 0);
        REQUIRE(MemoryReporter.Size(Memory::Reporter::RT_Used) == 0);
        REQUIRE(MemoryReporter.Size(Memory::Reporter::RT_Allocated) == 0);
    }
    
    SECTION("Reporting")
    {
        Memory::Reporter MemoryReporter{"Test reporter"};
        Memory::Reporter MemoryReporter2{"Test reporter 2"};
        
        MemoryReporter.ReportIncrease(Memory::Reporter::RT_Physical, 2_KB);
        MemoryReporter.ReportIncrease(Memory::Reporter::RT_Used, 1_KB);
        MemoryReporter.ReportIncrease(Memory::Reporter::RT_Used, 1_KB);
        
        REQUIRE(MemoryReporter.Size(Memory::Reporter::RT_Physical) == 2_KB);
        REQUIRE(MemoryReporter.Size(Memory::Reporter::RT_VirtualReserved) == 0);
        REQUIRE(MemoryReporter.Size(Memory::Reporter::RT_VirtualCommited) == 0);
        REQUIRE(MemoryReporter.Size(Memory::Reporter::RT_Used) == 2_KB);
        REQUIRE(MemoryReporter.Size(Memory::Reporter::RT_Allocated) == 2_KB);
        
        MemoryReporter2.ReportIncrease(Memory::Reporter::RT_Physical, 2_KB);
        MemoryReporter2.ReportIncrease(Memory::Reporter::RT_Used, 2_KB);
        REQUIRE(MemoryReporter.Size(Memory::Reporter::RT_Allocated) == 2_KB);
        
        MemoryReporter.ReportIncrease(Memory::Reporter::RT_VirtualReserved, 4_KB);
        MemoryReporter.ReportIncrease(Memory::Reporter::RT_VirtualCommited, 4_KB);
        REQUIRE(MemoryReporter.Size(Memory::Reporter::RT_Used) == 2_KB);
        REQUIRE(MemoryReporter.Size(Memory::Reporter::RT_Allocated) == 6_KB);
        
        MemoryReporter.ReportDecrease(Memory::Reporter::RT_Used, 512);
        REQUIRE(MemoryReporter.Size(Memory::Reporter::RT_Used) == (2_KB - 512));
    }
    
    SECTION("Limits")
    {
        Memory::Reporter MemoryReporterBound{"Test reporter With Bounds", 1_KB};
        REQUIRE(MemoryReporterBound.Limit() == 1_KB);
        
        REQUIRE(MemoryReporterBound.Available() == 0);
        
        MemoryReporterBound.ReportIncrease(Memory::Reporter::RT_Physical, 2_KB);
        REQUIRE(MemoryReporterBound.Available() == 1_KB);
        
        MemoryReporterBound.ReportIncrease(Memory::Reporter::RT_Used, 512);
        REQUIRE(MemoryReporterBound.Available() == 512);
        
        MemoryReporterBound.ReportIncrease(Memory::Reporter::RT_Used, 512);
        REQUIRE(MemoryReporterBound.Available() == 0);
    }
}
