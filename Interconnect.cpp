#include "Interconnect.hpp"

Interconnect::Interconnect(InterconnectTopology topo, int baseLat, int width, int remotePenalty)
    : topology(topo), baseLatency(baseLat), linkWidth(width), 
      remoteChipletPenalty(remotePenalty), busy(false), busyUntil(0),
      totalTransfers(0), busyCycles(0) {
}

bool Interconnect::isAvailable() const {
    return !busy && requestQueue.empty();
}

int Interconnect::calculateLatency(int srcChiplet, int dstChiplet, int dataSize) const {
    // Start with base latency
    int latency = baseLatency;
    
    // Add serialization delay based on data size and link width
    // Serialization delay = ceil(dataSize / linkWidth)
    if (linkWidth > 0) {
        int serializationDelay = (dataSize + linkWidth - 1) / linkWidth;
        latency += serializationDelay;
    }
    
    // Add remote chiplet penalty for inter-chiplet transfers
    if (srcChiplet != dstChiplet) {
        latency += remoteChipletPenalty;
    }
    
    return latency;
}

void Interconnect::enqueue(uint64_t address, AccessType rw, int coreId,
                           int taskInstanceId, uint64_t requestTime,
                           int srcChiplet, int dstChiplet, int dataSize) {
    InterconnectRequest req;
    req.address = address;
    req.rw = rw;
    req.coreId = coreId;
    req.taskInstanceId = taskInstanceId;
    req.requestTime = requestTime;
    req.srcChipletId = srcChiplet;
    req.dstChipletId = dstChiplet;
    req.dataSize = dataSize;
    
    requestQueue.push(req);
}

uint64_t Interconnect::arbitrate(uint64_t currentTime) {
    // If no requests queued, return 0
    if (requestQueue.empty()) {
        busy = false;
        busyUntil = 0;
        return 0;
    }
    
    // Get next request from queue (FIFO arbitration)
    InterconnectRequest req = requestQueue.front();
    requestQueue.pop();
    
    // Calculate latency for this transfer
    int latency = calculateLatency(req.srcChipletId, req.dstChipletId, req.dataSize);
    
    // Mark interconnect as busy
    busy = true;
    busyUntil = currentTime + latency;
    
    // Update statistics
    totalTransfers++;
    busyCycles += latency;
    
    // Return completion time
    return busyUntil;
}

bool Interconnect::hasQueuedRequests() const {
    return !requestQueue.empty();
}

uint64_t Interconnect::getBusyUntil() const {
    return busyUntil;
}

void Interconnect::addBusyCycles(uint64_t cycles) {
    busyCycles += cycles;
}

uint64_t Interconnect::getTotalTransfers() const {
    return totalTransfers;
}

uint64_t Interconnect::getBusyCycles() const {
    return busyCycles;
}
