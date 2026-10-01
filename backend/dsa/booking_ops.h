// =============================================================================
// booking_ops.h - Booking, cancellation, and passenger operations
// MODULE VIII: Stack (undo cancellation via LIFO)
// MODULE IX: Queue (waiting list promotion via FIFO)
// =============================================================================
#ifndef BOOKING_OPS_H
#define BOOKING_OPS_H

#include <vector>
#include <string>
#include "structures.h"

// Generate next PNR number (highest existing + 1, starting at 1001). O(n)
int generatePNR(const std::vector<Passenger>& passengers);

// Generate next waiting list ID (highest existing + 1, starting at 1). O(total entries)
int generateWaitId(const std::map<int, std::queue<WaitingEntry> >& waitingLists);

// Check for duplicate booking (same name, train, date, CONFIRMED). O(n)
bool isDuplicateBooking(const std::vector<Passenger>& passengers,
                        int trainNo, const std::string& name, Date date);

// Book a ticket. Returns BookingResult with status and details. O(n)
BookingResult bookTicket(RailwaySystem& sys,
                         int trainNo, const std::string& name,
                         int age, char gender, Date travelDate);

// Cancel a ticket by PNR. Auto-promotes from waiting queue. O(n)
CancelResult cancelTicket(RailwaySystem& sys, int pnr);

// Get passengers filtered by train number. O(n)
std::vector<Passenger> getPassengersByTrain(const std::vector<Passenger>& passengers,
                                            int trainNo);

// Get passenger by PNR. Returns empty vector if not found. O(n)
std::vector<Passenger> getPassengerByPNR(const std::vector<Passenger>& passengers,
                                         int pnr);

// Format a passenger's info as a string. O(1)
std::string formatPassengerInfo(const Passenger& p);

// Format a ticket receipt as a string. O(1)
std::string formatTicket(const Passenger& p, const Train& t);

#endif // BOOKING_OPS_H
