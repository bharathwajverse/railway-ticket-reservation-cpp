#ifndef RAILWAY_H
#define RAILWAY_H

#include "structures.h"
#include "sqlite3.h"

// System initialization and seeding
void loadSystem(RailwaySystem& sys, sqlite3* db);
void seedSampleTrains(RailwaySystem& sys, sqlite3* db);

// Train management operations
void addTrain(RailwaySystem& sys, sqlite3* db);
void displayTrains(const RailwaySystem& sys);
int binarySearchTrain(const vector<Train>& trains, int trainNo);
void searchTrainByNumber(const RailwaySystem& sys);
void searchTrainByDestination(const RailwaySystem& sys);
void sortTrains(RailwaySystem& sys);

// Booking operations
int findTrainIndex(const RailwaySystem& sys, int trainNo);
int findFreeSeat(const RailwaySystem& sys, int trainIndex);
int generatePNR(const RailwaySystem& sys);
void bookTicket(RailwaySystem& sys, sqlite3* db);

// Cancellation and queue promotion operations
int findPassengerByPNR(const RailwaySystem& sys, int pnr);
bool promoteFromWaitingList(RailwaySystem& sys, sqlite3* db, int trainIndex, int seatNo);
void cancelTicket(RailwaySystem& sys, sqlite3* db);

// Bonus Stack Operations (Module VIII: LIFO)
void viewLastCancelledTicket(const RailwaySystem& sys);
void undoLastCancellation(RailwaySystem& sys, sqlite3* db);

// Display and reporting operations
void displayAvailableSeats(const RailwaySystem& sys);
void displayPassengerDetails(const RailwaySystem& sys);
void displayWaitingList(const RailwaySystem& sys);

#endif // RAILWAY_H
