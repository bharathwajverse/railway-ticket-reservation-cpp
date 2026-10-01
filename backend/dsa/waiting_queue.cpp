// =============================================================================
// waiting_queue.cpp - Queue helpers for waiting list management
// MODULE IX: Queue ADT, Queue Operations, FIFO Queue Management
// MODULE X: STL Map and Queue Containers
// =============================================================================
#include "structures.h"
#include "config.h"
#include "waiting_queue.h"
#include <map>
#include <queue>
#include <vector>
#include <utility>

using namespace std;

// Time Complexity: O(log T) where T is number of trains in map
// Purpose: Enqueue a waiting entry for a train (FIFO)
bool enqueueWaiting(map<int, queue<WaitingEntry> >& waitingLists, const WaitingEntry& w) {
    if (waitingLists[w.trainNo].size() < MAX_WAITING) {
        waitingLists[w.trainNo].push(w);
        return true;
    }
    return false;
}

// Time Complexity: O(log T)
// Purpose: Dequeue the front waiting entry for a train (FIFO)
bool dequeueWaiting(map<int, queue<WaitingEntry> >& waitingLists, int trainNo, WaitingEntry& out) {
    map<int, queue<WaitingEntry> >::iterator it = waitingLists.find(trainNo);
    if (it != waitingLists.end() && !it->second.empty()) {
        out = it->second.front();
        it->second.pop();
        return true;
    }
    return false;
}

// Time Complexity: O(log T)
// Purpose: Check if a train's waiting queue is empty
bool isWaitingQueueEmpty(const map<int, queue<WaitingEntry> >& waitingLists, int trainNo) {
    map<int, queue<WaitingEntry> >::const_iterator it = waitingLists.find(trainNo);
    if (it == waitingLists.end()) {
        return true;
    }
    return it->second.empty();
}

// Time Complexity: O(log T)
// Purpose: Get size of a train's waiting queue
int waitingQueueSize(const map<int, queue<WaitingEntry> >& waitingLists, int trainNo) {
    map<int, queue<WaitingEntry> >::const_iterator it = waitingLists.find(trainNo);
    if (it == waitingLists.end()) {
        return 0;
    }
    return (int)it->second.size();
}

// Time Complexity: O(W) where W is size of waiting queue for train
// Purpose: Get vector of waiting list entries for display without modifying the queue
vector<WaitingEntry> getWaitingList(const map<int, queue<WaitingEntry> >& waitingLists, int trainNo) {
    vector<WaitingEntry> result;
    map<int, queue<WaitingEntry> >::const_iterator it = waitingLists.find(trainNo);
    if (it != waitingLists.end()) {
        queue<WaitingEntry> tempQueue = it->second;
        while (!tempQueue.empty()) {
            result.push_back(tempQueue.front());
            tempQueue.pop();
        }
    }
    return result;
}

// Time Complexity: O(T * W) where T is trains, W is average waiting list size
// Purpose: Get all waiting lists grouped by train
vector<pair<int, vector<WaitingEntry> > > getAllWaitingLists(const map<int, queue<WaitingEntry> >& waitingLists) {
    vector<pair<int, vector<WaitingEntry> > > result;
    for (map<int, queue<WaitingEntry> >::const_iterator it = waitingLists.begin(); it != waitingLists.end(); ++it) {
        if (!it->second.empty()) {
            vector<WaitingEntry> entries;
            queue<WaitingEntry> tempQueue = it->second;
            while (!tempQueue.empty()) {
                entries.push_back(tempQueue.front());
                tempQueue.pop();
            }
            result.push_back(make_pair(it->first, entries));
        }
    }
    return result;
}
