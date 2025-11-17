#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "Types.hpp"
#include "JSONParser.hpp"
#include <string>
#include <stdexcept>
#include <vector>
#include <cstdint>

// Configuration structure for the simulator
struct Config {
    // Core and chiplet configuration
    int numCores;
    int numMemoryBanks;
    int numChiplets;
    
    // Scheduling policy
    SchedulingPolicy schedulingPolicy;
    
    // Cache configuration
    bool cacheEnabled;
    int cacheSize;           // Size in bytes
    int cacheHitLatency;     // Cycles for cache hit
    int cachePortLimit;      // Max concurrent cache accesses
    
    // DTCM configuration
    bool dtcmEnabled;
    uint64_t dtcmBase;       // Base address
    uint64_t dtcmSize;       // Size in bytes
    int dtcmLatency;         // Access latency in cycles
    
    // Memory bank configuration
    int bankServiceLatency;  // Cycles to service a request
    BankIndexFunction bankIndexFn;
    BankConflictPolicy bankConflictPolicy;
    int bankPortLimit;       // Max concurrent bank accesses
    
    // Interconnect configuration
    InterconnectTopology interconnectTopology;
    int interconnectLatency; // Base interconnect latency
    int interconnectLinkWidth; // Bytes per cycle
    
    // Chiplet configuration
    int remoteChipletPenalty; // Additional cycles for inter-chiplet access
    
    // System configuration
    double frequencyGHz;     // Clock frequency for time conversion
    
    // Chiplet mapping vectors (computed during initialization)
    std::vector<int> coreToChiplet;
    std::vector<int> bankToChiplet;
    
    // Default constructor
    Config() 
        : numCores(1), numMemoryBanks(1), numChiplets(1),
          schedulingPolicy(SchedulingPolicy::FIFO),
          cacheEnabled(false), cacheSize(0), cacheHitLatency(0), cachePortLimit(1),
          dtcmEnabled(false), dtcmBase(0), dtcmSize(0), dtcmLatency(0),
          bankServiceLatency(0), bankIndexFn(BankIndexFunction::AddressModN),
          bankConflictPolicy(BankConflictPolicy::Serialize), bankPortLimit(1),
          interconnectTopology(InterconnectTopology::Bus),
          interconnectLatency(0), interconnectLinkWidth(8),
          remoteChipletPenalty(0), frequencyGHz(1.0) {}
    
    // Load configuration from JSON file
    static Config loadFromFile(const std::string& filepath);
    
    // Validate configuration parameters
    void validate() const;
    
    // Get chiplet ID for a core
    int getCoreChiplet(int coreId) const;
    
    // Get chiplet ID for a bank
    int getBankChiplet(int bankId) const;

private:
    // Helper methods for parsing
    static SchedulingPolicy parseSchedulingPolicy(const std::string& str);
    static BankIndexFunction parseBankIndexFunction(const std::string& str);
    static BankConflictPolicy parseBankConflictPolicy(const std::string& str);
    static InterconnectTopology parseInterconnectTopology(const std::string& str);
    
    // Initialize chiplet mappings
    void initializeChipletMappings();
};

#endif // CONFIG_HPP
