#ifndef MEMORYBANK_HPP
#define MEMORYBANK_HPP

#include <cstdint>
#include <queue>
#include "Types.hpp"

/**
 * MemoryBank class
 * Services memory requests with configurable conflict handling and port limits
 */
class MemoryBank {
private:
    int bankId;                      // Bank identifier
    int chipletId;                   // Chiplet location
    int serviceLatency;              // Cycles to service a request
    int portLimit;                   // Max concurrent accesses
    int currentPorts;                // Currently used ports
    BankConflictPolicy conflictPolicy;  // How to handle conflicts
    
    // Request queue for pending requests
    struct MemoryRequest {
        uint64_t address;
        AccessType rw;
        int coreId;
        int taskInstanceId;
        uint64_t requestTime;
        int srcChipletId;
    };
    std::queue<MemoryRequest> requestQueue;
    
    bool busy;                       // Whether bank is servicing a request
    uint64_t busyUntil;              // Time when current request completes
    
    // Statistics tracking
    uint64_t requestCount;
    uint64_t conflictCount;
    uint64_t portConflictCount;

public:
    /**
     * Constructor
     * @param id Bank identifier
     * @param chiplet Chiplet location
     * @param latency Service latency in cycles
     * @param portLim Maximum concurrent accesses (0 = unlimited)
     * @param policy Conflict handling policy
     */
    MemoryBank(int id, int chiplet, int latency, int portLim, BankConflictPolicy policy);
    
    /**
     * Check if a port is available
     * @return true if port available, false otherwise
     */
    bool portAvailable() const;
    
    /**
     * Get bank ID
     * @return Bank identifier
     */
    int getBankId() const;
    
    /**
     * Get chiplet ID
     * @return Chiplet identifier
     */
    int getChipletId() const;
    
    /**
     * Get service latency
     * @return Service latency in cycles
     */
    int getServiceLatency() const;
    
    /**
     * Check if bank is busy
     * @return true if busy, false otherwise
     */
    bool isBusy() const;
    
    /**
     * Get the time when bank will be free
     * @return Time in cycles
     */
    uint64_t getBusyUntil() const;
    
    /**
     * Static method to calculate bank index from address
     * @param address Memory address
     * @param numBanks Total number of banks
     * @param fn Bank index function to use
     * @return Bank index (0 to numBanks-1)
     */
    static int getBankIndex(uint64_t address, int numBanks, BankIndexFunction fn);
    
    /**
     * Receive a request from interconnect
     * @param address Memory address
     * @param rw Read or Write
     * @param coreId Requesting core
     * @param taskInstanceId Task instance making request
     * @param requestTime Time request was issued
     * @param srcChiplet Source chiplet ID
     */
    void receiveRequest(uint64_t address, AccessType rw, int coreId, 
                       int taskInstanceId, uint64_t requestTime, int srcChiplet);
    
    /**
     * Service a request based on conflict policy
     * @param currentTime Current simulation time
     * @return Completion time for the request, or 0 if queued
     */
    uint64_t serviceRequest(uint64_t currentTime);
    
    /**
     * Check if there are queued requests
     * @return true if queue is not empty
     */
    bool hasQueuedRequests() const;
    
    /**
     * Get statistics
     */
    uint64_t getRequestCount() const;
    uint64_t getConflictCount() const;
    uint64_t getPortConflictCount() const;
};

#endif // MEMORYBANK_HPP
