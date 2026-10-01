// =============================================================================
// seat_map.h - 2D Array helpers for seat management
// MODULE IV: Two-Dimensional Numeric Arrays, Matrix Operations
// =============================================================================
#ifndef SEAT_MAP_H
#define SEAT_MAP_H

#include <string>
#include "structures.h"

// Initialize entire seat map to 0 (all free). O(MAX_TRAINS * MAX_SEATS)
void initSeatMap(int seatMap[MAX_TRAINS][MAX_SEATS]);

// Find first free seat (0) in a row. Returns seat number (1-based), or -1. O(totalSeats)
int findFirstFreeSeat(const int seatMap[MAX_TRAINS][MAX_SEATS],
                      int trainIdx, int totalSeats);

// Count how many seats are booked (1) in a row. O(totalSeats)
int countOccupiedSeats(const int seatMap[MAX_TRAINS][MAX_SEATS],
                       int trainIdx, int totalSeats);

// Mark a seat as booked. O(1)
void markSeatBooked(int seatMap[MAX_TRAINS][MAX_SEATS],
                    int trainIdx, int seatNo);

// Mark a seat as free. O(1)
void markSeatFree(int seatMap[MAX_TRAINS][MAX_SEATS],
                  int trainIdx, int seatNo);

// Build seat layout as formatted string (for display). O(totalSeats)
std::string getSeatMapString(const int seatMap[MAX_TRAINS][MAX_SEATS],
                             int trainIdx, int totalSeats,
                             int trainNo, const std::string& trainName);

// Rebuild the entire seat map from passenger records. O(passengers * trains)
void rebuildSeatMap(int seatMap[MAX_TRAINS][MAX_SEATS],
                    const std::vector<Train>& trains,
                    const std::vector<Passenger>& passengers);

#endif // SEAT_MAP_H
