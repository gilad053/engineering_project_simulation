#include "StatsCollector.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <algorithm>

// Constructor
StatsCollector::StatsCollector(int cores)
    : totalCycles(0),
      numCores(cores),
      coreBusyCycles(cores, 0),
      coreLastBusyStart(cores, 0),
      dtcmHits(0),
      cacheHits(0),
      cacheMisses(0),
      mainMemAccesses(0),
      interconnectBusyCycles(0),
      interconnectLastBusyStart(0),
      bankConflicts(0),
      cachePortConflicts(0),
      bankPortConflicts(0),
      intraChipletConflicts(0),
      interChipletConflicts(0),
      currentTime(0) {
}

// Event notification dispatcher
void StatsCollector::onEvent(const Event& e, uint64_t now) {
    currentTime = now;
    
    switch (e.type) {
        case EventType::TaskReady:
            recordTaskReady(e.taskInstanceId, now);
            break;
        case EventType::TaskDispatched:
            recordTaskDispatched(e.taskInstanceId, now);
            break;
        case EventType::TaskDone:
            recordTaskDone(e.taskInstanceId, now);
            break;
        case EventType::ComputeDone:
        case EventType::MemRespDone:
            // Core busy cycles are tracked separately via recordCoreBusy/Idle
            break;
        default:
            // Other events don't require specific tracking
            break;
    }
}

// Record task ready time
void StatsCollector::recordTaskReady(int instanceId, uint64_t time) {
    taskReadyTimes[instanceId] = time;
}

// Record task dispatched time and calculate wait time
void StatsCollector::recordTaskDispatched(int instanceId, uint64_t time) {
    taskDispatchTimes[instanceId] = time;
    
    // Calculate wait time (ready to dispatch)
    auto readyIt = taskReadyTimes.find(instanceId);
    if (readyIt != taskReadyTimes.end()) {
        uint64_t waitTime = time - readyIt->second;
        taskWaitTimes.push_back(waitTime);
    }
}

// Record task done time and calculate latency
void StatsCollector::recordTaskDone(int instanceId, uint64_t time) {
    // Calculate latency (ready to done)
    auto readyIt = taskReadyTimes.find(instanceId);
    if (readyIt != taskReadyTimes.end()) {
        uint64_t latency = time - readyIt->second;
        taskLatencies.push_back(latency);
    }
}

// Record core busy period start
void StatsCollector::recordCoreBusy(int coreId, uint64_t startTime) {
    if (coreId >= 0 && coreId < numCores) {
        coreLastBusyStart[coreId] = startTime;
    }
}

// Record core idle and accumulate busy cycles
void StatsCollector::recordCoreIdle(int coreId, uint64_t endTime) {
    if (coreId >= 0 && coreId < numCores) {
        uint64_t busyDuration = endTime - coreLastBusyStart[coreId];
        coreBusyCycles[coreId] += busyDuration;
    }
}

// Record memory access by tier
void StatsCollector::recordMemoryAccess(MemoryTier tier) {
    switch (tier) {
        case MemoryTier::DTCM:
            dtcmHits++;
            break;
        case MemoryTier::Cache:
            cacheHits++;
            break;
        case MemoryTier::MainMemory:
            cacheMisses++;
            mainMemAccesses++;
            break;
    }
}

// Record conflict
void StatsCollector::recordConflict(ConflictType type, bool interChiplet) {
    switch (type) {
        case ConflictType::BankConflict:
            bankConflicts++;
            break;
        case ConflictType::CachePortConflict:
            cachePortConflicts++;
            break;
        case ConflictType::BankPortConflict:
            bankPortConflicts++;
            break;
    }
    
    // Track chiplet-level conflicts
    if (interChiplet) {
        interChipletConflicts++;
    } else {
        intraChipletConflicts++;
    }
}

// Record interconnect busy period start
void StatsCollector::recordInterconnectBusy(uint64_t startTime) {
    interconnectLastBusyStart = startTime;
}

// Record interconnect idle and accumulate busy cycles
void StatsCollector::recordInterconnectIdle(uint64_t endTime) {
    uint64_t busyDuration = endTime - interconnectLastBusyStart;
    interconnectBusyCycles += busyDuration;
}

// Set total simulation time
void StatsCollector::setTotalCycles(uint64_t cycles) {
    totalCycles = cycles;
}

// Generate console report
void StatsCollector::generateReport(double frequencyGHz) const {
    std::cout << "\n========================================\n";
    std::cout << "       SIMULATION STATISTICS\n";
    std::cout << "========================================\n\n";
    
    // Makespan
    double makespanSeconds = totalCycles / (frequencyGHz * 1e9);
    std::cout << "Makespan:\n";
    std::cout << "  Total Cycles: " << totalCycles << "\n";
    std::cout << "  Time (seconds): " << std::scientific << std::setprecision(6) 
              << makespanSeconds << "\n\n";
    
    // Core utilization
    std::cout << "Core Utilization:\n";
    for (int i = 0; i < numCores; i++) {
        double utilization = totalCycles > 0 ? 
            (100.0 * coreBusyCycles[i]) / totalCycles : 0.0;
        std::cout << "  Core " << i << ": " << std::fixed << std::setprecision(2) 
                  << utilization << "% (" << coreBusyCycles[i] << " cycles)\n";
    }
    
    // Average core utilization
    uint64_t totalBusyCycles = std::accumulate(coreBusyCycles.begin(), 
                                                coreBusyCycles.end(), 0ULL);
    double avgUtilization = (numCores > 0 && totalCycles > 0) ? 
        (100.0 * totalBusyCycles) / (numCores * totalCycles) : 0.0;
    std::cout << "  Average: " << std::fixed << std::setprecision(2) 
              << avgUtilization << "%\n\n";
    
    // Task timing statistics
    std::cout << "Task Statistics:\n";
    std::cout << "  Total Tasks Completed: " << taskLatencies.size() << "\n";
    
    if (!taskLatencies.empty()) {
        double avgLatency = std::accumulate(taskLatencies.begin(), 
                                           taskLatencies.end(), 0.0) / taskLatencies.size();
        std::cout << "  Average Task Latency: " << std::fixed << std::setprecision(2) 
                  << avgLatency << " cycles\n";
    }
    
    if (!taskWaitTimes.empty()) {
        double avgWait = std::accumulate(taskWaitTimes.begin(), 
                                        taskWaitTimes.end(), 0.0) / taskWaitTimes.size();
        std::cout << "  Average Task Wait Time: " << std::fixed << std::setprecision(2) 
                  << avgWait << " cycles\n";
    }
    std::cout << "\n";
    
    // Memory hierarchy statistics
    std::cout << "Memory Hierarchy:\n";
    std::cout << "  DTCM Hits: " << dtcmHits << "\n";
    std::cout << "  Cache Hits: " << cacheHits << "\n";
    std::cout << "  Cache Misses: " << cacheMisses << "\n";
    std::cout << "  Main Memory Accesses: " << mainMemAccesses << "\n";
    
    uint64_t totalMemAccesses = dtcmHits + cacheHits + cacheMisses;
    if (totalMemAccesses > 0) {
        std::cout << "  DTCM Hit Rate: " << std::fixed << std::setprecision(2) 
                  << (100.0 * dtcmHits / totalMemAccesses) << "%\n";
        std::cout << "  Cache Hit Rate: " << std::fixed << std::setprecision(2) 
                  << (100.0 * cacheHits / totalMemAccesses) << "%\n";
    }
    std::cout << "\n";
    
    // Interconnect utilization
    double interconnectUtil = totalCycles > 0 ? 
        (100.0 * interconnectBusyCycles) / totalCycles : 0.0;
    std::cout << "Interconnect:\n";
    std::cout << "  Busy Cycles: " << interconnectBusyCycles << "\n";
    std::cout << "  Utilization: " << std::fixed << std::setprecision(2) 
              << interconnectUtil << "%\n\n";
    
    // Conflict statistics
    std::cout << "Conflicts:\n";
    std::cout << "  Bank Conflicts: " << bankConflicts << "\n";
    std::cout << "  Cache Port Conflicts: " << cachePortConflicts << "\n";
    std::cout << "  Bank Port Conflicts: " << bankPortConflicts << "\n";
    std::cout << "  Intra-Chiplet Conflicts: " << intraChipletConflicts << "\n";
    std::cout << "  Inter-Chiplet Conflicts: " << interChipletConflicts << "\n";
    
    std::cout << "\n========================================\n\n";
}

// Write JSON output
void StatsCollector::writeJSON(const std::string& filepath, double frequencyGHz) const {
    std::ofstream outFile(filepath);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open " << filepath << " for writing\n";
        return;
    }
    
    double makespanSeconds = totalCycles / (frequencyGHz * 1e9);
    
    outFile << "{\n";
    outFile << "  \"makespan_cycles\": " << totalCycles << ",\n";
    outFile << "  \"makespan_seconds\": " << std::scientific << std::setprecision(9) 
            << makespanSeconds << ",\n";
    
    // Core utilization array
    outFile << "  \"core_utilization\": [";
    for (int i = 0; i < numCores; i++) {
        double utilization = totalCycles > 0 ? 
            (static_cast<double>(coreBusyCycles[i]) / totalCycles) : 0.0;
        outFile << std::fixed << std::setprecision(4) << utilization;
        if (i < numCores - 1) outFile << ", ";
    }
    outFile << "],\n";
    
    // Core busy cycles array
    outFile << "  \"core_busy_cycles\": [";
    for (int i = 0; i < numCores; i++) {
        outFile << coreBusyCycles[i];
        if (i < numCores - 1) outFile << ", ";
    }
    outFile << "],\n";
    
    // Average core utilization
    uint64_t totalBusyCycles = std::accumulate(coreBusyCycles.begin(), 
                                                coreBusyCycles.end(), 0ULL);
    double avgUtilization = (numCores > 0 && totalCycles > 0) ? 
        (static_cast<double>(totalBusyCycles) / (numCores * totalCycles)) : 0.0;
    outFile << "  \"avg_core_utilization\": " << std::fixed << std::setprecision(4) 
            << avgUtilization << ",\n";
    
    // Task statistics
    outFile << "  \"total_tasks_completed\": " << taskLatencies.size() << ",\n";
    
    double avgLatency = !taskLatencies.empty() ? 
        std::accumulate(taskLatencies.begin(), taskLatencies.end(), 0.0) / taskLatencies.size() : 0.0;
    outFile << "  \"avg_task_latency_cycles\": " << std::fixed << std::setprecision(2) 
            << avgLatency << ",\n";
    
    double avgWait = !taskWaitTimes.empty() ? 
        std::accumulate(taskWaitTimes.begin(), taskWaitTimes.end(), 0.0) / taskWaitTimes.size() : 0.0;
    outFile << "  \"avg_task_wait_cycles\": " << std::fixed << std::setprecision(2) 
            << avgWait << ",\n";
    
    // Memory accesses
    outFile << "  \"memory_accesses\": {\n";
    outFile << "    \"dtcm_hits\": " << dtcmHits << ",\n";
    outFile << "    \"cache_hits\": " << cacheHits << ",\n";
    outFile << "    \"cache_misses\": " << cacheMisses << ",\n";
    outFile << "    \"main_memory_accesses\": " << mainMemAccesses << "\n";
    outFile << "  },\n";
    
    // Interconnect utilization
    double interconnectUtil = totalCycles > 0 ? 
        (static_cast<double>(interconnectBusyCycles) / totalCycles) : 0.0;
    outFile << "  \"interconnect_busy_cycles\": " << interconnectBusyCycles << ",\n";
    outFile << "  \"interconnect_utilization\": " << std::fixed << std::setprecision(4) 
            << interconnectUtil << ",\n";
    
    // Conflicts
    outFile << "  \"conflicts\": {\n";
    outFile << "    \"bank_conflicts\": " << bankConflicts << ",\n";
    outFile << "    \"cache_port_conflicts\": " << cachePortConflicts << ",\n";
    outFile << "    \"bank_port_conflicts\": " << bankPortConflicts << ",\n";
    outFile << "    \"intra_chiplet_conflicts\": " << intraChipletConflicts << ",\n";
    outFile << "    \"inter_chiplet_conflicts\": " << interChipletConflicts << "\n";
    outFile << "  }\n";
    
    outFile << "}\n";
    
    outFile.close();
    std::cout << "Statistics written to " << filepath << "\n";
}
