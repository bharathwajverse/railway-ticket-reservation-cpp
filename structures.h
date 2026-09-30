#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <string>
#include <vector>
#include <map>
#include <queue>

using namespace std;

// Maximum capacity constants
const int MAX_TRAINS = 20;
const int MAX_SEATS = 60;
const int MAX_WAITING = 10;

// Nested date structure for passenger travel date
struct Date {
    int day;
    int month;
    int year;
};

// Train entity structure
struct Train {
    int trainNo;
    string name;
    string source;
    string destination;
    string departure;
    int totalSeats;
    int availableSeats;
    float fare;
};

// Passenger ticket entity structure
struct Passenger {
    int pnr;
    string name;
    int age;
    char gender;
    int trainNo;
    int seatNo;
    Date travelDate;
    string status; // "CONFIRMED" or "CANCELLED"
};

// Waiting list entry structure
struct WaitingEntry {
    int waitId;
    string name;
    int age;
    char gender;
    int trainNo;
    Date travelDate;
};

// In-memory working system holding trains, passengers, waiting queues, and 2D seat map
struct RailwaySystem {
    vector<Train> trains;
    vector<Passenger> passengers;
    // Map key = trainNo, value = FIFO queue of waiting passengers
    map<int, queue<WaitingEntry> > waitingLists;
    // 2D seat map: 0 = free, 1 = booked. Row = index of train in trains vector
    int seatMap[MAX_TRAINS][MAX_SEATS];
};

#endif // STRUCTURES_H
