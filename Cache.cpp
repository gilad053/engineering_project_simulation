#include "Cache.hpp"

Cache::Cache(int sz, int hitLat, int portLim)
    : size(sz), hitLatency(hitLat), portLimit(portLim), currentPorts(0),
      hitCount(0), missCount(0), portConflictCount(0) {
}

bool Cache::lookup(uint64_t address) {
    // Check if address is in cache
    auto it = cachedLines.find(address);
    
    if (it != cachedLines.end()) {
        // Cache hit - move to front of LRU list (most recently used)
        lruList.erase(it->second);
        lruList.push_front(address);
        cachedLines[address] = lruList.begin();
        
        hitCount++;
        return true;
    } else {
        // Cache miss
        missCount++;
        return false;
    }
}

void Cache::insert(uint64_t address) {
    // Check if already in cache
    auto it = cachedLines.find(address);
    
    if (it != cachedLines.end()) {
        // Already in cache, just update LRU position
        lruList.erase(it->second);
        lruList.push_front(address);
        cachedLines[address] = lruList.begin();
        return;
    }
    
    // Not in cache, need to insert
    // Check if cache is full
    if (static_cast<int>(cachedLines.size()) >= size) {
        // Evict LRU (back of list)
        uint64_t evictAddr = lruList.back();
        lruList.pop_back();
        cachedLines.erase(evictAddr);
    }
    
    // Insert new address at front (most recently used)
    lruList.push_front(address);
    cachedLines[address] = lruList.begin();
}

bool Cache::portAvailable() const {
    return (portLimit == 0) || (currentPorts < portLimit);
}

void Cache::acquirePort() {
    if (portLimit > 0) {
        currentPorts++;
        if (currentPorts > portLimit) {
            portConflictCount++;
        }
    }
}

void Cache::releasePort() {
    if (portLimit > 0 && currentPorts > 0) {
        currentPorts--;
    }
}

int Cache::getHitLatency() const {
    return hitLatency;
}

uint64_t Cache::getHitCount() const {
    return hitCount;
}

uint64_t Cache::getMissCount() const {
    return missCount;
}

uint64_t Cache::getPortConflictCount() const {
    return portConflictCount;
}

bool Cache::hasQueuedRequests() const {
    return !portQueue.empty();
}

void Cache::enqueueRequest(uint64_t address, AccessType rw, int coreId, 
                          int taskInstanceId, uint64_t requestTime) {
    PortRequest req;
    req.address = address;
    req.rw = rw;
    req.coreId = coreId;
    req.taskInstanceId = taskInstanceId;
    req.requestTime = requestTime;
    portQueue.push(req);
}

Cache::PortRequest Cache::dequeueRequest() {
    PortRequest req = portQueue.front();
    portQueue.pop();
    return req;
}
