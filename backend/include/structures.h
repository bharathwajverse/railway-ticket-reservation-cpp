// =============================================================================
// structures.h - All data structures (structs) for the Railway System
// MODULE VI: Structures - declaration, initialization, nested structs
// MODULE X: STL Containers - vector, map, queue used in RailwaySystem
// =============================================================================
#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <string>
#include <vector>
#include <map>
#include <queue>
#include "config.h"

// --- Nested structure: Date embedded inside Passenger and WaitingEntry ---
struct Date {
    int day;
    int month;
    int year;
};

// --- Train information structure ---
struct Train {
    int trainNo;
    std::string name;
    std::string source;
    std::string destination;
    std::string departure;
    int totalSeats;
    int availableSeats;
    float fare;
};

// --- Passenger booking record ---
struct Passenger {
    int pnr;
    std::string name;
    int age;
    char gender;         // 'M', 'F', or 'O'
    int trainNo;
    int seatNo;
    Date travelDate;     // Nested structure
    std::string status;  // "CONFIRMED" or "CANCELLED"
};

// --- Waiting list entry ---
struct WaitingEntry {
    int waitId;
    std::string name;
    int age;
    char gender;
    int trainNo;
    Date travelDate;     // Nested structure
};

// --- Result of a booking attempt (returned by bookTicket) ---
struct BookingResult {
    bool success;
    std::string status;      // "CONFIRMED", "WAITING", or "ERROR"
    Passenger passenger;     // Filled if CONFIRMED
    WaitingEntry waitEntry;  // Filled if WAITING
    int waitPosition;        // Position in queue (1-based)
    std::string errorMsg;    // Filled if ERROR
};

// --- Result of a cancellation (returned by cancelTicket) ---
struct CancelResult {
    bool success;
    Passenger cancelled;         // The cancelled passenger
    bool promoted;               // True if someone was promoted from waiting
    Passenger promotedPassenger; // The promoted passenger (if any)
    int promotedWaitId;          // Wait ID of the promoted passenger
    std::string errorMsg;
};

// =============================================================================
// RailwaySystem - The central data holder
// Uses: vector (Module X), map (Module X), queue (Module IX), 2D array (Module IV)
// =============================================================================
struct RailwaySystem {
    std::vector<Train> trains;                              // Dynamic array of trains
    std::vector<Passenger> passengers;                      // Dynamic array of passengers
    std::map<int, std::queue<WaitingEntry> > waitingLists;  // trainNo -> waiting queue
    int seatMap[MAX_TRAINS][MAX_SEATS];                     // 2D array: 0=free, 1=booked
};

#endif // STRUCTURES_H
