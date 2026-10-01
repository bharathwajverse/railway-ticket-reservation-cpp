#ifndef PASSENGER_REPO_H
#define PASSENGER_REPO_H
#include <vector>
#include <string>
#include "../include/structures.h"

// Insert a passenger into the database
bool dbInsertPassenger(const Passenger& p);
// Update a passenger's status
bool dbUpdatePassengerStatus(int pnr, const std::string& status);
// Load all passengers from database
bool dbLoadPassengers(std::vector<Passenger>& passengers);

#endif
