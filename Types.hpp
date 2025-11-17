#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstdint>
#include <string>
#include <vector>

// Event types for discrete-event simulation
enum class EventType {
    TaskReady,
    TaskDispatched,
    ComputeDone,
    MemReqIssued,
    CacheHit,
    CacheMiss,
    BankGrant,
    MemRespDone,
    TaskDone
};

// Operation types
enum class OpType {
    Compute,
    Memory
};

// Memory access types
enum class AccessType {
    Read,
    Write
};

// Scheduling policies
enum class SchedulingPolicy {
    FIFO,
    RoundRobin,
    ShortestOpsFirst
};

// Bank index functions
enum class BankIndexFunction {
    AddressModN,
    XorFold
};

// Bank conflict policies
enum class BankConflictPolicy {
    Serialize,
    Queue,
    ExtraDelay
};

// Interconnect topologies
enum class InterconnectTopology {
    Bus,
    Mesh
};

// Operation structure
struct Op {
    OpType type;
    int cycles;           // Compute cycles (0 for memory ops)
    uint64_t address;     // Memory address (0 for compute)
    AccessType rw;        // Read or Write
    
    Op() : type(OpType::Compute), cycles(0), address(0), rw(AccessType::Read) {}
    
    Op(OpType t, int c, uint64_t addr, AccessType access)
        : type(t), cycles(c), address(addr), rw(access) {}
};

// Event structure for discrete-event simulation
struct Event {
    EventType type;
    uint64_t time;
    int coreId;
    int taskInstanceId;
    uint64_t address;
    int context;  // Additional context field for flexible use
    
    Event() : type(EventType::TaskReady), time(0), coreId(-1), 
              taskInstanceId(-1), address(0), context(0) {}
    
    Event(EventType t, uint64_t tm, int core = -1, int instance = -1, 
          uint64_t addr = 0, int ctx = 0)
        : type(t), time(tm), coreId(core), taskInstanceId(instance), 
          address(addr), context(ctx) {}
};

// Task structure representing a task definition
struct Task {
    int id;
    std::string name;
    int executions;
    std::vector<int> dependencies;  // IDs of predecessor tasks
    std::vector<Op> ops;            // Sequence of operations
    
    Task() : id(-1), name(""), executions(1) {}
    
    Task(int taskId, const std::string& taskName, int exec)
        : id(taskId), name(taskName), executions(exec) {}
};

// TaskInstance structure representing a runtime instance of a task
struct TaskInstance {
    int instanceId;
    int taskId;
    int currentOpIndex;
    int inDegree;                   // Number of unsatisfied dependencies
    uint64_t readyTime;             // Cycle when became ready
    uint64_t dispatchTime;          // Cycle when dispatched
    uint64_t doneTime;              // Cycle when completed
    std::vector<int> successors;    // Instance IDs of dependent instances
    
    TaskInstance() 
        : instanceId(-1), taskId(-1), currentOpIndex(0), inDegree(0),
          readyTime(0), dispatchTime(0), doneTime(0) {}
    
    TaskInstance(int instId, int tId, int degree)
        : instanceId(instId), taskId(tId), currentOpIndex(0), inDegree(degree),
          readyTime(0), dispatchTime(0), doneTime(0) {}
};

#endif // TYPES_HPP
