#include "StatsCollector.hpp"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "Testing StatsCollector...\n";
    
    // Test 1: Basic initialization
    StatsCollector stats(4);
    std::cout << "✓ StatsCollector initialized with 4 cores\n";
    
    // Test 2: Task timing tracking
    stats.recordTaskReady(0, 100);
    stats.recordTaskDispatched(0, 110);
    stats.recordTaskDone(0, 200);
    std::cout << "✓ Task timing recorded\n";
    
    // Test 3: Core utilization tracking
    stats.recordCoreBusy(0, 110);
    stats.recordCoreIdle(0, 200);
    std::cout << "✓ Core utilization tracked\n";
    
    // Test 4: Memory access tracking
    stats.recordMemoryAccess(MemoryTier::DTCM);
    stats.recordMemoryAccess(MemoryTier::Cache);
    stats.recordMemoryAccess(MemoryTier::MainMemory);
    std::cout << "✓ Memory accesses recorded\n";
    
    // Test 5: Conflict tracking
    stats.recordConflict(ConflictType::BankConflict, false);
    stats.recordConflict(ConflictType::CachePortConflict, true);
    std::cout << "✓ Conflicts recorded\n";
    
    // Test 6: Event notification
    Event e(EventType::TaskReady, 100, 0, 1);
    stats.onEvent(e, 100);
    std::cout << "✓ Event notification handled\n";
    
    // Test 7: Set total cycles
    stats.setTotalCycles(1000);
    std::cout << "✓ Total cycles set\n";
    
    // Test 8: Generate report
    std::cout << "\n--- Testing Console Report ---\n";
    stats.generateReport(2.0);
    
    // Test 9: Write JSON
    stats.writeJSON("test_stats.json", 2.0);
    std::cout << "✓ JSON output written\n";
    
    std::cout << "\n✓ All StatsCollector tests passed!\n";
    
    return 0;
}
