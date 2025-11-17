#ifndef DTCM_HPP
#define DTCM_HPP

#include <cstdint>
#include "Types.hpp"

/**
 * DTCM (Data Tightly-Coupled Memory) class
 * Models private low-latency memory with address range checking
 */
class DTCM {
private:
    uint64_t baseAddress;  // Start of DTCM range
    uint64_t size;         // Size in bytes
    int latency;           // Access latency in cycles
    int coreId;            // Owning core
    
    // Statistics tracking
    uint64_t accessCount;  // Total number of accesses

public:
    /**
     * Constructor
     * @param base Base address of DTCM
     * @param sz Size in bytes
     * @param lat Access latency in cycles
     * @param core Owning core ID
     */
    DTCM(uint64_t base, uint64_t sz, int lat, int core);
    
    /**
     * Check if an address is within DTCM bounds
     * @param address Memory address to check
     * @return true if address is in DTCM range, false otherwise
     */
    bool inRange(uint64_t address) const;
    
    /**
     * Get the access latency
     * @return Fixed latency in cycles
     */
    int getLatency() const;
    
    /**
     * Record a memory access for statistics tracking
     * @param address Memory address being accessed
     * @param rw Read or Write access type
     */
    void access(uint64_t address, AccessType rw);
    
    /**
     * Get the total number of accesses
     * @return Access count
     */
    uint64_t getAccessCount() const;
    
    /**
     * Get the core ID that owns this DTCM
     * @return Core ID
     */
    int getCoreId() const;
};

#endif // DTCM_HPP
