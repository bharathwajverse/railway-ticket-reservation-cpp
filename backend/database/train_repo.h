#ifndef TRAIN_REPO_H
#define TRAIN_REPO_H
#include <vector>
#include "../include/structures.h"

// Insert a train into the database
bool dbInsertTrain(const Train& t);
// Update available seats for a train
bool dbUpdateTrainSeats(int trainNo, int availableSeats);
// Load all trains from database
bool dbLoadTrains(std::vector<Train>& trains);

#endif
