#ifndef CORE_HPP
#define CORE_HPP

#include "Types.hpp"
#include "EventQueue.hpp"
#include "TaskGraph.hpp"
#include <cstdint>

class Core {
public:
    Core(int coreId);
    
    // Check if core is idle
    bool isIdle() const;
    
    // Execute an operation from a task instance
    void executeOp(const Op& op, int instanceId, uint64_t currentTime, 
                   EventQueue& eventQueue, TaskGraph& taskGraph);
    
    // Complete the current operation and advance to next
    void completeOp(uint64_t currentTime, EventQueue& eventQueue, TaskGraph& taskGraph);
    
    // Getters
    int getCoreId() const { return coreId; }
    int getCurrentInstanceId() const { return currentInstanceId; }
    uint64_t getBusySince() const { return busySince; }
    
private:
    int coreId;
    bool busy;
    int currentInstanceId;
    uint64_t busySince;
    
    // Helper methods for operation execution
    void handleComputeOp(const Op& op, int instanceId, uint64_t currentTime, EventQueue& eventQueue);
    void handleMemoryOp(const Op& op, int instanceId, uint64_t currentTime, EventQueue& eventQueue);
};

#endif // CORE_HPP
