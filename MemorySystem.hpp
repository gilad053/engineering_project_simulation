#ifndef MEMORYSYSTEM_HPP
#define MEMORYSYSTEM_HPP

#include <cstdint>
#include <vector>
#include <memory>
#include "Types.hpp"
#include "Config.hpp"
#include "DTCM.hpp"
#include "Cache.hpp"
#include "Interconnect.hpp"
#include "MemoryBank.hpp"
#include "MainMemory.hpp"

/**
 * MemorySystem class
 * Facade that routes memory requests through the hierarchy and coordinates responses
 */
class MemorySystem {
private:
    // Component ownership (nullptr for disabled features)
    std::unique_ptr<DTCM> dtcm;
    std::unique_ptr<Cache> cache;
    std::unique_ptr<Interconnect> interconnect;
    std::vector<MemoryBank> banks;
    std::unique_ptr<MainMemory> mainMemory;
    
    // Configuration reference
    const Config& config;
    
    // Callback function for scheduling events
    // This will be set by the Simulator to schedule MemRespDone events
    using EventSchedulerCallback = void(*)(Event event, void* context);
    EventSchedulerCallback eventScheduler;
    void* schedulerContext;
    
    // Memory request structure for internal tracking
    struct MemoryRequest {
        uint64_t address;
        AccessType rw;
        int coreId;
        int taskInstanceId;
        uint64_t requestTime;
        int tierLatency;  // Accumulated latency through tiers
    };

public:
    /**
     * Constructor
     * @param cfg Configuration reference
     */
    explicit MemorySystem(const Config& cfg);
    
    /**
     * Set the event scheduler callback for scheduling events
     * @param callback Function pointer to event scheduler
     * @param context Context pointer (typically the Simulator instance)
     */
    void setEventScheduler(EventSchedulerCallback callback, void* context);
    
    /**
     * Issue a memory request (entry point)
     * @param address Memory address
     * @param rw Read or Write
     * @param coreId Requesting core
     * @param taskInstanceId Task instance making request
     * @param currentTime Current simulation time
     */
    void issueRequest(uint64_t address, AccessType rw, int coreId, 
                     int taskInstanceId, uint64_t currentTime);
    
    /**
     * Route request to appropriate memory tier
     * @param req Memory request
     * @param currentTime Current simulation time
     */
    void routeRequest(const MemoryRequest& req, uint64_t currentTime);
    
    /**
     * Handle DTCM access
     * @param req Memory request
     * @param currentTime Current simulation time
     */
    void handleDTCMAccess(const MemoryRequest& req, uint64_t currentTime);
    
    /**
     * Handle cache access
     * @param req Memory request
     * @param currentTime Current simulation time
     */
    void handleCacheAccess(const MemoryRequest& req, uint64_t currentTime);
    
    /**
     * Handle memory bank access
     * @param req Memory request
     * @param currentTime Current simulation time
     */
    void handleBankAccess(const MemoryRequest& req, uint64_t currentTime);
    
    /**
     * Complete a memory request and schedule response event
     * @param req Memory request
     * @param currentTime Current simulation time
     * @param additionalLatency Additional latency to add
     */
    void completeRequest(const MemoryRequest& req, uint64_t currentTime, 
                        int additionalLatency);
    
    /**
     * Get DTCM pointer (for statistics)
     * @return DTCM pointer or nullptr if disabled
     */
    DTCM* getDTCM() const;
    
    /**
     * Get Cache pointer (for statistics)
     * @return Cache pointer or nullptr if disabled
     */
    Cache* getCache() const;
    
    /**
     * Get Interconnect pointer (for statistics)
     * @return Interconnect pointer or nullptr
     */
    Interconnect* getInterconnect() const;
    
    /**
     * Get MemoryBank reference (for statistics)
     * @param bankId Bank identifier
     * @return Reference to memory bank
     */
    MemoryBank& getBank(int bankId);
    
    /**
     * Get MainMemory pointer (for statistics)
     * @return MainMemory pointer or nullptr
     */
    MainMemory* getMainMemory() const;
};

#endif // MEMORYSYSTEM_HPP
