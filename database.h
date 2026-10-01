#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include "dsa_manager.h"

// ============================================================================
// SEPARATE MONGODB DOCUMENT DATABASE LAYER (NoSQL Persistence)
// Document-oriented collections (trains, passengers, waiting_list) in JSON
// with automatic mongosh shell script generation (mongo_seed.js)
// ============================================================================

bool dbOpen(const std::string& dataDirectory = "mongodb_data");
void dbClose();
bool dbCreateCollections();

// Train Document Collection Operations
bool dbInsertTrain(const Train& t);
bool dbUpdateTrainSeats(int trainNo, int availableSeats);
bool dbLoadTrains(std::vector<Train>& trains);

// Passenger Document Collection Operations
bool dbInsertPassenger(const Passenger& p);
bool dbUpdatePassengerStatus(int pnr, const std::string& status);
bool dbLoadPassengers(std::vector<Passenger>& passengers);

// Waiting List Document Collection Operations
bool dbInsertWaiting(WaitingEntry& w);
bool dbDeleteWaiting(int waitId);
bool dbLoadWaiting(std::vector<WaitingEntry>& waitingList);

// MongoDB Shell Script Generation (mongosh compatible)
bool dbExportMongoScript(const std::string& scriptFileName = "mongo_seed.js");

#endif // DATABASE_H
