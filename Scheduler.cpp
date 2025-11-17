#include "Scheduler.hpp"
#include <algorithm>
#include <stdexcept>

Scheduler::Scheduler(SchedulingPolicy policy, int numCores, TaskGraph* taskGraph)
    : policy(policy), coreIdle(numCores, true), nextCoreRoundRobin(0), taskGraph(taskGraph) {
    if (taskGraph == nullptr) {
        throw std::invalid_argument("TaskGraph pointer cannot be null");
    }
}

// Subtask 6.1: Add instance to ready queue
void Scheduler::addReady(int instanceId) {
    readyQueue.push_back(instanceId);
}

// Subtask 6.2: Select next instance based on scheduling policy
int Scheduler::selectNextInstance() {
    if (readyQueue.empty()) {
        return -1;  // No ready instances
    }
    
    int selectedInstanceId = -1;
    
    switch (policy) {
        case SchedulingPolicy::FIFO:
            // FIFO: Take the front of the queue
            selectedInstanceId = readyQueue.front();
            readyQueue.pop_front();
            break;
            
        case SchedulingPolicy::RoundRobin:
            // RoundRobin: Also FIFO for task selection, core selection handles round-robin
            selectedInstanceId = readyQueue.front();
            readyQueue.pop_front();
            break;
            
        case SchedulingPolicy::ShortestOpsFirst:
            // ShortestOpsFirst: Find instance with fewest remaining operations
            {
                int minOpsIndex = 0;
                int minOpsCount = INT_MAX;
                
                for (size_t i = 0; i < readyQueue.size(); ++i) {
                    int instanceId = readyQueue[i];
                    const TaskInstance& instance = taskGraph->getInstance(instanceId);
                    const std::vector<Op>& ops = taskGraph->getOps(instance.taskId);
                    int remainingOps = ops.size() - instance.currentOpIndex;
                    
                    if (remainingOps < minOpsCount) {
                        minOpsCount = remainingOps;
                        minOpsIndex = i;
                    }
                }
                
                selectedInstanceId = readyQueue[minOpsIndex];
                readyQueue.erase(readyQueue.begin() + minOpsIndex);
            }
            break;
            
        default:
            throw std::runtime_error("Unknown scheduling policy");
    }
    
    return selectedInstanceId;
}

// Subtask 6.3: Select an idle core using round-robin
int Scheduler::selectIdleCore() {
    int numCores = coreIdle.size();
    
    // Try to find an idle core starting from nextCoreRoundRobin
    for (int i = 0; i < numCores; ++i) {
        int coreId = (nextCoreRoundRobin + i) % numCores;
        if (coreIdle[coreId]) {
            nextCoreRoundRobin = (coreId + 1) % numCores;  // Update for next selection
            return coreId;
        }
    }
    
    return -1;  // No idle core available
}

// Subtask 6.3: Dispatch instance to core
void Scheduler::dispatch(int instanceId, int coreId, uint64_t currentTime) {
    if (coreId < 0 || coreId >= static_cast<int>(coreIdle.size())) {
        throw std::out_of_range("Invalid core ID");
    }
    
    if (!coreIdle[coreId]) {
        throw std::runtime_error("Cannot dispatch to busy core");
    }
    
    // Mark core as busy
    coreIdle[coreId] = false;
    
    // Update instance dispatch time
    TaskInstance& instance = taskGraph->getInstance(instanceId);
    instance.dispatchTime = currentTime;
}

// Subtask 6.3: Release core and mark as idle
void Scheduler::releaseCore(int coreId) {
    if (coreId < 0 || coreId >= static_cast<int>(coreIdle.size())) {
        throw std::out_of_range("Invalid core ID");
    }
    
    coreIdle[coreId] = true;
}
