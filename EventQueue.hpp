#ifndef EVENTQUEUE_HPP
#define EVENTQUEUE_HPP

#include "Types.hpp"
#include <queue>
#include <vector>

// Comparator for min-heap ordering based on event time
struct EventComparator {
    bool operator()(const Event& a, const Event& b) const {
        return a.time > b.time;  // Min-heap: earlier times have higher priority
    }
};

// EventQueue class using std::priority_queue with time-based min-heap ordering
class EventQueue {
private:
    std::priority_queue<Event, std::vector<Event>, EventComparator> queue;

public:
    EventQueue() = default;
    
    // Add an event to the queue
    void push(const Event& event) {
        queue.push(event);
    }
    
    // Remove and return the earliest event
    Event pop() {
        Event e = queue.top();
        queue.pop();
        return e;
    }
    
    // Check if the queue is empty
    bool empty() const {
        return queue.empty();
    }
    
    // Get the earliest event without removing it
    const Event& top() const {
        return queue.top();
    }
};

#endif // EVENTQUEUE_HPP
