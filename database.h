#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include "sqlite3.h"
#include "dsa_manager.h"

// ============================================================================
// SEPARATE DATABASE LAYER (SQLite 3 Persistence)
// Completely isolated database lifecycle, tables, prepared statements, and CRUD
// ============================================================================

bool dbOpen(sqlite3*& db, const char* fileName);
void dbClose(sqlite3* db);
bool dbCreateTables(sqlite3* db);

// Train Operations
bool dbInsertTrain(sqlite3* db, const Train& t);
bool dbUpdateTrainSeats(sqlite3* db, int trainNo, int availableSeats);
bool dbLoadTrains(sqlite3* db, std::vector<Train>& trains);

// Passenger Operations
bool dbInsertPassenger(sqlite3* db, const Passenger& p);
bool dbUpdatePassengerStatus(sqlite3* db, int pnr, const std::string& status);
bool dbLoadPassengers(sqlite3* db, std::vector<Passenger>& passengers);

// Waiting List Operations
bool dbInsertWaiting(sqlite3* db, WaitingEntry& w);
bool dbDeleteWaiting(sqlite3* db, int waitId);
bool dbLoadWaiting(sqlite3* db, std::vector<WaitingEntry>& waitingList);

#endif // DATABASE_H
