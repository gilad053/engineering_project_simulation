#ifndef MAINMEMORY_HPP
#define MAINMEMORY_HPP

#include <cstdint>
#include <map>
#include "Types.hpp"

class MainMemory {
public:
    MainMemory(int latency);
    
    // Read data from memory (functional behavior optional)
    void read(uint64_t address, int size);
    
    // Write data to memory (functional behavior optional)
    void write(uint64_t address, int size);
    
    // Get access latency
    int getLatency() const;

private:
    std::map<uint64_t, uint8_t> storage_;  // Sparse storage
    int baseLatency_;                       // Access latency in cycles
};

#endif // MAINMEMORY_HPP
