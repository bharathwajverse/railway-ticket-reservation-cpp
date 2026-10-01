// =============================================================================
// seat_map.cpp - 2D Array helpers for seat management
// MODULE IV: Two-Dimensional Numeric Arrays, Matrix Operations
// =============================================================================
#include "structures.h"
#include "config.h"
#include "seat_map.h"
#include <string>
#include <sstream>

using namespace std;

// Time Complexity: O(MAX_TRAINS * MAX_SEATS)
// Purpose: Initialize all seats in 2D array to 0 (free)
void initSeatMap(int seatMap[MAX_TRAINS][MAX_SEATS]) {
    for (int i = 0; i < MAX_TRAINS; i++) {
        for (int j = 0; j < MAX_SEATS; j++) {
            seatMap[i][j] = 0;
        }
    }
}

// Time Complexity: O(totalSeats)
// Purpose: Find the first free seat (0) for a train, returning 1-based seat number
int findFirstFreeSeat(const int seatMap[MAX_TRAINS][MAX_SEATS],
                      int trainIdx, int totalSeats) {
    if (trainIdx < 0 || trainIdx >= MAX_TRAINS) return -1;
    for (int j = 0; j < totalSeats && j < MAX_SEATS; j++) {
        if (seatMap[trainIdx][j] == 0) {
            return j + 1; // 1-based seat number
        }
    }
    return -1;
}

// Time Complexity: O(totalSeats)
// Purpose: Count number of occupied seats (1) for a train
int countOccupiedSeats(const int seatMap[MAX_TRAINS][MAX_SEATS],
                       int trainIdx, int totalSeats) {
    if (trainIdx < 0 || trainIdx >= MAX_TRAINS) return 0;
    int count = 0;
    for (int j = 0; j < totalSeats && j < MAX_SEATS; j++) {
        if (seatMap[trainIdx][j] == 1) {
            count++;
        }
    }
    return count;
}

// Time Complexity: O(1)
// Purpose: Mark a specific seat as booked (1)
void markSeatBooked(int seatMap[MAX_TRAINS][MAX_SEATS],
                    int trainIdx, int seatNo) {
    if (trainIdx >= 0 && trainIdx < MAX_TRAINS && seatNo >= 1 && seatNo <= MAX_SEATS) {
        seatMap[trainIdx][seatNo - 1] = 1;
    }
}

// Time Complexity: O(1)
// Purpose: Mark a specific seat as free (0)
void markSeatFree(int seatMap[MAX_TRAINS][MAX_SEATS],
                  int trainIdx, int seatNo) {
    if (trainIdx >= 0 && trainIdx < MAX_TRAINS && seatNo >= 1 && seatNo <= MAX_SEATS) {
        seatMap[trainIdx][seatNo - 1] = 0;
    }
}

// Time Complexity: O(totalSeats)
// Purpose: Generate a visual string representation of the coach layout
string getSeatMapString(const int seatMap[MAX_TRAINS][MAX_SEATS],
                             int trainIdx, int totalSeats,
                             int trainNo, const string& trainName) {
    if (trainIdx < 0 || trainIdx >= MAX_TRAINS) return "Invalid train index.\n";
    stringstream ss;
    ss << "Coach Seat Map: Train " << trainNo << " - " << trainName << "\n";
    ss << "---------------------------------------------------------\n";
    for (int j = 0; j < totalSeats && j < MAX_SEATS; j++) {
        ss << "[" << (j + 1) << ":";
        if (seatMap[trainIdx][j] == 0) {
            ss << "_]";
        } else {
            ss << "X]";
        }
        if ((j + 1) % 6 == 0) {
            ss << "\n";
        } else {
            ss << " ";
        }
    }
    if (totalSeats % 6 != 0) {
        ss << "\n";
    }
    ss << "Legend: [_] = Available   [X] = Booked\n";
    return ss.str();
}

// Time Complexity: O(passengers * trains)
// Purpose: Rebuild the seat map from passenger records
void rebuildSeatMap(int seatMap[MAX_TRAINS][MAX_SEATS],
                    const vector<Train>& trains,
                    const vector<Passenger>& passengers) {
    initSeatMap(seatMap);
    for (size_t i = 0; i < passengers.size(); i++) {
        if (passengers[i].status == "CONFIRMED") {
            int trainIdx = -1;
            for (size_t j = 0; j < trains.size(); j++) {
                if (trains[j].trainNo == passengers[i].trainNo) {
                    trainIdx = (int)j;
                    break;
                }
            }
            if (trainIdx != -1) {
                markSeatBooked(seatMap, trainIdx, passengers[i].seatNo);
            }
        }
    }
}
