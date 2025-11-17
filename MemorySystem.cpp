#include "MemorySystem.hpp"
#include <stdexcept>

MemorySystem::MemorySystem(const Config& cfg) 
    : config(cfg), eventScheduler(nullptr), schedulerContext(nullptr) {
    
    // Initialize DTCM if enabled
    if (config.dtcmEnabled) {
        // Create one DTCM per core (each core has its own DTCM)
        // For simplicity, we'll create a single DTCM for core 0
        // In a full implementation, we'd have a vector of DTCMs
        dtcm = std::make_unique<DTCM>(
            config.dtcmBase,
            config.dtcmSize,
            config.dtcmLatency,
            0  // Core 0
        );
    }
    
    // Initialize Cache if enabled
    if (config.cacheEnabled) {
        cache = std::make_unique<Cache>(
            config.cacheSize,
            config.cacheHitLatency,
            config.cachePortLimit
        );
    }
    
    // Initialize Interconnect (always needed for bank access)
    interconnect = std::make_unique<Interconnect>(
        config.interconnectTopology,
        config.interconnectLatency,
        config.interconnectLinkWidth,
        config.remoteChipletPenalty
    );
    
    // Initialize Memory Banks
    banks.reserve(config.numMemoryBanks);
    for (int i = 0; i < config.numMemoryBanks; ++i) {
        int chipletId = config.getBankChiplet(i);
        banks.emplace_back(
            i,
            chipletId,
            config.bankServiceLatency,
            config.bankPortLimit,
            config.bankConflictPolicy
        );
    }
    
    // Initialize Main Memory (always needed as backing store)
    mainMemory = std::make_unique<MainMemory>(config.bankServiceLatency);
}

void MemorySystem::setEventScheduler(EventSchedulerCallback callback, void* context) {
    eventScheduler = callback;
    schedulerContext = context;
}

void MemorySystem::issueRequest(uint64_t address, AccessType rw, int coreId,
                                int taskInstanceId, uint64_t currentTime) {
    MemoryRequest req;
    req.address = address;
    req.rw = rw;
    req.coreId = coreId;
    req.taskInstanceId = taskInstanceId;
    req.requestTime = currentTime;
    req.tierLatency = 0;
    
    // Route the request through the memory hierarchy
    routeRequest(req, currentTime);
}

DTCM* MemorySystem::getDTCM() const {
    return dtcm.get();
}

Cache* MemorySystem::getCache() const {
    return cache.get();
}

Interconnect* MemorySystem::getInterconnect() const {
    return interconnect.get();
}

MemoryBank& MemorySystem::getBank(int bankId) {
    if (bankId < 0 || bankId >= static_cast<int>(banks.size())) {
        throw std::out_of_range("Invalid bank ID");
    }
    return banks[bankId];
}

MainMemory* MemorySystem::getMainMemory() const {
    return mainMemory.get();
}

void MemorySystem::routeRequest(const MemoryRequest& req, uint64_t currentTime) {
    // Check DTCM range first (highest priority, lowest latency)
    if (dtcm && dtcm->inRange(req.address)) {
        handleDTCMAccess(req, currentTime);
        return;
    }
    
    // Check Cache if enabled (second priority)
    if (cache) {
        handleCacheAccess(req, currentTime);
        return;
    }
    
    // Fall through to memory banks (lowest priority, highest latency)
    handleBankAccess(req, currentTime);
}

void MemorySystem::handleDTCMAccess(const MemoryRequest& req, uint64_t currentTime) {
    // Record the access for statistics
    dtcm->access(req.address, req.rw);
    
    // DTCM has fixed low latency - schedule MemRespDone event
    int latency = dtcm->getLatency();
    completeRequest(req, currentTime, latency);
}

void MemorySystem::handleCacheAccess(const MemoryRequest& req, uint64_t currentTime) {
    // Check if address is in cache
    bool hit = cache->lookup(req.address);
    
    if (hit) {
        // Cache hit - fast path
        int latency = cache->getHitLatency();
        completeRequest(req, currentTime, latency);
    } else {
        // Cache miss - insert into cache and forward to banks
        cache->insert(req.address);
        
        // Forward to memory banks (slow path)
        handleBankAccess(req, currentTime);
    }
}

void MemorySystem::handleBankAccess(const MemoryRequest& req, uint64_t currentTime) {
    // Determine which bank to access based on address
    int bankId = MemoryBank::getBankIndex(req.address, config.numMemoryBanks, 
                                          config.bankIndexFn);
    
    // Get source and destination chiplets for interconnect latency calculation
    int srcChiplet = config.getCoreChiplet(req.coreId);
    int dstChiplet = banks[bankId].getChipletId();
    
    // Calculate interconnect latency
    int interconnectLatency = interconnect->calculateLatency(srcChiplet, dstChiplet, 64);
    
    // Enqueue request to interconnect
    interconnect->enqueue(req.address, req.rw, req.coreId, req.taskInstanceId,
                         currentTime, srcChiplet, dstChiplet, 64);
    
    // Forward request to the appropriate bank
    banks[bankId].receiveRequest(req.address, req.rw, req.coreId, 
                                 req.taskInstanceId, currentTime, srcChiplet);
    
    // Calculate total latency: interconnect + bank service
    int bankLatency = banks[bankId].getServiceLatency();
    int totalLatency = interconnectLatency + bankLatency;
    
    // Complete the request with total latency
    completeRequest(req, currentTime, totalLatency);
}

void MemorySystem::completeRequest(const MemoryRequest& req, uint64_t currentTime,
                                   int additionalLatency) {
    // Calculate completion time
    uint64_t completionTime = currentTime + additionalLatency;
    
    // Create MemRespDone event
    Event responseEvent(
        EventType::MemRespDone,
        completionTime,
        req.coreId,
        req.taskInstanceId,
        req.address,
        0  // context field
    );
    
    // Schedule the event through the callback if available
    if (eventScheduler) {
        eventScheduler(responseEvent, schedulerContext);
    }
    
    // Note: Statistics collection will be handled by the Simulator/StatsCollector
    // when it processes the MemRespDone event. The memory tier accessed is implicit
    // from the event flow (DTCM hit, cache hit/miss, bank access).
}
