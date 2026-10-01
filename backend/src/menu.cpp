// =============================================================================
// menu.cpp - Console menu I/O and dispatch
// MODULE II: Control Statements (switch, do-while, functions)
// This file handles all cin/cout; DSA functions return results, menu prints them
// =============================================================================
#include "menu.h"
#include "validation.h"
#include "train_ops.h"
#include "booking_ops.h"
#include "seat_map.h"
#include "waiting_queue.h"
#include "train_repo.h"
#include "passenger_repo.h"
#include "waiting_repo.h"
#include <iostream>
#include <iomanip>
#include <cctype>
#include <limits>

using namespace std;

// =============================================================================
// Console I/O Helpers - these read from cin with validation
// =============================================================================

// Read an integer within [minVal, maxVal]. Loops until valid. O(1) per attempt
int readInt(const string& prompt, int minVal, int maxVal) {
    int val;
    while (true) {
        cout << prompt;
        if (cin >> val) {
            string dummy;
            getline(cin, dummy);  // consume leftover newline
            if (val >= minVal && val <= maxVal) return val;
            cout << "  Input out of range (" << minVal << " - " << maxVal << ").\n";
        } else {
            if (cin.eof()) {
                cout << "\nEnd of input. Exiting.\n";
                exit(0);
            }
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "  Invalid number. Try again.\n";
        }
    }
}

// Read a float within [minVal, maxVal]. O(1) per attempt
float readFloat(const string& prompt, float minVal, float maxVal) {
    float val;
    while (true) {
        cout << prompt;
        if (cin >> val) {
            string dummy;
            getline(cin, dummy);
            if (val >= minVal && val <= maxVal) return val;
            cout << "  Input out of range.\n";
        } else {
            if (cin.eof()) { cout << "\nEnd of input.\n"; exit(0); }
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "  Invalid number. Try again.\n";
        }
    }
}

// Read a non-empty string (letters and spaces only). O(n)
string readNonEmptyString(const string& prompt) {
    string s;
    while (true) {
        cout << prompt;
        getline(cin, s);
        if (s.empty() && cin.eof()) { cout << "\nEnd of input.\n"; exit(0); }
        if (isValidName(s)) return s;
        cout << "  Enter a valid name (letters and spaces only).\n";
    }
}

// Read gender: M, F, or O. O(1)
char readGender(const string& prompt) {
    char g;
    while (true) {
        cout << prompt;
        string line;
        getline(cin, line);
        if (!line.empty()) {
            g = toupper(line[0]);
            if (isValidGender(g)) return g;
        }
        cout << "  Enter M (Male), F (Female), or O (Other).\n";
    }
}

// Read a valid date (dd mm yyyy). O(1)
Date readDate(const string& prompt) {
    Date d;
    while (true) {
        cout << prompt;
        if (cin >> d.day >> d.month >> d.year) {
            string dummy;
            getline(cin, dummy);
            if (isValidDate(d.day, d.month, d.year)) return d;
            cout << "  Invalid date. Try again.\n";
        } else {
            if (cin.eof()) { cout << "\nEnd of input.\n"; exit(0); }
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "  Enter date as: DD MM YYYY\n";
        }
    }
}

// =============================================================================
// Menu Display
// =============================================================================

// Print the main menu. O(1)
void printMenu() {
    cout << "\n=======================================================" << endl;
    cout << "         RAILWAY TICKET RESERVATION SYSTEM              " << endl;
    cout << "             (MongoDB Document Edition)                 " << endl;
    cout << "=======================================================" << endl;
    cout << "  1. Add New Train" << endl;
    cout << "  2. Display All Trains" << endl;
    cout << "  3. Search Train" << endl;
    cout << "  4. Book a Ticket" << endl;
    cout << "  5. Cancel a Ticket" << endl;
    cout << "  6. Display Available Seats (2D Seat Map)" << endl;
    cout << "  7. Display Passenger Details" << endl;
    cout << "  8. Display Waiting List" << endl;
    cout << "  9. Sort Trains" << endl;
    cout << "  0. Exit" << endl;
    cout << "=======================================================" << endl;
}

// =============================================================================
// Menu Dispatch - calls DSA functions, handles I/O
// =============================================================================

// Handle menu choice. Each case calls pure DSA functions and prints results. O(varies)
void handleChoice(RailwaySystem& sys, int choice) {
    switch (choice) {

    // --- Option 1: Add a new train ---
    case 1: {
        cout << "\n--- Add New Train ---\n";
        Train t;
        t.trainNo = readInt("  Train Number: ", 10000, 99999);

        // Check duplicate using binary search O(log n)
        if (binarySearchTrain(sys.trains, t.trainNo) != -1) {
            cout << "  Error: Train " << t.trainNo << " already exists.\n";
            break;
        }

        // Consume any leftover newline before reading strings
        t.name = readNonEmptyString("  Train Name: ");
        t.source = readNonEmptyString("  Source Station: ");
        t.destination = readNonEmptyString("  Destination Station: ");
        t.departure = readNonEmptyString("  Departure Time (e.g. 06:00 AM): ");
        t.totalSeats = readInt("  Total Seats (1-60): ", 1, MAX_SEATS);
        t.availableSeats = t.totalSeats;
        t.fare = readFloat("  Fare (Rs): ", 1.0f, 99999.0f);

        int idx = insertTrainSorted(sys.trains, t);
        if (idx >= 0) {
            dbInsertTrain(t);
            cout << "  Train " << t.trainNo << " added successfully at position "
                 << idx + 1 << ".\n";
        } else {
            cout << "  Error: Could not add train.\n";
        }
        break;
    }

    // --- Option 2: Display all trains ---
    case 2: {
        cout << "\n--- All Trains ---\n";
        if (sys.trains.empty()) {
            cout << "  No trains in the system.\n";
        } else {
            cout << formatTrainTable(sys.trains);
        }
        break;
    }

    // --- Option 3: Search train ---
    case 3: {
        cout << "\n--- Search Train ---\n";
        cout << "  1. Search by Train Number (Binary Search)\n";
        cout << "  2. Search by Destination (Linear Search)\n";
        int searchChoice = readInt("  Choice: ", 1, 2);

        if (searchChoice == 1) {
            int num = readInt("  Enter Train Number: ", 10000, 99999);
            int idx = binarySearchTrain(sys.trains, num);
            if (idx >= 0) {
                cout << "\n  Found:\n" << formatTrainInfo(sys.trains[idx]) << endl;
            } else {
                cout << "  Train " << num << " not found.\n";
            }
        } else {
            string dest = readNonEmptyString("  Enter Destination: ");
            vector<int> results = linearSearchByDestination(sys.trains, dest);
            if (results.empty()) {
                cout << "  No trains found to \"" << dest << "\".\n";
            } else {
                cout << "\n  Found " << results.size() << " train(s):\n";
                for (int i = 0; i < (int)results.size(); i++) {
                    cout << formatTrainInfo(sys.trains[results[i]]) << endl;
                }
            }
        }
        break;
    }

    // --- Option 4: Book a ticket ---
    case 4: {
        cout << "\n--- Book a Ticket ---\n";
        if (sys.trains.empty()) {
            cout << "  No trains available. Add a train first.\n";
            break;
        }

        // Show available trains briefly
        cout << "  Available trains:\n";
        for (int i = 0; i < (int)sys.trains.size(); i++) {
            cout << "    " << sys.trains[i].trainNo << " - " << sys.trains[i].name
                 << " (" << sys.trains[i].availableSeats << " seats available)\n";
        }

        int trainNo = readInt("  Enter Train Number: ", 10000, 99999);
        string name = readNonEmptyString("  Passenger Name: ");
        int age = readInt("  Age: ", 1, 120);
        char gender = readGender("  Gender (M/F/O): ");
        Date date = readDate("  Travel Date (DD MM YYYY): ");

        BookingResult result = bookTicket(sys, trainNo, name, age, gender, date);

        if (!result.success) {
            cout << "  Error: " << result.errorMsg << endl;
        } else if (result.status == "CONFIRMED") {
            // Save to database
            dbInsertPassenger(result.passenger);
            dbUpdateTrainSeats(trainNo, sys.trains[binarySearchTrain(sys.trains, trainNo)].availableSeats);

            cout << "\n  BOOKING CONFIRMED!\n";
            cout << formatTicket(result.passenger,
                     sys.trains[binarySearchTrain(sys.trains, trainNo)]) << endl;
        } else if (result.status == "WAITING") {
            // Save waiting entry to database
            dbInsertWaiting(result.waitEntry);

            cout << "\n  ADDED TO WAITING LIST\n";
            cout << "  Wait ID: " << result.waitEntry.waitId << endl;
            cout << "  Position in queue: " << result.waitPosition << endl;
            cout << "  You will be auto-confirmed when a seat frees up.\n";
        }
        break;
    }

    // --- Option 5: Cancel a ticket ---
    case 5: {
        cout << "\n--- Cancel a Ticket ---\n";
        int pnr = readInt("  Enter PNR Number: ", 1001, 99999);

        CancelResult result = cancelTicket(sys, pnr);

        if (!result.success) {
            cout << "  Error: " << result.errorMsg << endl;
        } else {
            // Update database
            dbUpdatePassengerStatus(pnr, "CANCELLED");
            int trainNo = result.cancelled.trainNo;
            int trainIdx = binarySearchTrain(sys.trains, trainNo);
            if (trainIdx >= 0) {
                dbUpdateTrainSeats(trainNo, sys.trains[trainIdx].availableSeats);
            }

            cout << "  Ticket PNR " << pnr << " cancelled successfully.\n";
            cout << "  Passenger: " << result.cancelled.name << endl;

            if (result.promoted) {
                // Save promoted passenger and remove from waiting
                dbInsertPassenger(result.promotedPassenger);
                dbDeleteWaiting(result.promotedWaitId);
                cout << "\n  WAITING LIST PROMOTION!\n";
                cout << "  " << result.promotedPassenger.name
                     << " has been confirmed with PNR "
                     << result.promotedPassenger.pnr
                     << " (Seat " << result.promotedPassenger.seatNo << ")\n";
            }
        }
        break;
    }

    // --- Option 6: Display seat map ---
    case 6: {
        cout << "\n--- Seat Map (2D Array) ---\n";
        if (sys.trains.empty()) {
            cout << "  No trains in the system.\n";
            break;
        }

        int trainNo = readInt("  Enter Train Number: ", 10000, 99999);
        int idx = binarySearchTrain(sys.trains, trainNo);
        if (idx < 0) {
            cout << "  Train " << trainNo << " not found.\n";
        } else {
            cout << getSeatMapString(sys.seatMap, idx,
                     sys.trains[idx].totalSeats,
                     sys.trains[idx].trainNo,
                     sys.trains[idx].name) << endl;
        }
        break;
    }

    // --- Option 7: Display passenger details ---
    case 7: {
        cout << "\n--- Passenger Details ---\n";
        cout << "  1. Search by PNR\n";
        cout << "  2. Search by Train Number\n";
        cout << "  3. Show All Passengers\n";
        int sub = readInt("  Choice: ", 1, 3);

        vector<Passenger> results;
        if (sub == 1) {
            int pnr = readInt("  Enter PNR: ", 1001, 99999);
            results = getPassengerByPNR(sys.passengers, pnr);
        } else if (sub == 2) {
            int tn = readInt("  Enter Train Number: ", 10000, 99999);
            results = getPassengersByTrain(sys.passengers, tn);
        } else {
            results = sys.passengers;
        }

        if (results.empty()) {
            cout << "  No passengers found.\n";
        } else {
            for (int i = 0; i < (int)results.size(); i++) {
                cout << formatPassengerInfo(results[i]) << endl;
            }
        }
        break;
    }

    // --- Option 8: Display waiting list ---
    case 8: {
        cout << "\n--- Waiting List ---\n";
        vector<pair<int, vector<WaitingEntry> > > all = getAllWaitingLists(sys.waitingLists);

        if (all.empty()) {
            cout << "  No passengers in any waiting list.\n";
            break;
        }

        for (int i = 0; i < (int)all.size(); i++) {
            int trainNo = all[i].first;
            vector<WaitingEntry>& entries = all[i].second;

            cout << "\n  Train " << trainNo << " (";
            int trainIdx = binarySearchTrain(sys.trains, trainNo);
            if (trainIdx >= 0) {
                cout << sys.trains[trainIdx].name;
            }
            cout << ") - " << entries.size() << " waiting:\n";

            for (int j = 0; j < (int)entries.size(); j++) {
                cout << "    " << (j + 1) << ". [WaitID: " << entries[j].waitId
                     << "] " << entries[j].name
                     << " (Age: " << entries[j].age
                     << ", Gender: " << entries[j].gender << ")\n";
            }
        }
        break;
    }

    // --- Option 9: Sort trains ---
    case 9: {
        cout << "\n--- Sort Trains ---\n";
        cout << "  1. Sort by Fare (ascending)\n";
        cout << "  2. Sort by Name (alphabetical)\n";
        int sortChoice = readInt("  Choice: ", 1, 2);

        vector<Train> sorted;
        if (sortChoice == 1) {
            sorted = sortTrainsByFare(sys.trains);
            cout << "\n  Trains sorted by fare:\n";
        } else {
            sorted = sortTrainsByName(sys.trains);
            cout << "\n  Trains sorted by name:\n";
        }
        cout << formatTrainTable(sorted);
        cout << "\n  (Note: Original order preserved for binary search.)\n";
        break;
    }

    // --- Option 0: Exit ---
    case 0:
        cout << "\n  Thank you for using the Railway Reservation System. Goodbye!\n";
        break;

    default:
        cout << "  Invalid choice.\n";
        break;
    }
}
