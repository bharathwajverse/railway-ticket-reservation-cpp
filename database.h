#ifndef DATABASE_H
#define DATABASE_H

#include "sqlite3.h"
#include "structures.h"
#include <vector>
#include <map>
#include <queue>
#include <string>

using namespace std;

// Database connection management
bool openDatabase(sqlite3*& db, const char* fileName);
void closeDatabase(sqlite3* db);

// Schema creation
bool createTables(sqlite3* db);

// Train CRUD operations
bool insertTrain(sqlite3* db, const Train& t);
bool updateTrainSeats(sqlite3* db, int trainNo, int availableSeats);
bool loadTrains(sqlite3* db, vector<Train>& trains);

// Passenger CRUD operations
bool insertPassenger(sqlite3* db, const Passenger& p);
bool updatePassengerStatus(sqlite3* db, int pnr, const string& status);
bool loadPassengers(sqlite3* db, vector<Passenger>& list);

// Waiting list CRUD operations
bool insertWaiting(sqlite3* db, WaitingEntry& w);
bool deleteWaiting(sqlite3* db, int waitId);
bool loadWaiting(sqlite3* db, map<int, queue<WaitingEntry> >& lists);

#endif // DATABASE_H
