#include "EventQueue.hpp"
#include <iostream>
#include <cassert>

int main() {
    EventQueue eq;
    
    // Test 1: Empty queue
    assert(eq.empty() == true);
    std::cout << "Test 1 passed: Empty queue check\n";
    
    // Test 2: Push events and verify ordering
    Event e1(EventType::TaskReady, 100, 0, 1);
    Event e2(EventType::ComputeDone, 50, 1, 2);
    Event e3(EventType::MemRespDone, 75, 2, 3);
    
    eq.push(e1);
    eq.push(e2);
    eq.push(e3);
    
    assert(eq.empty() == false);
    std::cout << "Test 2 passed: Queue not empty after push\n";
    
    // Test 3: Verify min-heap ordering (earliest time first)
    Event first = eq.top();
    assert(first.time == 50);
    assert(first.type == EventType::ComputeDone);
    std::cout << "Test 3 passed: Top returns earliest event (time=50)\n";
    
    // Test 4: Pop events in time order
    Event popped1 = eq.pop();
    assert(popped1.time == 50);
    assert(popped1.coreId == 1);
    assert(popped1.taskInstanceId == 2);
    std::cout << "Test 4 passed: First pop returns time=50\n";
    
    Event popped2 = eq.pop();
    assert(popped2.time == 75);
    assert(popped2.type == EventType::MemRespDone);
    std::cout << "Test 5 passed: Second pop returns time=75\n";
    
    Event popped3 = eq.pop();
    assert(popped3.time == 100);
    assert(popped3.type == EventType::TaskReady);
    std::cout << "Test 6 passed: Third pop returns time=100\n";
    
    // Test 5: Queue should be empty now
    assert(eq.empty() == true);
    std::cout << "Test 7 passed: Queue empty after all pops\n";
    
    std::cout << "\nAll EventQueue tests passed!\n";
    return 0;
}
