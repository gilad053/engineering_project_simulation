#include "Simulator.hpp"
#include <iostream>
#include <stdexcept>

Simulator::Simulator() : now(0) {}

void Simulator::initialize(const std::string& configPath, 
                          const std::string& tasksPath, 
                          const std::string& opsPath) {
    // Load configuration from JSON file
    config = Config::loadFromFile(configPath);
    config.validate();
    
    // Load and build task graph from CSV files
    taskGraph.loadFromCSV(tasksPath, opsPath);
    taskGraph.buildDAG();
    
    // Initialize cores array based on config.numCores
    cores.clear();
    cores.reserve(config.numCores);
    for (int i = 0; i < config.numCores; ++i) {
        cores.emplace_back(i);
    }
    
    // Initialize scheduler with policy and task graph
    scheduler = std::make_unique<Scheduler>(
        config.schedulingPolicy, 
        config.numCores, 
        &taskGraph
    );
    
    // Initialize memory system with config parameters
    memorySystem = std::make_unique<MemorySystem>(config);
    
    // Set up event scheduler callback for memory system
    memorySystem->setEventScheduler(&Simulator::eventSchedulerCallback, this);
    
    // Initialize statistics collector
    statsCollector = std::make_unique<StatsCollector>(config.numCores);
    
    // Seed initial TaskReady events for instances with inDegree == 0
    std::vector<int> readyInstances = taskGraph.getReadyInstances();
    for (int instanceId : readyInstances) {
        Event e(EventType::TaskReady, 0, -1, instanceId);
        scheduleEvent(e);
    }
    
    std::cout << "Simulator initialized with " << config.numCores << " cores, "
              << taskGraph.getInstances().size() << " task instances" << std::endl;
}

uint64_t Simulator::getCurrentTime() const {
    return now;
}

void Simulator::scheduleEvent(const Event& e) {
    eventQueue.push(e);
}

void Simulator::eventSchedulerCallback(Event event, void* context) {
    Simulator* sim = static_cast<Simulator*>(context);
    sim->scheduleEvent(event);
}

void Simulator::run() {
    std::cout << "Starting simulation..." << std::endl;
    
    // Main event loop - process events until queue is empty
    while (!eventQueue.empty()) {
        // Pop next event and advance time to event.time
        Event e = eventQueue.pop();
        now = e.time;
        
        // Dispatch event to appropriate handler based on EventType
        switch (e.type) {
            case EventType::TaskReady:
                handleTaskReady(e);
                break;
            case EventType::TaskDispatched:
                handleTaskDispatched(e);
                break;
            case EventType::ComputeDone:
                handleComputeDone(e);
                break;
            case EventType::MemReqIssued:
                handleMemReqIssued(e);
                break;
            case EventType::MemRespDone:
                handleMemRespDone(e);
                break;
            case EventType::TaskDone:
                handleTaskDone(e);
                break;
            default:
                // Other event types (CacheHit, CacheMiss, BankGrant) are internal
                // and don't require top-level handling
                break;
        }
        
        // Notify StatsCollector of each event
        statsCollector->onEvent(e, now);
    }
    
    std::cout << "Simulation complete at cycle " << now << std::endl;
    
    // Set total cycles in StatsCollector to final now value
    statsCollector->setTotalCycles(now);
    
    // Generate and output statistics
    statsCollector->generateReport(config.frequencyGHz);
    statsCollector->writeJSON("stats.json", config.frequencyGHz);
}

void Simulator::handleTaskReady(const Event& e) {
    // Add task instance to scheduler's ready queue
    scheduler->addReady(e.taskInstanceId);
    
    // Record ready time in stats
    statsCollector->recordTaskReady(e.taskInstanceId, now);
    
    // Try to dispatch if there's an idle core
    if (scheduler->hasReadyInstances()) {
        int coreId = scheduler->selectIdleCore();
        if (coreId != -1) {
            int instanceId = scheduler->selectNextInstance();
            if (instanceId != -1) {
                scheduler->dispatch(instanceId, coreId, now);
                
                // Schedule TaskDispatched event
                Event dispatchEvent(EventType::TaskDispatched, now, coreId, instanceId);
                scheduleEvent(dispatchEvent);
            }
        }
    }
}

void Simulator::handleTaskDispatched(const Event& e) {
    // Record dispatch time in stats
    statsCollector->recordTaskDispatched(e.taskInstanceId, now);
    
    // Get the task instance and its current operation
    TaskInstance& instance = taskGraph.getInstance(e.taskInstanceId);
    const std::vector<Op>& ops = taskGraph.getOps(instance.taskId);
    
    // Core executes the first operation
    if (instance.currentOpIndex < static_cast<int>(ops.size())) {
        const Op& op = ops[instance.currentOpIndex];
        cores[e.coreId].executeOp(op, e.taskInstanceId, now, eventQueue, taskGraph);
    }
}

void Simulator::handleComputeDone(const Event& e) {
    // Core completes the compute operation and moves to next
    cores[e.coreId].completeOp(now, eventQueue, taskGraph);
}

void Simulator::handleMemReqIssued(const Event& e) {
    // Issue memory request to memory system
    TaskInstance& instance = taskGraph.getInstance(e.taskInstanceId);
    const std::vector<Op>& ops = taskGraph.getOps(instance.taskId);
    const Op& op = ops[instance.currentOpIndex];
    
    memorySystem->issueRequest(e.address, op.rw, e.coreId, e.taskInstanceId, now);
}

void Simulator::handleMemRespDone(const Event& e) {
    // Memory response received, core completes the memory operation
    cores[e.coreId].completeOp(now, eventQueue, taskGraph);
}

void Simulator::handleTaskDone(const Event& e) {
    // Record task completion time
    statsCollector->recordTaskDone(e.taskInstanceId, now);
    
    // Mark instance as complete and get newly ready successors
    taskGraph.markInstanceComplete(e.taskInstanceId);
    
    // Release the core
    scheduler->releaseCore(e.coreId);
    
    // Schedule TaskReady events for newly ready successors
    std::vector<int> readyInstances = taskGraph.getReadyInstances();
    for (int instanceId : readyInstances) {
        TaskInstance& inst = taskGraph.getInstance(instanceId);
        // Only schedule if this instance just became ready (inDegree was just decremented to 0)
        if (inst.inDegree == 0 && inst.readyTime == 0) {
            inst.readyTime = now;
            Event readyEvent(EventType::TaskReady, now, -1, instanceId);
            scheduleEvent(readyEvent);
        }
    }
    
    // Try to dispatch another task to this now-idle core
    if (scheduler->hasReadyInstances()) {
        int instanceId = scheduler->selectNextInstance();
        if (instanceId != -1) {
            scheduler->dispatch(instanceId, e.coreId, now);
            
            // Schedule TaskDispatched event
            Event dispatchEvent(EventType::TaskDispatched, now, e.coreId, instanceId);
            scheduleEvent(dispatchEvent);
        }
    }
}
