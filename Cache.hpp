#ifndef CACHE_HPP
#define CACHE_HPP

#include <cstdint>
#include <set>
#include <list>
#include <unordered_map>
#include <queue>
#include "Types.hpp"

/**
 * Cache class
 * Models cache hit/miss behavior with configurable parameters and LRU eviction
 */
class Cache {
private:
    int size;                    // Cache size in bytes (number of cache lines)
    int hitLatency;              // Cycles for cache hit
    int portLimit;               // Max concurrent accesses
    int currentPorts;            // Currently used ports
    
    // LRU cache implementation using list + map
    std::list<uint64_t> lruList;                           // Most recent at front
    std::unordered_map<uint64_t, std::list<uint64_t>::iterator> cachedLines;  // Address -> iterator
    
    // Port management
    struct PortRequest {
        uint64_t address;
        AccessType rw;
        int coreId;
        int taskInstanceId;
        uint64_t requestTime;
    };
    std::queue<PortRequest> portQueue;  // Queue for requests exceeding port limit
    
    // Statistics tracking
    uint64_t hitCount;
    uint64_t missCount;
    uint64_t portConflictCount;

public:
    /**
     * Constructor
     * @param sz Cache size in bytes (number of cache lines to track)
     * @param hitLat Hit latency in cycles
     * @param portLim Maximum concurrent accesses (0 = unlimited)
     */
    Cache(int sz, int hitLat, int portLim);
    
    /**
     * Look up an address in the cache
     * @param address Memory address to look up
     * @return true if hit, false if miss
     */
    bool lookup(uint64_t address);
    
    /**
     * Insert an address into the cache with LRU eviction
     * @param address Memory address to insert
     */
    void insert(uint64_t address);
    
    /**
     * Check if a port is available
     * @return true if port available, false otherwise
     */
    bool portAvailable() const;
    
    /**
     * Acquire a port for a memory access
     */
    void acquirePort();
    
    /**
     * Release a port after memory access completes
     */
    void releasePort();
    
    /**
     * Get the hit latency
     * @return Hit latency in cycles
     */
    int getHitLatency() const;
    
    /**
     * Get statistics
     */
    uint64_t getHitCount() const;
    uint64_t getMissCount() const;
    uint64_t getPortConflictCount() const;
    
    /**
     * Check if port queue has pending requests
     * @return true if queue is not empty
     */
    bool hasQueuedRequests() const;
    
    /**
     * Enqueue a request when ports are unavailable
     */
    void enqueueRequest(uint64_t address, AccessType rw, int coreId, 
                       int taskInstanceId, uint64_t requestTime);
    
    /**
     * Dequeue the next request from port queue
     */
    PortRequest dequeueRequest();
};

#endif // CACHE_HPP
