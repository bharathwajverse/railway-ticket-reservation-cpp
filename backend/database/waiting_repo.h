#ifndef WAITING_REPO_H
#define WAITING_REPO_H
#include <map>
#include <queue>
#include "../include/structures.h"

// Insert a waiting entry into the database
bool dbInsertWaiting(const WaitingEntry& w);
// Delete a waiting entry by ID
bool dbDeleteWaiting(int waitId);
// Load all waiting entries from database
bool dbLoadWaiting(std::map<int, std::queue<WaitingEntry> >& waitingLists);

#endif
