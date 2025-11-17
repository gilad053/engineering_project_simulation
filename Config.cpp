#include "Config.hpp"
#include <algorithm>
#include <sstream>

Config Config::loadFromFile(const std::string& filepath) {
    Config config;
    
    try {
        auto root = JSONParser::parseFile(filepath);
        
        if (!root->isObject()) {
            throw std::runtime_error("Config file must contain a JSON object");
        }
        
        // Parse core and chiplet configuration
        if (root->hasKey("cores")) {
            config.numCores = root->get("cores")->asInt();
        }
        
        if (root->hasKey("chiplets")) {
            config.numChiplets = root->get("chiplets")->asInt();
        }
        
        // Parse scheduling policy
        if (root->hasKey("scheduling_policy")) {
            config.schedulingPolicy = parseSchedulingPolicy(root->get("scheduling_policy")->asString());
        }
        
        // Parse cache configuration
        if (root->hasKey("cache")) {
            auto cache = root->get("cache");
            if (cache->hasKey("enabled")) {
                config.cacheEnabled = cache->get("enabled")->asBool();
            }
            if (cache->hasKey("size_bytes")) {
                config.cacheSize = cache->get("size_bytes")->asInt();
            }
            if (cache->hasKey("hit_latency_cycles")) {
                config.cacheHitLatency = cache->get("hit_latency_cycles")->asInt();
            }
            if (cache->hasKey("port_limit")) {
                config.cachePortLimit = cache->get("port_limit")->asInt();
            }
        }
        
        // Parse DTCM configuration
        if (root->hasKey("dtcm")) {
            auto dtcm = root->get("dtcm");
            if (dtcm->hasKey("enabled")) {
                config.dtcmEnabled = dtcm->get("enabled")->asBool();
            }
            if (dtcm->hasKey("base_address")) {
                config.dtcmBase = dtcm->get("base_address")->asHexAddress();
            }
            if (dtcm->hasKey("size_bytes")) {
                config.dtcmSize = static_cast<uint64_t>(dtcm->get("size_bytes")->asNumber());
            }
            if (dtcm->hasKey("latency_cycles")) {
                config.dtcmLatency = dtcm->get("latency_cycles")->asInt();
            }
        }
        
        // Parse memory bank configuration
        if (root->hasKey("memory_banks")) {
            auto banks = root->get("memory_banks");
            if (banks->hasKey("count")) {
                config.numMemoryBanks = banks->get("count")->asInt();
            }
            if (banks->hasKey("service_latency_cycles")) {
                config.bankServiceLatency = banks->get("service_latency_cycles")->asInt();
            }
            if (banks->hasKey("bank_index_function")) {
                config.bankIndexFn = parseBankIndexFunction(banks->get("bank_index_function")->asString());
            }
            if (banks->hasKey("conflict_policy")) {
                config.bankConflictPolicy = parseBankConflictPolicy(banks->get("conflict_policy")->asString());
            }
            if (banks->hasKey("port_limit")) {
                config.bankPortLimit = banks->get("port_limit")->asInt();
            }
        }
        
        // Parse interconnect configuration
        if (root->hasKey("interconnect")) {
            auto interconnect = root->get("interconnect");
            if (interconnect->hasKey("topology")) {
                config.interconnectTopology = parseInterconnectTopology(interconnect->get("topology")->asString());
            }
            if (interconnect->hasKey("base_latency_cycles")) {
                config.interconnectLatency = interconnect->get("base_latency_cycles")->asInt();
            }
            if (interconnect->hasKey("link_width_bytes_per_cycle")) {
                config.interconnectLinkWidth = interconnect->get("link_width_bytes_per_cycle")->asInt();
            }
        }
        
        // Parse chiplet configuration
        if (root->hasKey("chiplet")) {
            auto chiplet = root->get("chiplet");
            if (chiplet->hasKey("remote_penalty_cycles")) {
                config.remoteChipletPenalty = chiplet->get("remote_penalty_cycles")->asInt();
            }
        }
        
        // Parse system configuration
        if (root->hasKey("frequency_ghz")) {
            config.frequencyGHz = root->get("frequency_ghz")->asNumber();
        }
        
        // Initialize chiplet mappings
        config.initializeChipletMappings();
        
        // Validate the configuration
        config.validate();
        
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to load config from " + filepath + ": " + e.what());
    }
    
    return config;
}

void Config::validate() const {
    // Validate core count
    if (numCores <= 0) {
        throw std::runtime_error("Number of cores must be positive");
    }
    
    // Validate memory bank count
    if (numMemoryBanks <= 0) {
        throw std::runtime_error("Number of memory banks must be positive");
    }
    
    // Validate chiplet count
    if (numChiplets <= 0) {
        throw std::runtime_error("Number of chiplets must be positive");
    }
    
    // Validate cache configuration
    if (cacheEnabled) {
        if (cacheSize <= 0) {
            throw std::runtime_error("Cache size must be positive when cache is enabled");
        }
        if (cacheHitLatency < 0) {
            throw std::runtime_error("Cache hit latency cannot be negative");
        }
        if (cachePortLimit <= 0) {
            throw std::runtime_error("Cache port limit must be positive");
        }
    }
    
    // Validate DTCM configuration
    if (dtcmEnabled) {
        if (dtcmSize == 0) {
            throw std::runtime_error("DTCM size must be positive when DTCM is enabled");
        }
        if (dtcmLatency < 0) {
            throw std::runtime_error("DTCM latency cannot be negative");
        }
    }
    
    // Validate memory bank configuration
    if (bankServiceLatency < 0) {
        throw std::runtime_error("Bank service latency cannot be negative");
    }
    if (bankPortLimit <= 0) {
        throw std::runtime_error("Bank port limit must be positive");
    }
    
    // Validate interconnect configuration
    if (interconnectLatency < 0) {
        throw std::runtime_error("Interconnect latency cannot be negative");
    }
    if (interconnectLinkWidth <= 0) {
        throw std::runtime_error("Interconnect link width must be positive");
    }
    
    // Validate chiplet configuration
    if (remoteChipletPenalty < 0) {
        throw std::runtime_error("Remote chiplet penalty cannot be negative");
    }
    
    // Validate frequency
    if (frequencyGHz <= 0.0) {
        throw std::runtime_error("Frequency must be positive");
    }
    
    // Validate chiplet mappings
    if (coreToChiplet.size() != static_cast<size_t>(numCores)) {
        throw std::runtime_error("Core to chiplet mapping size mismatch");
    }
    if (bankToChiplet.size() != static_cast<size_t>(numMemoryBanks)) {
        throw std::runtime_error("Bank to chiplet mapping size mismatch");
    }
    
    // Validate chiplet IDs are in valid range
    for (int chipletId : coreToChiplet) {
        if (chipletId < 0 || chipletId >= numChiplets) {
            throw std::runtime_error("Invalid chiplet ID in core mapping");
        }
    }
    for (int chipletId : bankToChiplet) {
        if (chipletId < 0 || chipletId >= numChiplets) {
            throw std::runtime_error("Invalid chiplet ID in bank mapping");
        }
    }
}

int Config::getCoreChiplet(int coreId) const {
    if (coreId < 0 || coreId >= numCores) {
        throw std::runtime_error("Invalid core ID: " + std::to_string(coreId));
    }
    return coreToChiplet[coreId];
}

int Config::getBankChiplet(int bankId) const {
    if (bankId < 0 || bankId >= numMemoryBanks) {
        throw std::runtime_error("Invalid bank ID: " + std::to_string(bankId));
    }
    return bankToChiplet[bankId];
}

SchedulingPolicy Config::parseSchedulingPolicy(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "fifo") {
        return SchedulingPolicy::FIFO;
    } else if (lower == "roundrobin" || lower == "round_robin") {
        return SchedulingPolicy::RoundRobin;
    } else if (lower == "shortestopsfirst" || lower == "shortest_ops_first") {
        return SchedulingPolicy::ShortestOpsFirst;
    } else {
        throw std::runtime_error("Unknown scheduling policy: " + str);
    }
}

BankIndexFunction Config::parseBankIndexFunction(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "addr_mod_n" || lower == "addressmodn") {
        return BankIndexFunction::AddressModN;
    } else if (lower == "xor_fold" || lower == "xorfold") {
        return BankIndexFunction::XorFold;
    } else {
        throw std::runtime_error("Unknown bank index function: " + str);
    }
}

BankConflictPolicy Config::parseBankConflictPolicy(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "serialize") {
        return BankConflictPolicy::Serialize;
    } else if (lower == "queue") {
        return BankConflictPolicy::Queue;
    } else if (lower == "extra_delay" || lower == "extradelay") {
        return BankConflictPolicy::ExtraDelay;
    } else {
        throw std::runtime_error("Unknown bank conflict policy: " + str);
    }
}

InterconnectTopology Config::parseInterconnectTopology(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "bus") {
        return InterconnectTopology::Bus;
    } else if (lower == "mesh") {
        return InterconnectTopology::Mesh;
    } else {
        throw std::runtime_error("Unknown interconnect topology: " + str);
    }
}

void Config::initializeChipletMappings() {
    // Simple round-robin distribution of cores and banks to chiplets
    coreToChiplet.resize(numCores);
    for (int i = 0; i < numCores; i++) {
        coreToChiplet[i] = i % numChiplets;
    }
    
    bankToChiplet.resize(numMemoryBanks);
    for (int i = 0; i < numMemoryBanks; i++) {
        bankToChiplet[i] = i % numChiplets;
    }
}
