// =============================================================================
// booking_ops.cpp - Booking, cancellation, and passenger operations
// MODULE VIII: Stacks and LIFO operations
// MODULE IX: Queues and FIFO promotion operations
// MODULE VI: Nested structures and functions
// =============================================================================
#include "structures.h"
#include "config.h"
#include "booking_ops.h"
#include "train_ops.h"
#include "seat_map.h"
#include "waiting_queue.h"
#include "validation.h"
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace std;

// Time Complexity: O(N) where N is passengers count
// Purpose: Generate next PNR (starts at 1001, highest + 1)
int generatePNR(const vector<Passenger>& passengers) {
    int maxPnr = 1000;
    for (size_t i = 0; i < passengers.size(); i++) {
        if (passengers[i].pnr > maxPnr) {
            maxPnr = passengers[i].pnr;
        }
    }
    return maxPnr + 1;
}

// Time Complexity: O(T * W) where T is trains, W is average waiting queue size
// Purpose: Generate next Wait ID (starts at 1, highest + 1)
int generateWaitId(const map<int, queue<WaitingEntry> >& waitingLists) {
    int maxId = 0;
    for (map<int, queue<WaitingEntry> >::const_iterator it = waitingLists.begin(); it != waitingLists.end(); ++it) {
        queue<WaitingEntry> tempQueue = it->second;
        while (!tempQueue.empty()) {
            if (tempQueue.front().waitId > maxId) {
                maxId = tempQueue.front().waitId;
            }
            tempQueue.pop();
        }
    }
    return maxId + 1;
}

// Time Complexity: O(N) where N is passengers count
// Purpose: Check for duplicate confirmed booking for same train, date, and passenger
bool isDuplicateBooking(const vector<Passenger>& passengers, int trainNo, const string& name, Date date) {
    string lowerName = toLowerCase(name);
    for (size_t i = 0; i < passengers.size(); i++) {
        if (passengers[i].trainNo == trainNo &&
            passengers[i].status == "CONFIRMED" &&
            passengers[i].travelDate.day == date.day &&
            passengers[i].travelDate.month == date.month &&
            passengers[i].travelDate.year == date.year) {
            
            if (toLowerCase(passengers[i].name) == lowerName) {
                return true;
            }
        }
    }
    return false;
}

// Time Complexity: O(N + log T)
// Purpose: Book a ticket (allocates seat if available, else adds to waiting queue)
BookingResult bookTicket(RailwaySystem& sys, int trainNo, const string& name, int age, char gender, Date travelDate) {
    BookingResult res;
    res.success = false;
    res.status = "ERROR";
    res.waitPosition = 0;

    int trainIdx = binarySearchTrain(sys.trains, trainNo);
    if (trainIdx == -1) {
        res.errorMsg = "Train not found.";
        return res;
    }

    if (isDuplicateBooking(sys.passengers, trainNo, name, travelDate)) {
        res.errorMsg = "Duplicate booking: Passenger is already confirmed on this train for the same date.";
        return res;
    }

    Train& train = sys.trains[trainIdx];

    if (train.availableSeats > 0) {
        int seatNo = findFirstFreeSeat(sys.seatMap, trainIdx, train.totalSeats);
        if (seatNo != -1) {
            markSeatBooked(sys.seatMap, trainIdx, seatNo);

            Passenger p;
            p.pnr = generatePNR(sys.passengers);
            p.name = name;
            p.age = age;
            p.gender = gender;
            p.trainNo = trainNo;
            p.seatNo = seatNo;
            p.travelDate = travelDate;
            p.status = "CONFIRMED";

            sys.passengers.push_back(p);
            train.availableSeats--;

            res.success = true;
            res.status = "CONFIRMED";
            res.passenger = p;
            return res;
        }
    }

    // If no seats available, try waiting list
    if (waitingQueueSize(sys.waitingLists, trainNo) < MAX_WAITING) {
        WaitingEntry w;
        w.waitId = generateWaitId(sys.waitingLists);
        w.name = name;
        w.age = age;
        w.gender = gender;
        w.trainNo = trainNo;
        w.travelDate = travelDate;

        enqueueWaiting(sys.waitingLists, w);

        res.success = true;
        res.status = "WAITING";
        res.waitEntry = w;
        res.waitPosition = waitingQueueSize(sys.waitingLists, trainNo);
        return res;
    }

    res.errorMsg = "Train is completely booked and waiting list is full.";
    return res;
}

// Time Complexity: O(N + log T)
// Purpose: Cancel a confirmed ticket by PNR and auto-promote first waiting passenger
CancelResult cancelTicket(RailwaySystem& sys, int pnr) {
    CancelResult res;
    res.success = false;
    res.promoted = false;

    for (size_t i = 0; i < sys.passengers.size(); i++) {
        if (sys.passengers[i].pnr == pnr) {
            if (sys.passengers[i].status != "CONFIRMED") {
                res.errorMsg = "Ticket is already cancelled.";
                return res;
            }

            sys.passengers[i].status = "CANCELLED";
            res.cancelled = sys.passengers[i];
            int trainNo = sys.passengers[i].trainNo;
            int seatNo = sys.passengers[i].seatNo;
            int trainIdx = binarySearchTrain(sys.trains, trainNo);

            if (trainIdx != -1) {
                markSeatFree(sys.seatMap, trainIdx, seatNo);

                // Auto-promote from FIFO waiting queue if one exists
                if (!isWaitingQueueEmpty(sys.waitingLists, trainNo)) {
                    WaitingEntry w;
                    if (dequeueWaiting(sys.waitingLists, trainNo, w)) {
                        Passenger p;
                        p.pnr = generatePNR(sys.passengers);
                        p.name = w.name;
                        p.age = w.age;
                        p.gender = w.gender;
                        p.trainNo = w.trainNo;
                        p.travelDate = w.travelDate;
                        p.seatNo = seatNo;
                        p.status = "CONFIRMED";

                        markSeatBooked(sys.seatMap, trainIdx, p.seatNo);
                        sys.passengers.push_back(p);

                        res.promoted = true;
                        res.promotedPassenger = p;
                        res.promotedWaitId = w.waitId;
                    }
                } else {
                    sys.trains[trainIdx].availableSeats++;
                }
            }

            res.success = true;
            return res;
        }
    }

    res.errorMsg = "PNR not found.";
    return res;
}

// Time Complexity: O(N)
// Purpose: Filter passengers by train number
vector<Passenger> getPassengersByTrain(const vector<Passenger>& passengers, int trainNo) {
    vector<Passenger> res;
    for (size_t i = 0; i < passengers.size(); i++) {
        if (passengers[i].trainNo == trainNo) {
            res.push_back(passengers[i]);
        }
    }
    return res;
}

// Time Complexity: O(N)
// Purpose: Find passenger by PNR
vector<Passenger> getPassengerByPNR(const vector<Passenger>& passengers, int pnr) {
    vector<Passenger> res;
    for (size_t i = 0; i < passengers.size(); i++) {
        if (passengers[i].pnr == pnr) {
            res.push_back(passengers[i]);
            break;
        }
    }
    return res;
}

// Time Complexity: O(1)
// Purpose: Format passenger details as string
string formatPassengerInfo(const Passenger& p) {
    stringstream ss;
    ss << "  PNR: " << p.pnr 
       << " | Name: " << p.name 
       << " | Age: " << p.age 
       << " | Gender: " << p.gender 
       << " | Train: " << p.trainNo 
       << " | Seat: " << p.seatNo 
       << " | Date: " << p.travelDate.day << "/" << p.travelDate.month << "/" << p.travelDate.year
       << " | Status: " << p.status;
    return ss.str();
}

// Time Complexity: O(1)
// Purpose: Format ticket receipt slip
string formatTicket(const Passenger& p, const Train& t) {
    stringstream ss;
    ss << "\n  +---------------------------------------------------+\n";
    ss << "  |             INDIAN RAILWAYS E-TICKET              |\n";
    ss << "  +---------------------------------------------------+\n";
    ss << "  | PNR Number     : " << left << setw(32) << p.pnr << "|\n";
    ss << "  | Passenger Name : " << left << setw(32) << p.name << "|\n";
    ss << "  | Age / Gender   : " << left << setw(3) << p.age << " / " << setw(27) << p.gender << "|\n";
    ss << "  | Train Number   : " << left << setw(6) << t.trainNo << " - " << setw(23) << t.name << "|\n";
    ss << "  | Route          : " << left << setw(13) << t.source << " -> " << setw(15) << t.destination << "|\n";
    ss << "  | Departure Time : " << left << setw(32) << t.departure << "|\n";
    ss << "  | Travel Date    : " << p.travelDate.day << "/" << p.travelDate.month << "/" << left << setw(26) << p.travelDate.year << "|\n";
    ss << "  | Seat Number    : " << left << setw(32) << p.seatNo << "|\n";
    ss << "  | Fare           : Rs. " << fixed << setprecision(2) << left << setw(27) << t.fare << "|\n";
    ss << "  | Status         : " << left << setw(32) << p.status << "|\n";
    ss << "  +---------------------------------------------------+\n";
    return ss.str();
}
