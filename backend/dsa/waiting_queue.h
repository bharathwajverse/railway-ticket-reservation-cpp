// =============================================================================
// waiting_queue.h - Queue helpers for waiting list management
// MODULE IX: Queue ADT, Queue Operations, Queue Implementation using Arrays
// =============================================================================
#ifndef WAITING_QUEUE_H
#define WAITING_QUEUE_H

#include <vector>
#include <utility>
#include "structures.h"

// Add entry to a train's waiting queue. Returns false if full. O(1)
bool enqueueWaiting(std::map<int, std::queue<WaitingEntry> >& waitingLists,
                    const WaitingEntry& w);

// Remove front entry from a train's queue. Returns false if empty. O(1)
bool dequeueWaiting(std::map<int, std::queue<WaitingEntry> >& waitingLists,
                    int trainNo, WaitingEntry& out);

// Check if a train's waiting queue is empty. O(1)
bool isWaitingQueueEmpty(const std::map<int, std::queue<WaitingEntry> >& waitingLists,
                         int trainNo);

// Get size of a train's waiting queue. O(1)
int waitingQueueSize(const std::map<int, std::queue<WaitingEntry> >& waitingLists,
                     int trainNo);

// Get a copy of the waiting list as a vector (for display). O(n)
std::vector<WaitingEntry> getWaitingList(
    const std::map<int, std::queue<WaitingEntry> >& waitingLists, int trainNo);

// Get all waiting lists as vector of pairs: (trainNo, entries). O(total entries)
std::vector<std::pair<int, std::vector<WaitingEntry> > > getAllWaitingLists(
    const std::map<int, std::queue<WaitingEntry> >& waitingLists);

#endif // WAITING_QUEUE_H
