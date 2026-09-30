#include "railway.h"
#include "database.h"
#include "utils.h"
#include <iostream>
#include <iomanip>

using namespace std;

// Seeds initial sample trains with small capacities (3 to 5 seats) for easy demoing
// Time Complexity: O(1)
void seedSampleTrains(RailwaySystem& sys, sqlite3* db) {
    if (!sys.trains.empty()) {
        return;
    }

    Train t1 = { 10101, "Rajdhani Express", "Delhi", "Mumbai", "06:00", 4, 4, 1500.0f };
    Train t2 = { 10202, "Vande Bharat", "Chennai", "Bangalore", "05:45", 5, 5, 950.0f };
    Train t3 = { 10303, "Shatabdi Express", "Kolkata", "Patna", "07:15", 3, 3, 750.0f };
    Train t4 = { 10404, "Tejas Express", "Ahmedabad", "Mumbai", "15:30", 4, 4, 1100.0f };

    Train sampleList[] = { t1, t2, t3, t4 };
    for (int i = 0; i < 4; i++) {
        insertTrain(db, sampleList[i]);
        sys.trains.push_back(sampleList[i]);
    }
}

// Loads system state from database and reconstructs the in-memory 2D seat grid
// Time Complexity: O(T + P + W)
void loadSystem(RailwaySystem& sys, sqlite3* db) {
    // Clear in-memory seat map
    for (int i = 0; i < MAX_TRAINS; i++) {
        for (int j = 0; j < MAX_SEATS; j++) {
            sys.seatMap[i][j] = 0;
        }
    }

    loadTrains(db, sys.trains);
    seedSampleTrains(sys, db);
    loadPassengers(db, sys.passengers);
    loadWaiting(db, sys.waitingLists);

    // Rebuild 2D seatMap from confirmed passengers
    for (size_t i = 0; i < sys.passengers.size(); i++) {
        if (sys.passengers[i].status == "CONFIRMED") {
            int trainIdx = findTrainIndex(sys, sys.passengers[i].trainNo);
            int seatIdx = sys.passengers[i].seatNo - 1;
            if (trainIdx >= 0 && trainIdx < MAX_TRAINS && seatIdx >= 0 && seatIdx < MAX_SEATS) {
                sys.seatMap[trainIdx][seatIdx] = 1; // 1 = booked
            }
        }
    }
}

// Iterative Binary Search on trains sorted by trainNo
// Time Complexity: O(log N) where N is number of trains
int binarySearchTrain(const vector<Train>& trains, int trainNo) {
    int low = 0;
    int high = (int)trains.size() - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (trains[mid].trainNo == trainNo) {
            return mid;
        } else if (trains[mid].trainNo < trainNo) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return -1; // Train not found
}

// Finds train index in vector using binary search
// Time Complexity: O(log N)
int findTrainIndex(const RailwaySystem& sys, int trainNo) {
    return binarySearchTrain(sys.trains, trainNo);
}

// Adds a new train, keeping the trains vector sorted by trainNo for binary search
// Time Complexity: O(N) due to sorted vector insertion
void addTrain(RailwaySystem& sys, sqlite3* db) {
    if ((int)sys.trains.size() >= MAX_TRAINS) {
        cout << "\n[Error] Cannot add train. System reached maximum capacity of " << MAX_TRAINS << " trains.\n";
        return;
    }

    cout << "\n--- Add New Train ---\n";
    int trainNo = readInt("Enter Train Number (1000 - 99999): ", 1000, 99999);

    if (findTrainIndex(sys, trainNo) != -1) {
        cout << "[Error] Train number " << trainNo << " already exists! Duplicates are not allowed.\n";
        return;
    }

    Train t;
    t.trainNo = trainNo;
    t.name = readName("Enter Train Name: ");
    t.source = readName("Enter Source Station: ");
    t.destination = readName("Enter Destination Station: ");

    cout << "Enter Departure Time (e.g. 09:30 AM): ";
    getline(cin, t.departure);
    if (t.departure.empty()) {
        t.departure = "12:00 PM";
    }

    t.totalSeats = readInt("Enter Total Seats (1 - " + to_string(MAX_SEATS) + "): ", 1, MAX_SEATS);
    t.availableSeats = t.totalSeats;
    t.fare = readFloat("Enter Ticket Fare in INR (10.0 - 10000.0): ", 10.0f, 10000.0f);

    // Find sorted insertion position
    int pos = 0;
    while (pos < (int)sys.trains.size() && sys.trains[pos].trainNo < t.trainNo) {
        pos++;
    }

    // Shift seatMap rows to maintain index alignment
    for (int i = (int)sys.trains.size(); i > pos; i--) {
        for (int s = 0; s < MAX_SEATS; s++) {
            sys.seatMap[i][s] = sys.seatMap[i - 1][s];
        }
    }
    // Initialize seats for the new train
    for (int s = 0; s < MAX_SEATS; s++) {
        sys.seatMap[pos][s] = 0;
    }

    sys.trains.insert(sys.trains.begin() + pos, t);

    if (insertTrain(db, t)) {
        cout << "\n[Success] Train " << t.trainNo << " - " << t.name << " added and saved to database successfully!\n";
    }
}

// Displays all trains in a formatted table
// Time Complexity: O(N)
void displayTrains(const RailwaySystem& sys) {
    if (sys.trains.empty()) {
        cout << "\n[Notice] No trains available in the system.\n";
        return;
    }

    cout << "\n========================================================================================================\n";
    cout << setw(8)  << "Train No"
         << setw(22) << "Train Name"
         << setw(16) << "Source"
         << setw(16) << "Destination"
         << setw(12) << "Departure"
         << setw(8)  << "Total"
         << setw(10) << "Available"
         << setw(10) << "Fare (INR)" << "\n";
    cout << "========================================================================================================\n";

    for (size_t i = 0; i < sys.trains.size(); i++) {
        const Train& t = sys.trains[i];
        cout << setw(8)  << t.trainNo
             << setw(22) << t.name
             << setw(16) << t.source
             << setw(16) << t.destination
             << setw(12) << t.departure
             << setw(8)  << t.totalSeats
             << setw(10) << t.availableSeats
             << setw(10) << fixed << setprecision(2) << t.fare << "\n";
    }
    cout << "========================================================================================================\n";
}

// Searches train by train number using Binary Search
// Time Complexity: O(log N)
void searchTrainByNumber(const RailwaySystem& sys) {
    int trainNo = readInt("\nEnter Train Number to search: ", 1000, 99999);
    int idx = findTrainIndex(sys, trainNo);

    if (idx == -1) {
        cout << "[Notice] Train number " << trainNo << " was not found.\n";
        return;
    }

    const Train& t = sys.trains[idx];
    cout << "\n--- Train Details Found (via Binary Search) ---\n";
    cout << "Train Number    : " << t.trainNo << "\n";
    cout << "Train Name      : " << t.name << "\n";
    cout << "Route           : " << t.source << " -> " << t.destination << "\n";
    cout << "Departure Time  : " << t.departure << "\n";
    cout << "Available Seats : " << t.availableSeats << " / " << t.totalSeats << "\n";
    cout << "Fare per ticket : Rs. " << fixed << setprecision(2) << t.fare << "\n";
}

// Searches trains by destination using Linear Search and substring matching
// Time Complexity: O(N * M)
void searchTrainByDestination(const RailwaySystem& sys) {
    string dest = readName("\nEnter Destination Station to search: ");
    bool found = false;

    cout << "\n--- Trains matching destination: \"" << dest << "\" (via Linear Search) ---\n";
    for (size_t i = 0; i < sys.trains.size(); i++) {
        if (containsIgnoreCase(sys.trains[i].destination, dest)) {
            if (!found) {
                cout << setw(8)  << "Train No"
                     << setw(22) << "Train Name"
                     << setw(16) << "Source"
                     << setw(16) << "Destination"
                     << setw(10) << "Available"
                     << setw(10) << "Fare" << "\n";
                cout << "--------------------------------------------------------------------------------\n";
                found = true;
            }
            const Train& t = sys.trains[i];
            cout << setw(8)  << t.trainNo
                 << setw(22) << t.name
                 << setw(16) << t.source
                 << setw(16) << t.destination
                 << setw(10) << t.availableSeats
                 << setw(10) << fixed << setprecision(2) << t.fare << "\n";
        }
    }

    if (!found) {
        cout << "[Notice] No trains found traveling to \"" << dest << "\".\n";
    }
}

// Bubble sort on a copy of trains vector for display purposes only
// Sorting a copy keeps main trains vector sorted by trainNo so binary search remains valid
// Time Complexity: O(N^2)
void sortTrains(RailwaySystem& sys) {
    if (sys.trains.empty()) {
        cout << "\n[Notice] No trains to sort.\n";
        return;
    }

    cout << "\n--- Sort Trains (for Display) ---\n";
    cout << "1. Sort by Ticket Fare (Lowest to Highest)\n";
    cout << "2. Sort by Train Name (Alphabetical A-Z)\n";
    int choice = readInt("Select sorting criteria (1 or 2): ", 1, 2);

    // Make a copy so main vector remains sorted by trainNo
    vector<Train> copyList = sys.trains;
    int n = (int)copyList.size();

    // Manual Bubble Sort
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            bool shouldSwap = false;
            if (choice == 1) {
                if (copyList[j].fare > copyList[j + 1].fare) {
                    shouldSwap = true;
                }
            } else {
                if (toLowerCase(copyList[j].name) > toLowerCase(copyList[j + 1].name)) {
                    shouldSwap = true;
                }
            }
            if (shouldSwap) {
                Train temp = copyList[j];
                copyList[j] = copyList[j + 1];
                copyList[j + 1] = temp;
            }
        }
    }

    RailwaySystem tempSys;
    tempSys.trains = copyList;
    cout << "\n[Displaying sorted list of trains]:\n";
    displayTrains(tempSys);
}

// Scans 2D seatMap to locate the first free seat for a train
// Time Complexity: O(S) where S is total seats
int findFreeSeat(const RailwaySystem& sys, int trainIndex) {
    int total = sys.trains[trainIndex].totalSeats;
    for (int s = 0; s < total; s++) {
        if (sys.seatMap[trainIndex][s] == 0) {
            return s + 1; // 1-based seat number
        }
    }
    return -1;
}

// Generates next unique PNR (max PNR + 1 starting from 1001)
// Time Complexity: O(P) where P is number of passengers
int generatePNR(const RailwaySystem& sys) {
    int maxPnr = 1000;
    for (size_t i = 0; i < sys.passengers.size(); i++) {
        if (sys.passengers[i].pnr > maxPnr) {
            maxPnr = sys.passengers[i].pnr;
        }
    }
    return maxPnr + 1;
}

// Books a ticket, allocating seat via 2D seat map or placing passenger in FIFO waiting queue
// Time Complexity: O(P + log N)
void bookTicket(RailwaySystem& sys, sqlite3* db) {
    cout << "\n--- Book a Ticket ---\n";
    int trainNo = readInt("Enter Train Number: ", 1000, 99999);
    int trainIdx = findTrainIndex(sys, trainNo);

    if (trainIdx == -1) {
        cout << "[Error] Train number " << trainNo << " does not exist.\n";
        return;
    }

    string name = readName("Enter Passenger Name: ");
    int age = readInt("Enter Age (1 - 120): ", 1, 120);
    char gender = readGender("Enter Gender (M/F/O): ");
    Date travelDate = readDate("Enter Date of Travel:");

    // Check duplicate: Same passenger already CONFIRMED on the same train & date
    for (size_t i = 0; i < sys.passengers.size(); i++) {
        const Passenger& p = sys.passengers[i];
        if (p.status == "CONFIRMED" && p.trainNo == trainNo &&
            toLowerCase(p.name) == toLowerCase(name) &&
            p.travelDate.day == travelDate.day &&
            p.travelDate.month == travelDate.month &&
            p.travelDate.year == travelDate.year) {
            cout << "\n[Error] Duplicate booking! " << name << " is already confirmed on train "
                 << trainNo << " on this travel date.\n";
            return;
        }
    }

    int freeSeat = findFreeSeat(sys, trainIdx);

    // Case 1: Seats available
    if (freeSeat != -1 && sys.trains[trainIdx].availableSeats > 0) {
        int seatIdx = freeSeat - 1;
        sys.seatMap[trainIdx][seatIdx] = 1; // Mark booked
        sys.trains[trainIdx].availableSeats--;

        updateTrainSeats(db, trainNo, sys.trains[trainIdx].availableSeats);

        Passenger p;
        p.pnr = generatePNR(sys);
        p.name = name;
        p.age = age;
        p.gender = gender;
        p.trainNo = trainNo;
        p.seatNo = freeSeat;
        p.travelDate = travelDate;
        p.status = "CONFIRMED";

        sys.passengers.push_back(p);
        insertPassenger(db, p);

        cout << "\n========================================================\n";
        cout << "               TICKET BOOKED SUCCESSFULLY!             \n";
        cout << "========================================================\n";
        cout << "PNR Number       : " << p.pnr << "\n";
        cout << "Passenger Name   : " << p.name << " (Age: " << p.age << ", Gender: " << p.gender << ")\n";
        cout << "Train            : " << sys.trains[trainIdx].name << " (#" << trainNo << ")\n";
        cout << "Seat Number      : " << p.seatNo << "\n";
        cout << "Travel Date      : " << p.travelDate.day << "/" << p.travelDate.month << "/" << p.travelDate.year << "\n";
        cout << "Fare Charged     : Rs. " << fixed << setprecision(2) << sys.trains[trainIdx].fare << "\n";
        cout << "Status           : CONFIRMED\n";
        cout << "========================================================\n";
    }
    // Case 2: Train is full -> Queue in waiting list
    else {
        cout << "\n[Notice] Train #" << trainNo << " is fully booked! (Available Seats: 0)\n";
        
        queue<WaitingEntry>& wQueue = sys.waitingLists[trainNo];
        if ((int)wQueue.size() >= MAX_WAITING) {
            cout << "[Error] Waiting list for train " << trainNo << " is also FULL (max " << MAX_WAITING << " passengers). Cannot book.\n";
            return;
        }

        WaitingEntry w;
        w.name = name;
        w.age = age;
        w.gender = gender;
        w.trainNo = trainNo;
        w.travelDate = travelDate;

        insertWaiting(db, w);
        wQueue.push(w);

        cout << "========================================================\n";
        cout << "        ADDED TO WAITING LIST (FIFO QUEUE)              \n";
        cout << "========================================================\n";
        cout << "Passenger Name   : " << w.name << "\n";
        cout << "Train            : #" << trainNo << " (" << sys.trains[trainIdx].name << ")\n";
        cout << "Waiting Position : WL-" << wQueue.size() << "\n";
        cout << "Status           : WAITING\n";
        cout << "Note: If any confirmed passenger cancels, you will be\n"
             << "      automatically promoted in First-Come, First-Served order.\n";
        cout << "========================================================\n";
    }
}

// Linear search to find passenger by PNR
// Time Complexity: O(P)
int findPassengerByPNR(const RailwaySystem& sys, int pnr) {
    for (size_t i = 0; i < sys.passengers.size(); i++) {
        if (sys.passengers[i].pnr == pnr) {
            return (int)i;
        }
    }
    return -1;
}

// Automatically promotes the first waiting passenger in the queue into a freed seat
// Time Complexity: O(1)
bool promoteFromWaitingList(RailwaySystem& sys, sqlite3* db, int trainIndex, int seatNo) {
    int trainNo = sys.trains[trainIndex].trainNo;
    queue<WaitingEntry>& wQueue = sys.waitingLists[trainNo];

    if (!wQueue.empty()) {
        WaitingEntry topWait = wQueue.front();
        wQueue.pop();

        deleteWaiting(db, topWait.waitId);

        Passenger promoted;
        promoted.pnr = generatePNR(sys);
        promoted.name = topWait.name;
        promoted.age = topWait.age;
        promoted.gender = topWait.gender;
        promoted.trainNo = topWait.trainNo;
        promoted.seatNo = seatNo;
        promoted.travelDate = topWait.travelDate;
        promoted.status = "CONFIRMED";

        sys.seatMap[trainIndex][seatNo - 1] = 1; // Mark booked
        sys.passengers.push_back(promoted);
        insertPassenger(db, promoted);

        cout << "\n>>> [AUTO-PROMOTION EVENT] <<<\n";
        cout << "Waiting passenger " << promoted.name << " was promoted to Seat #" << seatNo
             << " with newly generated PNR: " << promoted.pnr << "!\n";
        return true;
    } else {
        // Queue was empty, seat remains available
        sys.trains[trainIndex].availableSeats++;
        updateTrainSeats(db, trainNo, sys.trains[trainIndex].availableSeats);
        return false;
    }
}

// Cancels a ticket by PNR and triggers queue promotion if passengers are waiting
// Time Complexity: O(P + log N)
void cancelTicket(RailwaySystem& sys, sqlite3* db) {
    cout << "\n--- Cancel Ticket ---\n";
    int pnr = readInt("Enter PNR number to cancel: ", 1000, 999999);
    int pIdx = findPassengerByPNR(sys, pnr);

    if (pIdx == -1) {
        cout << "[Error] No ticket found matching PNR " << pnr << ".\n";
        return;
    }

    Passenger& p = sys.passengers[pIdx];
    if (p.status == "CANCELLED") {
        cout << "[Notice] Ticket with PNR " << pnr << " is ALREADY CANCELLED.\n";
        return;
    }

    cout << "\nTicket Details:\n";
    cout << "  PNR: " << p.pnr << " | Passenger: " << p.name << " | Train: " << p.trainNo << " | Seat: " << p.seatNo << "\n";
    
    cout << "Are you sure you want to cancel this ticket? (Y/N): ";
    string confirm;
    getline(cin, confirm);
    if (confirm.empty() || (confirm[0] != 'y' && confirm[0] != 'Y')) {
        cout << "[Notice] Cancellation aborted by user.\n";
        return;
    }

    p.status = "CANCELLED";
    updatePassengerStatus(db, pnr, "CANCELLED");

    int trainIdx = findTrainIndex(sys, p.trainNo);
    int freedSeat = p.seatNo;
    sys.seatMap[trainIdx][freedSeat - 1] = 0; // Free seat

    cout << "\n[Success] Ticket PNR " << pnr << " has been CANCELLED successfully.\n";

    // Auto-promote waiting passenger if one exists
    promoteFromWaitingList(sys, db, trainIdx, freedSeat);
}

// Displays available seat count and visual 2D seating layout for a train
// Time Complexity: O(S) where S is total seats
void displayAvailableSeats(const RailwaySystem& sys) {
    int trainNo = readInt("\nEnter Train Number: ", 1000, 99999);
    int trainIdx = findTrainIndex(sys, trainNo);

    if (trainIdx == -1) {
        cout << "[Error] Train #" << trainNo << " does not exist.\n";
        return;
    }

    const Train& t = sys.trains[trainIdx];
    int bookedSeats = t.totalSeats - t.availableSeats;

    cout << "\n=======================================================\n";
    cout << " Seat Availability for Train " << t.trainNo << " (" << t.name << ")\n";
    cout << "=======================================================\n";
    cout << " Total Seats     : " << t.totalSeats << "\n";
    cout << " Booked Seats    : " << bookedSeats << "\n";
    cout << " Available Seats : " << t.availableSeats << "\n";
    cout << "-------------------------------------------------------\n";
    cout << " 2D Seat Map Layout ([XX] = Booked, [ 1] = Available):\n\n";

    for (int s = 0; s < t.totalSeats; s++) {
        if (sys.seatMap[trainIdx][s] == 1) {
            cout << "[ XX ] ";
        } else {
            cout << "[ " << setw(2) << (s + 1) << " ] ";
        }
        if ((s + 1) % 6 == 0) {
            cout << "\n";
        }
    }
    if (t.totalSeats % 6 != 0) {
        cout << "\n";
    }
    cout << "=======================================================\n";
}

// Displays passenger details by PNR, per train, or all passengers
// Time Complexity: O(P)
void displayPassengerDetails(const RailwaySystem& sys) {
    if (sys.passengers.empty()) {
        cout << "\n[Notice] No passenger records found in the system.\n";
        return;
    }

    cout << "\n--- Passenger Details Submenu ---\n";
    cout << "1. Search passenger by PNR\n";
    cout << "2. View all passengers for a specific train\n";
    cout << "3. View all passenger records\n";
    int choice = readInt("Select an option (1 - 3): ", 1, 3);

    if (choice == 1) {
        int pnr = readInt("Enter PNR: ", 1000, 999999);
        int idx = findPassengerByPNR(sys, pnr);
        if (idx == -1) {
            cout << "[Notice] No passenger found with PNR " << pnr << ".\n";
            return;
        }
        const Passenger& p = sys.passengers[idx];
        cout << "\n-------------------------------------------------\n";
        cout << "PNR Number     : " << p.pnr << "\n";
        cout << "Name           : " << p.name << "\n";
        cout << "Age / Gender   : " << p.age << " / " << p.gender << "\n";
        cout << "Train Number   : " << p.trainNo << "\n";
        cout << "Seat Number    : " << p.seatNo << "\n";
        cout << "Travel Date    : " << p.travelDate.day << "/" << p.travelDate.month << "/" << p.travelDate.year << "\n";
        cout << "Status         : " << p.status << "\n";
        cout << "-------------------------------------------------\n";
    } else if (choice == 2) {
        int trainNo = readInt("Enter Train Number: ", 1000, 99999);
        bool found = false;
        cout << "\n=================================================================================\n";
        cout << setw(8)  << "PNR"
             << setw(20) << "Name"
             << setw(6)  << "Age"
             << setw(8)  << "Gender"
             << setw(10) << "Seat No"
             << setw(14) << "Date"
             << setw(12) << "Status" << "\n";
        cout << "=================================================================================\n";

        for (size_t i = 0; i < sys.passengers.size(); i++) {
            const Passenger& p = sys.passengers[i];
            if (p.trainNo == trainNo) {
                found = true;
                string dStr = to_string(p.travelDate.day) + "/" + to_string(p.travelDate.month) + "/" + to_string(p.travelDate.year);
                cout << setw(8)  << p.pnr
                     << setw(20) << p.name
                     << setw(6)  << p.age
                     << setw(8)  << p.gender
                     << setw(10) << p.seatNo
                     << setw(14) << dStr
                     << setw(12) << p.status << "\n";
            }
        }
        if (!found) {
            cout << "No passengers found for train #" << trainNo << ".\n";
        }
        cout << "=================================================================================\n";
    } else {
        cout << "\n============================================================================================\n";
        cout << setw(8)  << "PNR"
             << setw(20) << "Name"
             << setw(6)  << "Age"
             << setw(8)  << "Gender"
             << setw(10) << "Train No"
             << setw(10) << "Seat No"
             << setw(14) << "Date"
             << setw(12) << "Status" << "\n";
        cout << "============================================================================================\n";

        for (size_t i = 0; i < sys.passengers.size(); i++) {
            const Passenger& p = sys.passengers[i];
            string dStr = to_string(p.travelDate.day) + "/" + to_string(p.travelDate.month) + "/" + to_string(p.travelDate.year);
            cout << setw(8)  << p.pnr
                 << setw(20) << p.name
                 << setw(6)  << p.age
                 << setw(8)  << p.gender
                 << setw(10) << p.trainNo
                 << setw(10) << p.seatNo
                 << setw(14) << dStr
                 << setw(12) << p.status << "\n";
        }
        cout << "============================================================================================\n";
    }
}

// Displays waiting list queues for each train non-destructively
// Time Complexity: O(T * W) where T is trains and W is waiting queue length
void displayWaitingList(const RailwaySystem& sys) {
    if (sys.waitingLists.empty()) {
        cout << "\n[Notice] No active waiting lists.\n";
        return;
    }

    bool hasAnyWaiting = false;
    map<int, queue<WaitingEntry> >::const_iterator it;

    for (it = sys.waitingLists.begin(); it != sys.waitingLists.end(); ++it) {
        int trainNo = it->first;
        // Non-destructive queue copy so the real queue remains intact
        queue<WaitingEntry> copyQueue = it->second;

        if (!copyQueue.empty()) {
            hasAnyWaiting = true;
            cout << "\n========================================================================\n";
            cout << " WAITING LIST FOR TRAIN #" << trainNo << " (Queue Size: " << copyQueue.size() << ")\n";
            cout << "========================================================================\n";
            cout << setw(6)  << "Pos"
                 << setw(20) << "Name"
                 << setw(6)  << "Age"
                 << setw(8)  << "Gender"
                 << setw(16) << "Date" << "\n";
            cout << "------------------------------------------------------------------------\n";

            int pos = 1;
            while (!copyQueue.empty()) {
                WaitingEntry w = copyQueue.front();
                copyQueue.pop(); // Pop from local copy only

                string dStr = to_string(w.travelDate.day) + "/" + to_string(w.travelDate.month) + "/" + to_string(w.travelDate.year);
                cout << setw(6)  << ("WL-" + to_string(pos))
                     << setw(20) << w.name
                     << setw(6)  << w.age
                     << setw(8)  << w.gender
                     << setw(16) << dStr << "\n";
                pos++;
            }
            cout << "========================================================================\n";
        }
    }

    if (!hasAnyWaiting) {
        cout << "\n[Notice] All waiting lists are currently empty.\n";
    }
}
