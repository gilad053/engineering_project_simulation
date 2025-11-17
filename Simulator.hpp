#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include "Types.hpp"
#include "EventQueue.hpp"
#include "Config.hpp"
#include "TaskGraph.hpp"
#include "Scheduler.hpp"
#include "Core.hpp"
#include "MemorySystem.hpp"
#include "StatsCollector.hpp"
#include <vector>
#include <memory>
#include <string>
#include <cstdint>

/**
 * Simulator class
 * Top-level discrete-event simulation engine that manages global time and coordinates all components
 */
class Simulator {
private:
    // Current simulation time in cycles
    uint64_t now;
    
    // Core components
    EventQueue eventQueue;
    Config config;
    TaskGraph taskGraph;
    std::unique_ptr<Scheduler> scheduler;
    std::vector<Core> cores;
    std::unique_ptr<MemorySystem> memorySystem;
    std::unique_ptr<StatsCollector> statsCollector;

public:
    /**
     * Constructor
     */
    Simulator();
    
    /**
     * Initialize the simulator with configuration and task files
     * @param configPath Path to configuration JSON file
     * @param tasksPath Path to tasks CSV file
     * @param opsPath Path to operations CSV file
     */
    void initialize(const std::string& configPath, 
                   const std::string& tasksPath, 
                   const std::string& opsPath);
    
    /**
     * Run the simulation until completion
     */
    void run();
    
    /**
     * Get current simulation time
     * @return Current time in cycles
     */
    uint64_t getCurrentTime() const;

private:
    // Event handlers
    void handleTaskReady(const Event& e);
    void handleTaskDispatched(const Event& e);
    void handleComputeDone(const Event& e);
    void handleMemReqIssued(const Event& e);
    void handleMemRespDone(const Event& e);
    void handleTaskDone(const Event& e);
    
    // Helper method to schedule events
    void scheduleEvent(const Event& e);
    
    // Static callback for MemorySystem to schedule events
    static void eventSchedulerCallback(Event event, void* context);
};

#endif // SIMULATOR_HPP
