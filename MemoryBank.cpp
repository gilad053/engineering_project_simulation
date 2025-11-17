#include "MemoryBank.hpp"

MemoryBank::MemoryBank(int id, int chiplet, int latency, int portLim, BankConflictPolicy policy)
    : bankId(id), chipletId(chiplet), serviceLatency(latency), portLimit(portLim),
      currentPorts(0), conflictPolicy(policy), busy(false), busyUntil(0),
      requestCount(0), conflictCount(0), portConflictCount(0) {
}

bool MemoryBank::portAvailable() const {
    return (portLimit == 0) || (currentPorts < portLimit);
}

int MemoryBank::getBankId() const {
    return bankId;
}

int MemoryBank::getChipletId() const {
    return chipletId;
}

int MemoryBank::getServiceLatency() const {
    return serviceLatency;
}

bool MemoryBank::isBusy() const {
    return busy;
}

uint64_t MemoryBank::getBusyUntil() const {
    return busyUntil;
}

int MemoryBank::getBankIndex(uint64_t address, int numBanks, BankIndexFunction fn) {
    switch (fn) {
        case BankIndexFunction::AddressModN:
            // (address / 64) % numBanks
            // Divide by cache line size (64 bytes) then modulo
            return static_cast<int>((address / 64) % numBanks);
        
        case BankIndexFunction::XorFold:
            // (address ^ (address >> 16)) % numBanks
            // XOR fold to distribute bits more evenly
            return static_cast<int>((address ^ (address >> 16)) % numBanks);
        
        default:
            // Default to AddressModN
            return static_cast<int>((address / 64) % numBanks);
    }
}

void MemoryBank::receiveRequest(uint64_t address, AccessType rw, int coreId,
                                int taskInstanceId, uint64_t requestTime, int srcChiplet) {
    MemoryRequest req;
    req.address = address;
    req.rw = rw;
    req.coreId = coreId;
    req.taskInstanceId = taskInstanceId;
    req.requestTime = requestTime;
    req.srcChipletId = srcChiplet;
    
    requestQueue.push(req);
    requestCount++;
}

uint64_t MemoryBank::serviceRequest(uint64_t currentTime) {
    // Check if there are any requests to service
    if (requestQueue.empty()) {
        return 0;
    }
    
    uint64_t completionTime = 0;
    
    switch (conflictPolicy) {
        case BankConflictPolicy::Serialize: {
            // Queue and service one at a time
            if (!busy) {
                // Bank is free, service the next request
                MemoryRequest req = requestQueue.front();
                requestQueue.pop();
                
                busy = true;
                busyUntil = currentTime + serviceLatency;
                completionTime = busyUntil;
                
                currentPorts = 1;
            } else {
                // Bank is busy, request stays queued
                conflictCount++;
                completionTime = 0;  // Indicates request is queued
            }
            break;
        }
        
        case BankConflictPolicy::Queue: {
            // Parallel service up to portLimit
            if (portAvailable()) {
                // Port available, service the request
                MemoryRequest req = requestQueue.front();
                requestQueue.pop();
                
                currentPorts++;
                
                // Calculate completion time
                completionTime = currentTime + serviceLatency;
                
                // Update busyUntil to track when a port will be freed
                if (completionTime > busyUntil) {
                    busyUntil = completionTime;
                }
                busy = (currentPorts > 0);
            } else {
                // No ports available, request stays queued
                portConflictCount++;
                conflictCount++;
                completionTime = 0;  // Indicates request is queued
            }
            break;
        }
        
        case BankConflictPolicy::ExtraDelay: {
            // Add fixed penalty for conflicts
            MemoryRequest req = requestQueue.front();
            requestQueue.pop();
            
            int extraDelay = 0;
            if (busy && currentTime < busyUntil) {
                // Bank is busy, add extra delay penalty
                extraDelay = 10;  // Fixed penalty cycles
                conflictCount++;
            }
            
            busy = true;
            completionTime = currentTime + serviceLatency + extraDelay;
            busyUntil = completionTime;
            
            currentPorts = 1;
            break;
        }
        
        default:
            // Default to Serialize policy
            if (!busy) {
                MemoryRequest req = requestQueue.front();
                requestQueue.pop();
                
                busy = true;
                busyUntil = currentTime + serviceLatency;
                completionTime = busyUntil;
                
                currentPorts = 1;
            } else {
                conflictCount++;
                completionTime = 0;
            }
            break;
    }
    
    return completionTime;
}

bool MemoryBank::hasQueuedRequests() const {
    return !requestQueue.empty();
}

uint64_t MemoryBank::getRequestCount() const {
    return requestCount;
}

uint64_t MemoryBank::getConflictCount() const {
    return conflictCount;
}

uint64_t MemoryBank::getPortConflictCount() const {
    return portConflictCount;
}
