// =============================================================================
// server.cpp - REST API Web Server Entry Point
// Initializes data, registers endpoints, mounts static frontend, and listens
// =============================================================================
#include "third_party/httplib.h"
#include "routes.h"
#include "structures.h"
#include "db_connection.h"
#include "train_repo.h"
#include "passenger_repo.h"
#include "waiting_repo.h"
#include "seat_map.h"
#include <iostream>
#include <cstdlib>

using namespace std;

// Seed 4 sample trains if database is empty
static void seedDefaultTrains(RailwaySystem& sys) {
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

    sys.trains.push_back(t1);
    sys.trains.push_back(t2);
    sys.trains.push_back(t3);
    sys.trains.push_back(t4);

    dbInsertTrain(t1);
    dbInsertTrain(t2);
    dbInsertTrain(t3);
    dbInsertTrain(t4);
}

int main() {
    cout << "=======================================================\n"
         << "     RAILWAY RESERVATION REST API & WEB SERVER        \n"
         << "=======================================================\n";

    // Connect to database
    if (!dbConnect("backend/database/data")) {
        // Fallback to local path if executed from backend/ directory
        if (!dbConnect("database/data")) {
            cout << "Warning: Could not open database directory.\n";
        }
    }

    // Initialize Railway System
    RailwaySystem sys;
    initSeatMap(sys.seatMap);

    // Load persisted records
    dbLoadTrains(sys.trains);
    dbLoadPassengers(sys.passengers);
    dbLoadWaiting(sys.waitingLists);

    if (sys.trains.empty()) {
        seedDefaultTrains(sys);
    }

    // Reconstruct 2D seat map
    rebuildSeatMap(sys.seatMap, sys.trains, sys.passengers);

    cout << "[DB] Loaded " << sys.trains.size() << " trains, "
         << sys.passengers.size() << " passengers into memory.\n";

    httplib::Server svr;

    // Register all REST API endpoints
    registerRoutes(svr, sys);

    // Mount frontend static files
    // Check multiple relative locations depending on where binary is executed from
    if (!svr.set_mount_point("/", "./frontend")) {
        svr.set_mount_point("/", "../frontend");
    }

    // Determine port from environment or default to 8080
    int port = 8080;
    const char* envPort = getenv("API_PORT");
    if (envPort != nullptr) {
        port = atoi(envPort);
        if (port <= 0) port = 8080;
    }

    cout << "[Server] REST API & Web Dashboard live at: http://localhost:" << port << "\n";
    cout << "[Server] Press Ctrl+C to stop.\n";

    if (!svr.listen("0.0.0.0", port)) {
        cerr << "Error: Could not bind to port " << port << ".\n";
        return 1;
    }

    dbClose();
    return 0;
}
