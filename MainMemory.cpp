#include "MainMemory.hpp"

MainMemory::MainMemory(int latency) 
    : baseLatency_(latency) {
}

void MainMemory::read(uint64_t address, int size) {
    // Functional behavior is optional - primarily for latency modeling
    // Access the storage to ensure the address exists (sparse allocation)
    for (int i = 0; i < size; ++i) {
        // Touch each byte in the range (creates entry if doesn't exist)
        [[maybe_unused]] uint8_t byte = storage_[address + i];
    }
}

void MainMemory::write(uint64_t address, int size) {
    // Functional behavior is optional - primarily for latency modeling
    // Write zeros to the storage (sparse allocation)
    for (int i = 0; i < size; ++i) {
        storage_[address + i] = 0;
    }
}

int MainMemory::getLatency() const {
    return baseLatency_;
}
