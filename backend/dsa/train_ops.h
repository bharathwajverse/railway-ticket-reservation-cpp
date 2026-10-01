// =============================================================================
// train_ops.h - Train search, sort, and management operations
// MODULE VII: Data Structures Performance Analysis (Binary Search, Bubble Sort)
// MODULE III: One-Dimensional Arrays (used in search results)
// =============================================================================
#ifndef TRAIN_OPS_H
#define TRAIN_OPS_H

#include <vector>
#include <string>
#include "structures.h"

// Binary search for a train by number. Returns index or -1. O(log n)
int binarySearchTrain(const std::vector<Train>& trains, int trainNo);

// Linear search for trains by destination (partial, case-insensitive). O(n)
std::vector<int> linearSearchByDestination(const std::vector<Train>& trains,
                                           const std::string& dest);

// Insert a train keeping vector sorted by trainNo. Returns index, or -1 if duplicate. O(n)
int insertTrainSorted(std::vector<Train>& trains, const Train& t);

// Sort a COPY of trains by fare (ascending). Bubble sort. O(n^2)
std::vector<Train> sortTrainsByFare(std::vector<Train> trains);

// Sort a COPY of trains by name (ascending). Bubble sort. O(n^2)
std::vector<Train> sortTrainsByName(std::vector<Train> trains);

// Format one train's info as a string. O(1)
std::string formatTrainInfo(const Train& t);

// Format all trains as a table string. O(n)
std::string formatTrainTable(const std::vector<Train>& trains);

#endif // TRAIN_OPS_H
