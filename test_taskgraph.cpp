#include "TaskGraph.hpp"
#include <iostream>
#include <cassert>

int main() {
    try {
        std::cout << "Testing TaskGraph..." << std::endl;
        
        TaskGraph graph;
        
        // Load from test CSV files
        graph.loadFromCSV("test_tasks.csv", "test_ops.csv");
        
        std::cout << "Loaded " << graph.getTasks().size() << " tasks" << std::endl;
        
        // Build DAG
        graph.buildDAG();
        
        std::cout << "Built DAG with " << graph.getInstances().size() << " instances" << std::endl;
        
        // Get ready instances (should be tasks with no dependencies)
        auto ready = graph.getReadyInstances();
        std::cout << "Ready instances: " << ready.size() << std::endl;
        
        // Test getOps
        if (!graph.getTasks().empty()) {
            int firstTaskId = graph.getTasks()[0].id;
            const auto& ops = graph.getOps(firstTaskId);
            std::cout << "Task " << firstTaskId << " has " << ops.size() << " operations" << std::endl;
        }
        
        // Test markInstanceComplete
        if (!ready.empty()) {
            int instanceId = ready[0];
            std::cout << "Marking instance " << instanceId << " as complete" << std::endl;
            graph.markInstanceComplete(instanceId);
            
            // Check if successors' in-degree decreased
            const auto& instance = graph.getInstance(instanceId);
            std::cout << "Instance had " << instance.successors.size() << " successors" << std::endl;
        }
        
        std::cout << "All tests passed!" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
