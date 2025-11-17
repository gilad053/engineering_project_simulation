#ifndef INTERCONNECT_HPP
#define INTERCONNECT_HPP

#include <cstdint>
#include <queue>
#include "Types.hpp"

/**
 * Interconnect class
 * Models on-chip network with topology, bandwidth, and contention
 */
class Interconnect {
private:
    InterconnectTopology topology;   // Bus or Mesh
    int baseLatency;                 // Minimum latency in cycles
    int linkWidth;                   // Bytes per cycle
    
    // Request structure for queuing
    struct InterconnectRequest {
        uint64_t address;
        AccessType rw;
        int coreId;
        int taskInstanceId;
        uint64_t requestTime;
        int srcChipletId;
        int dstChipletId;
        int dataSize;                // Size of data transfer in bytes
    };
    std::queue<InterconnectRequest> requestQueue;  // Pending transfers
    
    bool busy;                       // Whether interconnect is occupied
    uint64_t busyUntil;              // Time when current transfer completes
    
    // Configuration
    int remoteChipletPenalty;        // Additional cycles for inter-chiplet transfers
    
    // Statistics tracking
    uint64_t totalTransfers;
    uint64_t busyCycles;

public:
    /**
     * Constructor
     * @param topo Interconnect topology
     * @param baseLat Base latency in cycles
     * @param width Link width in bytes per cycle
     * @param remotePenalty Additional cycles for inter-chiplet transfers
     */
    Interconnect(InterconnectTopology topo, int baseLat, int width, int remotePenalty);
    
    /**
     * Check if interconnect is available to accept new transfer
     * @return true if available, false otherwise
     */
    bool isAvailable() const;
    
    /**
     * Calculate latency for a transfer
     * @param srcChiplet Source chiplet ID
     * @param dstChiplet Destination chiplet ID
     * @param dataSize Size of data transfer in bytes
     * @return Total latency in cycles
     */
    int calculateLatency(int srcChiplet, int dstChiplet, int dataSize) const;
    
    /**
     * Enqueue a request for transfer
     * @param address Memory address
     * @param rw Read or Write
     * @param coreId Requesting core
     * @param taskInstanceId Task instance making request
     * @param requestTime Time request was issued
     * @param srcChiplet Source chiplet ID
     * @param dstChiplet Destination chiplet ID
     * @param dataSize Size of data transfer in bytes (default 64 for cache line)
     */
    void enqueue(uint64_t address, AccessType rw, int coreId, 
                 int taskInstanceId, uint64_t requestTime,
                 int srcChiplet, int dstChiplet, int dataSize = 64);
    
    /**
     * Arbitrate and select next request when interconnect becomes free
     * @param currentTime Current simulation time
     * @return Completion time for the transfer, or 0 if no requests queued
     */
    uint64_t arbitrate(uint64_t currentTime);
    
    /**
     * Check if there are queued requests
     * @return true if queue is not empty
     */
    bool hasQueuedRequests() const;
    
    /**
     * Get the time when interconnect will be free
     * @return Time in cycles
     */
    uint64_t getBusyUntil() const;
    
    /**
     * Update busy cycles for statistics
     * @param cycles Number of cycles to add
     */
    void addBusyCycles(uint64_t cycles);
    
    /**
     * Get statistics
     */
    uint64_t getTotalTransfers() const;
    uint64_t getBusyCycles() const;
};

#endif // INTERCONNECT_HPP
