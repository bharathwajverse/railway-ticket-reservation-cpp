#include <iostream>
#include "dsa_manager.h"
#include "database.h"

using namespace std;

// ============================================================================
// MAIN APPLICATION DRIVER
// Organizes user interaction, menu dispatching, and coordinates DB + DSA Manager
// ============================================================================

int main() {
    sqlite3* db = NULL;
    if (!dbOpen(db, "railway.db")) {
        cout << "Error: Unable to connect to database 'railway.db'.\n";
        return 1;
    }

    if (!dbCreateTables(db)) {
        cout << "Error: Unable to initialize SQLite tables.\n";
        dbClose(db);
        return 1;
    }

    DSAManager manager;
    manager.loadFromDatabase(db);

    // Seed default trains if empty
    if (manager.getTrainCount() == 0) {
        Train t1 = {10101, "Rajdhani Express", "Delhi", "Mumbai", "06:00 AM", 4, 4, 1500.0f};
        Train t2 = {10202, "Vande Bharat", "Chennai", "Bangalore", "05:50 AM", 5, 5, 950.0f};
        Train t3 = {10303, "Shatabdi Express", "Kolkata", "Patna", "02:15 PM", 3, 3, 750.0f};
        Train t4 = {10404, "Tejas Express", "Ahmedabad", "Mumbai", "06:40 AM", 4, 4, 1100.0f};
        dbInsertTrain(db, t1);
        dbInsertTrain(db, t2);
        dbInsertTrain(db, t3);
        dbInsertTrain(db, t4);
        manager.loadFromDatabase(db);
    }

    int choice = -1;
    do {
        cout << "\n=======================================================\n"
             << "           RAILWAY TICKET RESERVATION SYSTEM           \n"
             << "=======================================================\n"
             << "  1. View Train Schedules & Fares\n"
             << "  2. Search Train by Train Number (Binary Search)\n"
             << "  3. Search Train by Destination (Linear Search)\n"
             << "  4. Check Coach Seat Layout (2D Array Matrix)\n"
             << "  5. Book a Ticket (Seat Allocation & Concession)\n"
             << "  6. Cancel Confirmed Ticket (Auto-Promotes Queue)\n"
             << "  7. Cancel Waiting List Entry\n"
             << "  8. Undo Last Cancellation (Stack LIFO)\n"
             << "  9. View PNR Status & Print E-Ticket Slip\n"
             << " 10. Sort Trains for Display (Bubble Sort)\n"
             << " 11. Add New Train to Fleet (Admin)\n"
             << " 12. View Unique Stations & Route Tokens (STL Set & Strings)\n"
             << "  0. Exit Application\n"
             << "=======================================================\n";

        choice = readInt("Select an option (0 - 12): ", 0, 12);
        switch (choice) {
            case 1: manager.displayTrains(); break;
            case 2: manager.searchTrainByNumber(); break;
            case 3: manager.searchTrainByDestination(); break;
            case 4: manager.displayCoachLayout(); break;
            case 5: manager.bookTicket(db); break;
            case 6: manager.cancelTicket(db); break;
            case 7: manager.cancelWaitingEntry(db); break;
            case 8: manager.undoLastCancellation(db); break;
            case 9: manager.checkPNRStatus(); break;
            case 10: manager.sortTrainsMenu(); break;
            case 11: manager.addTrain(db); break;
            case 12: manager.displayUniqueStations(); break;
            case 0: cout << "\nThank you for using the Railway Reservation System. Goodbye!\n"; break;
        }
    } while (choice != 0);

    dbClose(db);
    return 0;
}
