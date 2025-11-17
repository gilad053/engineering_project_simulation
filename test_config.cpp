#include "Config.hpp"
#include <iostream>
#include <cassert>

int main() {
    try {
        std::cout << "Testing Config class..." << std::endl;
        
        // Test loading from file
        Config config = Config::loadFromFile("test_config.json");
        
        // Verify basic parameters
        assert(config.numCores == 4);
        assert(config.numChiplets == 2);
        assert(config.numMemoryBanks == 4);
        assert(config.schedulingPolicy == SchedulingPolicy::FIFO);
        
        // Verify cache configuration
        assert(config.cacheEnabled == true);
        assert(config.cacheSize == 32768);
        assert(config.cacheHitLatency == 2);
        assert(config.cachePortLimit == 2);
        
        // Verify DTCM configuration
        assert(config.dtcmEnabled == true);
        assert(config.dtcmBase == 0x80000000);
        assert(config.dtcmSize == 16384);
        assert(config.dtcmLatency == 1);
        
        // Verify memory bank configuration
        assert(config.bankServiceLatency == 50);
        assert(config.bankIndexFn == BankIndexFunction::AddressModN);
        assert(config.bankConflictPolicy == BankConflictPolicy::Queue);
        assert(config.bankPortLimit == 2);
        
        // Verify interconnect configuration
        assert(config.interconnectTopology == InterconnectTopology::Bus);
        assert(config.interconnectLatency == 10);
        assert(config.interconnectLinkWidth == 8);
        
        // Verify chiplet configuration
        assert(config.remoteChipletPenalty == 20);
        assert(config.frequencyGHz == 2.0);
        
        // Test chiplet mapping methods
        assert(config.getCoreChiplet(0) == 0);
        assert(config.getCoreChiplet(1) == 1);
        assert(config.getCoreChiplet(2) == 0);
        assert(config.getCoreChiplet(3) == 1);
        
        assert(config.getBankChiplet(0) == 0);
        assert(config.getBankChiplet(1) == 1);
        assert(config.getBankChiplet(2) == 0);
        assert(config.getBankChiplet(3) == 1);
        
        std::cout << "All Config tests passed!" << std::endl;
        
        // Test validation with invalid config
        Config invalidConfig;
        invalidConfig.numCores = -1;
        try {
            invalidConfig.validate();
            std::cerr << "ERROR: Validation should have failed for negative cores" << std::endl;
            return 1;
        } catch (const std::runtime_error& e) {
            std::cout << "Validation correctly caught error: " << e.what() << std::endl;
        }
        
        std::cout << "Config validation test passed!" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
