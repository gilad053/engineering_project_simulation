#ifndef TASKGRAPH_HPP
#define TASKGRAPH_HPP

#include "Types.hpp"
#include <vector>
#include <map>
#include <string>

class TaskGraph {
public:
    TaskGraph();
    
    // Load tasks and operations from CSV files
    void loadFromCSV(const std::string& tasksPath, const std::string& opsPath);
    
    // Build DAG and create task instances
    void buildDAG();
    
    // Query methods
    std::vector<int> getReadyInstances() const;
    void markInstanceComplete(int instanceId);
    const std::vector<Op>& getOps(int taskId) const;
    
    // Accessors
    const std::vector<Task>& getTasks() const { return tasks; }
    const std::vector<TaskInstance>& getInstances() const { return instances; }
    TaskInstance& getInstance(int instanceId) { return instances[instanceId]; }
    const TaskInstance& getInstance(int instanceId) const { return instances[instanceId]; }
    
private:
    std::vector<Task> tasks;
    std::vector<TaskInstance> instances;
    std::map<int, std::vector<int>> adjacencyList;  // taskId -> list of successor taskIds
    
    // Helper methods
    std::vector<int> parseDependencies(const std::string& depsStr);
    void detectCycles();
    void dfsVisit(int taskId, std::map<int, int>& state, std::vector<int>& path);
};

#endif // TASKGRAPH_HPP
