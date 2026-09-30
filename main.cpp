#include <iostream>
#include "structures.h"
#include "database.h"
#include "railway.h"
#include "utils.h"

using namespace std;

// Displays the main interactive console menu banner
void displayMenu() {
    cout << "\n=======================================================\n";
    cout << "     RAILWAY TICKET RESERVATION SYSTEM (GROUP 4)       \n";
    cout << "=======================================================\n";
    cout << "  1. Add New Train Record\n";
    cout << "  2. Display All Trains\n";
    cout << "  3. Search Train (By Train No or Destination)\n";
    cout << "  4. Book a Ticket (Allocates Seat or Queues Waitlist)\n";
    cout << "  5. Cancel a Ticket (Frees Seat & Auto-promotes Queue)\n";
    cout << "  6. Display Available Seats & 2D Seat Map\n";
    cout << "  7. Display Passenger Details (By PNR / By Train / All)\n";
    cout << "  8. Display Waiting List Queues\n";
    cout << "  9. Sort Trains for Display (By Fare or Name)\n";
    cout << " 10. Recent Cancellations (Stack - LIFO) & Undo\n";
    cout << "  0. Exit Application\n";
    cout << "=======================================================\n";
}

int main() {
    cout << "\n>>> Starting Railway Ticket Reservation System <<<\n";

    sqlite3* db = NULL;
    if (!openDatabase(db, "railway.db")) {
        cout << "[Fatal Error] Unable to connect to railway.db. Exiting.\n";
        return 1;
    }

    if (!createTables(db)) {
        cout << "[Fatal Error] Unable to initialize database tables. Exiting.\n";
        closeDatabase(db);
        return 1;
    }

    RailwaySystem sys;
    loadSystem(sys, db);
    cout << "[System Ready] Loaded " << sys.trains.size() << " trains, " 
         << sys.passengers.size() << " passenger records from railway.db.\n";

    int choice = -1;
    do {
        displayMenu();
        choice = readInt("Enter your choice (0 - 10): ", 0, 10);

        switch (choice) {
            case 1:
                addTrain(sys, db);
                break;
            case 2:
                displayTrains(sys);
                break;
            case 3: {
                cout << "\n--- Search Submenu ---\n";
                cout << "1. Search by Train Number (Binary Search)\n";
                cout << "2. Search by Destination (Linear Search)\n";
                int sChoice = readInt("Select search type (1 or 2): ", 1, 2);
                if (sChoice == 1) {
                    searchTrainByNumber(sys);
                } else {
                    searchTrainByDestination(sys);
                }
                break;
            }
            case 4:
                bookTicket(sys, db);
                break;
            case 5:
                cancelTicket(sys, db);
                break;
            case 6:
                displayAvailableSeats(sys);
                break;
            case 7:
                displayPassengerDetails(sys);
                break;
            case 8:
                displayWaitingList(sys);
                break;
            case 9:
                sortTrains(sys);
                break;
            case 10: {
                cout << "\n--- Recent Cancellations (Module VIII: Stack LIFO) ---\n";
                cout << "1. View Most Recently Cancelled Ticket (Stack Top)\n";
                cout << "2. Undo Last Cancellation (Restore Seat)\n";
                int stackChoice = readInt("Select option (1 or 2): ", 1, 2);
                if (stackChoice == 1) {
                    viewLastCancelledTicket(sys);
                } else {
                    undoLastCancellation(sys, db);
                }
                break;
            }
            case 0:
                cout << "\nSaving system state and exiting. Thank you!\n";
                break;
            default:
                cout << "[Error] Invalid option. Try again.\n";
                break;
        }

    } while (choice != 0);

    closeDatabase(db);
    return 0;
}
