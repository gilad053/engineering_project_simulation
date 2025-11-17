#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "Types.hpp"
#include "TaskGraph.hpp"
#include <deque>
#include <vector>

class Scheduler {
public:
    Scheduler(SchedulingPolicy policy, int numCores, TaskGraph* taskGraph);
    
    // Add a task instance to the ready queue
    void addReady(int instanceId);
    
    // Select the next instance to dispatch based on policy
    int selectNextInstance();
    
    // Find an idle core using round-robin
    int selectIdleCore();
    
    // Dispatch an instance to a core
    void dispatch(int instanceId, int coreId, uint64_t currentTime);
    
    // Mark a core as idle
    void releaseCore(int coreId);
    
    // Check if there are ready instances
    bool hasReadyInstances() const { return !readyQueue.empty(); }
    
    // Check if a core is idle
    bool isCoreIdle(int coreId) const { return coreIdle[coreId]; }
    
private:
    SchedulingPolicy policy;
    std::deque<int> readyQueue;      // Queue of ready instance IDs
    std::vector<bool> coreIdle;      // Tracks which cores are available
    int nextCoreRoundRobin;          // For round-robin core selection
    TaskGraph* taskGraph;            // Pointer to task graph for accessing instances
};

#endif // SCHEDULER_HPP
