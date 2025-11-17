#include "Core.hpp"
#include <stdexcept>

// Constructor
Core::Core(int coreId)
    : coreId(coreId), busy(false), currentInstanceId(-1), busySince(0) {
}

// Subtask 7.1: Check if core is idle
bool Core::isIdle() const {
    return !busy;
}

// Subtask 7.2: Execute an operation from a task instance
void Core::executeOp(const Op& op, int instanceId, uint64_t currentTime, 
                     EventQueue& eventQueue, TaskGraph& taskGraph) {
    if (busy) {
        throw std::runtime_error("Cannot execute operation on busy core");
    }
    
    // Mark core as busy
    busy = true;
    currentInstanceId = instanceId;
    busySince = currentTime;
    
    // Route to appropriate handler based on operation type
    if (op.type == OpType::Compute) {
        handleComputeOp(op, instanceId, currentTime, eventQueue);
    } else if (op.type == OpType::Memory) {
        handleMemoryOp(op, instanceId, currentTime, eventQueue);
    } else {
        throw std::runtime_error("Unknown operation type");
    }
}

// Subtask 7.2: Handle compute operation
void Core::handleComputeOp(const Op& op, int instanceId, uint64_t currentTime, 
                           EventQueue& eventQueue) {
    // Schedule ComputeDone event at now + op.cycles
    Event computeDoneEvent(EventType::ComputeDone, currentTime + op.cycles, 
                          coreId, instanceId);
    eventQueue.push(computeDoneEvent);
}

// Subtask 7.2: Handle memory operation
void Core::handleMemoryOp(const Op& op, int instanceId, uint64_t currentTime, 
                          EventQueue& eventQueue) {
    // Schedule MemReqIssued event immediately
    Event memReqEvent(EventType::MemReqIssued, currentTime, coreId, instanceId, 
                     op.address, static_cast<int>(op.rw));
    eventQueue.push(memReqEvent);
}

// Subtask 7.3: Complete operation and advance to next
void Core::completeOp(uint64_t currentTime, EventQueue& eventQueue, TaskGraph& taskGraph) {
    if (!busy || currentInstanceId == -1) {
        throw std::runtime_error("Cannot complete operation on idle core");
    }
    
    // Get the current task instance
    TaskInstance& instance = taskGraph.getInstance(currentInstanceId);
    
    // Advance to next operation
    instance.currentOpIndex++;
    
    // Get the operations for this task
    const std::vector<Op>& ops = taskGraph.getOps(instance.taskId);
    
    // Check if all operations are complete
    if (instance.currentOpIndex >= static_cast<int>(ops.size())) {
        // All operations complete - schedule TaskDone event
        instance.doneTime = currentTime;
        Event taskDoneEvent(EventType::TaskDone, currentTime, coreId, currentInstanceId);
        eventQueue.push(taskDoneEvent);
        
        // Reset core to idle state
        busy = false;
        currentInstanceId = -1;
    } else {
        // More operations to execute - execute the next one
        const Op& nextOp = ops[instance.currentOpIndex];
        
        // Temporarily mark as not busy so executeOp can proceed
        busy = false;
        int instanceId = currentInstanceId;
        currentInstanceId = -1;
        
        // Execute the next operation
        executeOp(nextOp, instanceId, currentTime, eventQueue, taskGraph);
    }
}
