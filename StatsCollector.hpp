#ifndef STATSCOLLECTOR_HPP
#define STATSCOLLECTOR_HPP

#include "Types.hpp"
#include <vector>
#include <map>
#include <cstdint>
#include <string>

// Memory tier types for tracking
enum class MemoryTier {
    DTCM,
    Cache,
    MainMemory
};

// Conflict types for tracking
enum class ConflictType {
    BankConflict,
    CachePortConflict,
    BankPortConflict
};

// StatsCollector class for tracking simulation metrics
class StatsCollector {
private:
    // Makespan
    uint64_t totalCycles;
    
    // Per-core utilization tracking
    int numCores;
    std::vector<uint64_t> coreBusyCycles;
    std::vector<uint64_t> coreLastBusyStart;  // Track when core became busy
    
    // Task timing tracking
    std::map<int, uint64_t> taskReadyTimes;      // instanceId -> ready time
    std::map<int, uint64_t> taskDispatchTimes;   // instanceId -> dispatch time
    std::vector<uint64_t> taskLatencies;         // ready-to-done times
    std::vector<uint64_t> taskWaitTimes;         // ready-to-dispatch times
    
    // Memory tier counters
    uint64_t dtcmHits;
    uint64_t cacheHits;
    uint64_t cacheMisses;
    uint64_t mainMemAccesses;
    
    // Interconnect utilization
    uint64_t interconnectBusyCycles;
    uint64_t interconnectLastBusyStart;
    
    // Conflict counters
    uint64_t bankConflicts;
    uint64_t cachePortConflicts;
    uint64_t bankPortConflicts;
    uint64_t intraChipletConflicts;
    uint64_t interChipletConflicts;
    
    // Current simulation time (for tracking busy periods)
    uint64_t currentTime;

public:
    // Constructor
    explicit StatsCollector(int cores);
    
    // Event notification handlers
    void onEvent(const Event& e, uint64_t now);
    
    // Task timing recording
    void recordTaskReady(int instanceId, uint64_t time);
    void recordTaskDispatched(int instanceId, uint64_t time);
    void recordTaskDone(int instanceId, uint64_t time);
    
    // Core utilization tracking
    void recordCoreBusy(int coreId, uint64_t startTime);
    void recordCoreIdle(int coreId, uint64_t endTime);
    
    // Memory access tracking
    void recordMemoryAccess(MemoryTier tier);
    
    // Conflict tracking
    void recordConflict(ConflictType type, bool interChiplet = false);
    
    // Interconnect utilization tracking
    void recordInterconnectBusy(uint64_t startTime);
    void recordInterconnectIdle(uint64_t endTime);
    
    // Set total simulation time
    void setTotalCycles(uint64_t cycles);
    
    // Output methods
    void generateReport(double frequencyGHz) const;
    void writeJSON(const std::string& filepath, double frequencyGHz) const;
};

#endif // STATSCOLLECTOR_HPP
