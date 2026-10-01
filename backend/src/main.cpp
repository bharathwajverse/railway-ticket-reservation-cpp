// =============================================================================
// main.cpp - Application entry point
// Creates database connection, loads data, seeds defaults, runs menu loop
// =============================================================================
#include <iostream>
#include "structures.h"
#include "menu.h"
#include "db_connection.h"
#include "train_repo.h"
#include "passenger_repo.h"
#include "waiting_repo.h"
#include "seat_map.h"

using namespace std;

// Seed 4 sample trains if the database is empty. O(1)
void seedDefaultTrains(RailwaySystem& sys) {
    Train t1;
    t1.trainNo = 10101; t1.name = "Rajdhani Express";
    t1.source = "Delhi"; t1.destination = "Mumbai";
    t1.departure = "06:00 AM"; t1.totalSeats = 4;
    t1.availableSeats = 4; t1.fare = 1500.0f;

    Train t2;
    t2.trainNo = 10202; t2.name = "Vande Bharat";
    t2.source = "Chennai"; t2.destination = "Bangalore";
    t2.departure = "05:50 AM"; t2.totalSeats = 5;
    t2.availableSeats = 5; t2.fare = 950.0f;

    Train t3;
    t3.trainNo = 10303; t3.name = "Shatabdi Express";
    t3.source = "Kolkata"; t3.destination = "Patna";
    t3.departure = "02:15 PM"; t3.totalSeats = 3;
    t3.availableSeats = 3; t3.fare = 750.0f;

    Train t4;
    t4.trainNo = 10404; t4.name = "Tejas Express";
    t4.source = "Ahmedabad"; t4.destination = "Mumbai";
    t4.departure = "06:40 AM"; t4.totalSeats = 4;
    t4.availableSeats = 4; t4.fare = 1100.0f;

    // Insert in sorted order
    sys.trains.push_back(t1);
    sys.trains.push_back(t2);
    sys.trains.push_back(t3);
    sys.trains.push_back(t4);

    // Save each to database
    dbInsertTrain(t1);
    dbInsertTrain(t2);
    dbInsertTrain(t3);
    dbInsertTrain(t4);

    cout << "  Seeded 4 sample trains.\n";
}

// =============================================================================
// MAIN - entry point
// =============================================================================
int main() {
    // Connect to database (JSON file storage in backend/database/data/)
    if (!dbConnect("backend/database/data")) {
        cout << "Error: Could not open data directory.\n";
        return 1;
    }

    // Initialize the central RailwaySystem struct
    RailwaySystem sys;
    initSeatMap(sys.seatMap);

    // Load existing data from database files
    dbLoadTrains(sys.trains);
    dbLoadPassengers(sys.passengers);
    dbLoadWaiting(sys.waitingLists);

    // Seed default trains if database is empty
    if (sys.trains.empty()) {
        seedDefaultTrains(sys);
    }

    // Rebuild the 2D seat map from passenger records
    rebuildSeatMap(sys.seatMap, sys.trains, sys.passengers);

    cout << "\n  Loaded " << sys.trains.size() << " trains, "
         << sys.passengers.size() << " passengers from database.\n";

    // Menu loop (Module II: do-while + switch)
    int choice = -1;
    do {
        printMenu();
        choice = readInt("  Select option (0-9): ", 0, 9);
        handleChoice(sys, choice);
    } while (choice != 0);

    dbClose();
    return 0;
}
