#include "TaskGraph.hpp"
#include "CSVParser.hpp"
#include <sstream>
#include <stdexcept>
#include <algorithm>

TaskGraph::TaskGraph() {
}

void TaskGraph::loadFromCSV(const std::string& tasksPath, const std::string& opsPath) {
    // Parse tasks.csv
    auto taskRows = CSVParser::parseCSV(tasksPath);
    
    // Create Task objects
    for (const auto& row : taskRows) {
        Task task;
        task.id = std::stoi(row.at("id"));
        task.name = row.at("name");
        task.executions = std::stoi(row.at("executions"));
        
        // Parse dependencies (semicolon-separated)
        std::string depsStr = row.at("deps");
        task.dependencies = parseDependencies(depsStr);
        
        tasks.push_back(task);
    }
    
    // Parse ops.csv
    auto opsRows = CSVParser::parseCSV(opsPath);
    
    // Group operations by task_id and sort by seq_idx
    std::map<int, std::vector<std::pair<int, Op>>> taskOpsMap;
    
    for (const auto& row : opsRows) {
        int taskId = std::stoi(row.at("task_id"));
        int seqIdx = std::stoi(row.at("seq_idx"));
        
        Op op;
        std::string typeStr = row.at("type");
        if (typeStr == "compute") {
            op.type = OpType::Compute;
            op.cycles = std::stoi(row.at("cycles"));
            op.address = 0;
            op.rw = AccessType::Read;  // Default, not used for compute
        } else if (typeStr == "mem") {
            op.type = OpType::Memory;
            op.cycles = 0;
            
            // Parse address (hex format)
            std::string addrStr = row.at("address");
            if (!addrStr.empty()) {
                op.address = std::stoull(addrStr, nullptr, 16);
            } else {
                op.address = 0;
            }
            
            // Parse read/write
            std::string rwStr = row.at("rw");
            if (rwStr == "R" || rwStr == "r") {
                op.rw = AccessType::Read;
            } else if (rwStr == "W" || rwStr == "w") {
                op.rw = AccessType::Write;
            } else {
                throw std::runtime_error("Invalid rw field: " + rwStr);
            }
        } else {
            throw std::runtime_error("Invalid operation type: " + typeStr);
        }
        
        taskOpsMap[taskId].push_back({seqIdx, op});
    }
    
    // Sort operations by seq_idx and assign to tasks
    for (auto& task : tasks) {
        if (taskOpsMap.find(task.id) != taskOpsMap.end()) {
            auto& opsWithIdx = taskOpsMap[task.id];
            
            // Sort by seq_idx
            std::sort(opsWithIdx.begin(), opsWithIdx.end(),
                     [](const auto& a, const auto& b) { return a.first < b.first; });
            
            // Extract ops
            for (const auto& pair : opsWithIdx) {
                task.ops.push_back(pair.second);
            }
        }
    }
}

std::vector<int> TaskGraph::parseDependencies(const std::string& depsStr) {
    std::vector<int> deps;
    
    if (depsStr.empty()) {
        return deps;
    }
    
    std::stringstream ss(depsStr);
    std::string token;
    
    while (std::getline(ss, token, ';')) {
        // Trim whitespace
        token.erase(0, token.find_first_not_of(" \t\r\n"));
        token.erase(token.find_last_not_of(" \t\r\n") + 1);
        
        if (!token.empty()) {
            deps.push_back(std::stoi(token));
        }
    }
    
    return deps;
}

void TaskGraph::buildDAG() {
    // Build adjacency list for task-level dependencies
    adjacencyList.clear();
    
    for (const auto& task : tasks) {
        for (int depId : task.dependencies) {
            adjacencyList[depId].push_back(task.id);
        }
    }
    
    // Detect cycles
    detectCycles();
    
    // Create TaskInstances based on executions count
    instances.clear();
    int instanceIdCounter = 0;
    
    // Map from taskId to list of instance IDs for that task
    std::map<int, std::vector<int>> taskToInstances;
    
    for (const auto& task : tasks) {
        for (int exec = 0; exec < task.executions; exec++) {
            TaskInstance instance;
            instance.instanceId = instanceIdCounter++;
            instance.taskId = task.id;
            instance.currentOpIndex = 0;
            instance.inDegree = 0;  // Will be calculated below
            instance.readyTime = 0;
            instance.dispatchTime = 0;
            instance.doneTime = 0;
            
            instances.push_back(instance);
            taskToInstances[task.id].push_back(instance.instanceId);
        }
    }
    
    // Build instance-level dependencies and calculate in-degrees
    for (auto& instance : instances) {
        int taskId = instance.taskId;
        
        // Find the task
        const Task* task = nullptr;
        for (const auto& t : tasks) {
            if (t.id == taskId) {
                task = &t;
                break;
            }
        }
        
        if (task == nullptr) {
            throw std::runtime_error("Task not found for instance");
        }
        
        // For each dependency of this task
        for (int depTaskId : task->dependencies) {
            // All instances of the dependency task are predecessors
            const auto& depInstances = taskToInstances[depTaskId];
            instance.inDegree += depInstances.size();
        }
        
        // Build successor relationships
        if (adjacencyList.find(taskId) != adjacencyList.end()) {
            for (int successorTaskId : adjacencyList[taskId]) {
                // All instances of the successor task are successors
                const auto& successorInstances = taskToInstances[successorTaskId];
                for (int successorInstanceId : successorInstances) {
                    instance.successors.push_back(successorInstanceId);
                }
            }
        }
    }
}

void TaskGraph::detectCycles() {
    // Use DFS with three states: 0=unvisited, 1=visiting, 2=visited
    std::map<int, int> state;
    
    for (const auto& task : tasks) {
        state[task.id] = 0;
    }
    
    std::vector<int> path;
    
    for (const auto& task : tasks) {
        if (state[task.id] == 0) {
            dfsVisit(task.id, state, path);
        }
    }
}

void TaskGraph::dfsVisit(int taskId, std::map<int, int>& state, std::vector<int>& path) {
    state[taskId] = 1;  // Mark as visiting
    path.push_back(taskId);
    
    // Visit successors
    if (adjacencyList.find(taskId) != adjacencyList.end()) {
        for (int successorId : adjacencyList[taskId]) {
            if (state[successorId] == 1) {
                // Back edge detected - cycle found
                std::string cycleStr = "Cycle detected in task dependencies: ";
                for (int id : path) {
                    cycleStr += std::to_string(id) + " -> ";
                }
                cycleStr += std::to_string(successorId);
                throw std::runtime_error(cycleStr);
            } else if (state[successorId] == 0) {
                dfsVisit(successorId, state, path);
            }
        }
    }
    
    state[taskId] = 2;  // Mark as visited
    path.pop_back();
}

std::vector<int> TaskGraph::getReadyInstances() const {
    std::vector<int> ready;
    
    for (const auto& instance : instances) {
        if (instance.inDegree == 0 && instance.currentOpIndex == 0 && instance.readyTime == 0) {
            ready.push_back(instance.instanceId);
        }
    }
    
    return ready;
}

void TaskGraph::markInstanceComplete(int instanceId) {
    const auto& instance = instances[instanceId];
    
    // Decrement in-degree of all successors
    for (int successorId : instance.successors) {
        instances[successorId].inDegree--;
    }
}

const std::vector<Op>& TaskGraph::getOps(int taskId) const {
    for (const auto& task : tasks) {
        if (task.id == taskId) {
            return task.ops;
        }
    }
    
    throw std::runtime_error("Task not found: " + std::to_string(taskId));
}
