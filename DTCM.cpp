#include "DTCM.hpp"

DTCM::DTCM(uint64_t base, uint64_t sz, int lat, int core)
    : baseAddress(base), size(sz), latency(lat), coreId(core), accessCount(0) {
}

bool DTCM::inRange(uint64_t address) const {
    return (address >= baseAddress) && (address < baseAddress + size);
}

int DTCM::getLatency() const {
    return latency;
}

void DTCM::access(uint64_t address, AccessType rw) {
    // Record access for statistics tracking
    // The rw parameter is available for future use if needed
    (void)rw;  // Suppress unused parameter warning
    (void)address;  // Suppress unused parameter warning
    accessCount++;
}

uint64_t DTCM::getAccessCount() const {
    return accessCount;
}

int DTCM::getCoreId() const {
    return coreId;
}
