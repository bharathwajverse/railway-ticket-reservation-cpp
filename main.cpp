/*
  ========================================================================================================
  PROJECT: Railway Ticket Reservation System
  FILE: main.cpp (Complete All-in-One C++ Implementation)
  
  ARCHITECTURE:
    - Pure C++11 procedural design using 'struct' definitions.
    - Zero external dependencies: C++ standard library + SQLite 3 amalgamation + Winsock2.
    - STL Containers: vector, queue, map, stack, string.
    - Algorithms: Binary Search (O(log N)), Linear Search (O(N)), Bubble Sort (O(N^2)).
    - Dual Storage: In-memory working models (vector, 2D array, queue) synchronized with SQLite 3.
    - Dual Interface: Interactive Terminal Kiosk (Console) + Embedded C++ Winsock HTTP Server (Web UI).
  
  TABLE OF CONTENTS:
    SECTION 1: SYSTEM CONSTANTS & DATA STRUCTURES (Date, Train, Passenger, WaitingEntry, RailwaySystem)
    SECTION 2: INPUT VALIDATION & BUSINESS LOGIC UTILITIES (Date checks, concessions, e-ticket export)
    SECTION 3: SQLITE DATABASE LAYER & ATOMIC TRANSACTIONS (CRUD operations, prepared stmts, ACID blocks)
    SECTION 4: CORE RAILWAY DSA ALGORITHMS (Binary Search, Bubble Sort, Seat Matrix, FIFO Queue, LIFO Stack)
    SECTION 5: EMBEDDED WINSOCK HTTP SERVER & REST API (Port 8080, HTML5/CSS3/JS Web Interface)
    SECTION 6: PORTAL MENUS & MAIN ENTRY POINT (Passenger Portal, Admin Portal, Web Server Launcher)
  ========================================================================================================
*/

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <queue>
#include <stack>
#include <iomanip>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include "sqlite3.h"

// Cross-Platform Socket Headers (Windows Winsock2 vs. POSIX Sockets)
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef int socklen_t;
    #define CLOSE_SOCKET(s) closesocket(s)
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int SOCKET;
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR (-1)
    #define CLOSE_SOCKET(s) close(s)
#endif

using namespace std;

// Global Mutex for thread-safe concurrent access between Terminal Kiosk and Web Server
std::mutex g_sysMutex;

// ========================================================================================================
// SECTION 1: SYSTEM CONSTANTS & DATA STRUCTURES (MODULE VI & IV)
// ========================================================================================================

/*
  INSTRUCTION / EXPLANATION:
  - Constants define the hard capacity bounds of our system.
  - MAX_TRAINS (20): Maximum number of trains supported.
  - MAX_SEATS (60): Maximum seats per train (rows in our 2D seat grid).
  - MAX_WAITING (10): Maximum passengers allowed in the waiting queue per train.
*/
const int MAX_TRAINS = 20;
const int MAX_SEATS = 60;
const int MAX_WAITING = 10;

/*
  STRUCTURE: Date
  PURPOSE: Represents a calendar date (day, month, year).
  DESIGN RATIONALE: Nested inside Passenger and WaitingEntry to encapsulate date attributes cleanly.
*/
struct Date {
    int day;
    int month;
    int year;
};

/*
  STRUCTURE: Train
  PURPOSE: Represents a scheduled train in the railway network.
  ATTRIBUTES: trainNo, name, source, destination, departure time, totalSeats, availableSeats, fare.
  DESIGN RATIONALE: Groups all heterogeneous train specifications into a single contiguous record.
*/
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

/*
  STRUCTURE: Passenger
  PURPOSE: Represents a confirmed or cancelled passenger ticket.
  ATTRIBUTES: Unique PNR, name, age, gender, trainNo, seatNo, travelDate, status, concession, farePaid.
  DESIGN RATIONALE: Encapsulates passenger identity, assigned coach seat, concession tier, and billed fare.
*/
struct Passenger {
    int pnr;
    string name;
    int age;
    char gender;
    int trainNo;
    int seatNo;
    Date travelDate;
    string status;      // "CONFIRMED" or "CANCELLED"
    string concession;  // "GENERAL", "CHILD (50% OFF)", "SENIOR CITIZEN (40% OFF)"
    float farePaid;     // Final fare paid after concession
};

/*
  STRUCTURE: WaitingEntry
  PURPOSE: Represents a passenger placed on the waiting list when a train is full.
  DESIGN RATIONALE: Managed within a FIFO queue; promoted automatically upon ticket cancellation.
*/
struct WaitingEntry {
    int waitId;
    string name;
    int age;
    char gender;
    int trainNo;
    Date travelDate;
};

/*
  STRUCTURE: SystemStats
  PURPOSE: Encapsulates executive summary analytics for the Admin Portal and Web Dashboard.
*/
struct SystemStats {
    int totalTrains;
    int totalBookings;
    int confirmedBookings;
    int cancelledBookings;
    int totalWaitlisted;
    float totalRevenue;
};

/*
  STRUCTURE: RailwaySystem
  PURPOSE: Holds the entire active in-memory state of the railway application.
  DATA STRUCTURES INCLUDED:
    1. vector<Train> trains: Contiguous dynamic array, kept sorted by trainNo for Binary Search.
    2. vector<Passenger> passengers: Dynamic list of all booked and cancelled tickets.
    3. map<int, queue<WaitingEntry> > waitingLists: Associates each trainNo with a FIFO waiting queue.
    4. int seatMap[MAX_TRAINS][MAX_SEATS]: 2D array tracking seat occupancy (0 = free, 1 = booked).
    5. stack<Passenger> recentCancellations: LIFO stack tracking recently cancelled tickets for undo.
  DESIGN RATIONALE:
    Dual storage architecture: O(1) seat matrix checks, FIFO waiting queues, LIFO cancellation undo stack,
    and O(log N) binary search across trains, all mirrored to SQLite for ACID persistence.
*/
struct RailwaySystem {
    vector<Train> trains;
    vector<Passenger> passengers;
    map<int, queue<WaitingEntry> > waitingLists;
    int seatMap[MAX_TRAINS][MAX_SEATS];
    stack<Passenger> recentCancellations;
};


// ========================================================================================================
// SECTION 2: INPUT VALIDATION & BUSINESS LOGIC UTILITIES (MODULES I, II & V)
// ========================================================================================================

/*
  FUNCTION: isLeapYear
  PURPOSE: Determines whether a given year is a leap year.
  TIME COMPLEXITY: O(1)
  ALGORITHM NOTE: A year is leap if divisible by 4 and not 100, or if divisible by 400.
*/
bool isLeapYear(int year) {
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        return true;
    }
    return false;
}

/*
  FUNCTION: isValidDate
  PURPOSE: Validates if a given day, month, and year form a legitimate calendar date.
  TIME COMPLEXITY: O(1)
  ALGORITHM NOTE: Verifies month range [1-12], days per month via lookup table, and 29 days for Feb in leap years.
*/
bool isValidDate(int day, int month, int year) {
    if (year < 2024 || year > 2035) {
        return false;
    }
    if (month < 1 || month > 12) {
        return false;
    }
    // Days in each month (January to December)
    int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month == 2 && isLeapYear(year)) {
        daysInMonth[1] = 29; // February has 29 days in a leap year
    }
    if (day < 1 || day > daysInMonth[month - 1]) {
        return false;
    }
    return true;
}

/*
  FUNCTION: toLowerCase
  PURPOSE: Converts a string to all lowercase characters manually.
  TIME COMPLEXITY: O(N) where N is length of string.
*/
string toLowerCase(const string& str) {
    string result = "";
    for (size_t i = 0; i < str.length(); i++) {
        result += (char)tolower(str[i]);
    }
    return result;
}

/*
  FUNCTION: containsIgnoreCase
  PURPOSE: Performs case-insensitive substring pattern matching without using regex.
  TIME COMPLEXITY: O(N * M) where N = text length, M = pattern length.
*/
bool containsIgnoreCase(const string& text, const string& pattern) {
    string lowerText = toLowerCase(text);
    string lowerPattern = toLowerCase(pattern);
    return lowerText.find(lowerPattern) != string::npos;
}

/*
  FUNCTION: readInt
  PURPOSE: Robust integer reader that clears bad stream state and verifies range bounds.
*/
int readInt(const string& prompt, int minVal, int maxVal) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            if (value >= minVal && value <= maxVal) {
                // Clear remaining newline character from input buffer
                string dummy;
                getline(cin, dummy);
                return value;
            }
            cout << "[Error] Input out of range! Please enter a value between " 
                 << minVal << " and " << maxVal << ".\n";
        } else {
            cout << "[Error] Invalid input! Please enter a numeric integer.\n";
            cin.clear(); // Reset cin error state
            string badInput;
            cin >> badInput; // Discard invalid token
        }
    }
}

/*
  FUNCTION: readNonEmptyString
  PURPOSE: Prompts user until a non-empty, non-whitespace string is entered.
*/
string readNonEmptyString(const string& prompt) {
    string value;
    while (true) {
        cout << prompt;
        getline(cin, value);
        // Trim leading and trailing whitespace
        size_t start = value.find_first_not_of(" \t\r\n");
        size_t end = value.find_last_not_of(" \t\r\n");
        if (start != string::npos && end != string::npos) {
            return value.substr(start, end - start + 1);
        }
        cout << "[Error] Input cannot be blank! Please enter a valid text value.\n";
    }
}

/*
  FUNCTION: readDate
  PURPOSE: Prompts user to input day, month, and year and validates them.
*/
Date readDate(const string& prompt) {
    cout << prompt << "\n";
    Date d;
    while (true) {
        d.day = readInt("  Enter Day (1-31): ", 1, 31);
        d.month = readInt("  Enter Month (1-12): ", 1, 12);
        d.year = readInt("  Enter Year (2024-2035): ", 2024, 2035);

        if (isValidDate(d.day, d.month, d.year)) {
            return d;
        }
        cout << "[Error] Invalid date (" << d.day << "/" << d.month << "/" << d.year 
             << ")! Check month length & leap years. Try again.\n";
    }
}

/*
  FUNCTION: calculateConcession
  PURPOSE: Computes fare discount based on age categories.
  RULES:
    - Child (< 12 years): 50% discount
    - Senior Citizen (>= 60 years): 40% discount
    - General (12 - 59 years): 0% discount (full fare)
  DESIGN NOTE: Takes passenger age and train base fare, computing the concession tier and payable amount.
*/
void calculateConcession(int age, float baseFare, string& concession, float& finalFare) {
    if (age < 12) {
        concession = "CHILD (50% OFF)";
        finalFare = baseFare * 0.50f;
    } else if (age >= 60) {
        concession = "SENIOR CITIZEN (40% OFF)";
        finalFare = baseFare * 0.60f;
    } else {
        concession = "GENERAL";
        finalFare = baseFare;
    }
}

/*
  FUNCTION: exportTicketToFile
  PURPOSE: Generates a formatted official electronic ticket slip (ticket_<PNR>.txt) using <fstream>.
  DESIGN NOTE: Serializes the booking confirmation into an electronic reservation slip file on disk.
*/
bool exportTicketToFile(const Passenger& p, const Train& t) {
    string fileName = "ticket_" + to_string(p.pnr) + ".txt";
    ofstream fout(fileName.c_str());
    if (!fout.is_open()) {
        return false;
    }

    float discount = t.fare - p.farePaid;
    if (discount < 0.0f) discount = 0.0f;

    // Berth type deduction
    string berth = "Middle";
    if (p.seatNo % 4 == 1) berth = "Window (Lower)";
    else if (p.seatNo % 4 == 2) berth = "Aisle";
    else if (p.seatNo % 4 == 3) berth = "Middle";
    else berth = "Window (Upper)";

    fout << "======================================================================\n";
    fout << "                    INDIAN RAILWAY PASSENGER RESERVATION               \n";
    fout << "                           ELECTRONIC RESERVATION SLIP                 \n";
    fout << "======================================================================\n";
    fout << "PNR NUMBER        : " << p.pnr << "\n";
    fout << "BOOKING STATUS    : " << p.status << "\n";
    fout << "PASSENGER NAME    : " << p.name << "\n";
    fout << "AGE / GENDER      : " << p.age << " yrs / " << p.gender << "\n";
    fout << "CONCESSION TIER   : " << p.concession << "\n";
    fout << "----------------------------------------------------------------------\n";
    fout << "TRAIN NUMBER      : " << t.trainNo << "\n";
    fout << "TRAIN NAME        : " << t.name << "\n";
    fout << "JOURNEY ROUTE     : " << t.source << " --> " << t.destination << "\n";
    fout << "SCHEDULED DEPART  : " << t.departure << "\n";
    fout << "TRAVEL DATE       : " << setfill('0') << setw(2) << p.travelDate.day << "/"
                                  << setfill('0') << setw(2) << p.travelDate.month << "/"
                                  << p.travelDate.year << setfill(' ') << "\n";
    fout << "ALLOCATED SEAT    : Coach C1, Seat #" << p.seatNo << " (" << berth << ")\n";
    fout << "----------------------------------------------------------------------\n";
    fout << "BASE TRAIN FARE   : Rs. " << fixed << setprecision(2) << t.fare << "\n";
    fout << "CONCESSION SAVINGS: Rs. " << fixed << setprecision(2) << discount << "\n";
    fout << "TOTAL FARE CHARGED: Rs. " << fixed << setprecision(2) << p.farePaid << "\n";
    fout << "PAYMENT MODE      : ELECTRONIC CONFIRMED (ACID COMPLIANT)\n";
    fout << "======================================================================\n";
    fout << "  Please carry a valid Original Photo ID proof during the journey.   \n";
    fout << "          Wish you a happy and comfortable journey!                  \n";
    fout << "======================================================================\n";

    fout.close();
    return true;
}


// ========================================================================================================
// SECTION 3: SQLITE DATABASE LAYER & ATOMIC TRANSACTIONS (BACKEND PERSISTENCE)
// ========================================================================================================

/*
  EXPLANATION OF SQLITE 3 FUNCTIONS FOR BEGINNERS:
    - sqlite3_open(fileName, &db): Connects to the database file; creates it if missing.
    - sqlite3_prepare_v2(db, sql, ...): Compiles an SQL string into bytecode. '?' marks parameters.
    - sqlite3_bind_int/text/double(stmt, index, value): Safely fills in '?' to prevent SQL injection.
    - sqlite3_step(stmt): Executes statement (returns SQLITE_DONE for writes, SQLITE_ROW for reads).
    - sqlite3_column_int/text/double(stmt, col): Retrieves data from the current row.
    - sqlite3_finalize(stmt): Frees bytecode memory to prevent memory leaks.
    - sqlite3_close(db): Closes database connection.
*/

// Opens database connection
bool openDatabase(sqlite3*& db, const char* fileName) {
    int rc = sqlite3_open(fileName, &db);
    if (rc != SQLITE_OK) {
        cout << "[Database Error] Cannot open database: " << sqlite3_errmsg(db) << "\n";
        return false;
    }
    // Performance and data integrity optimization
    sqlite3_exec(db, "PRAGMA foreign_keys = ON; PRAGMA synchronous = NORMAL;", NULL, NULL, NULL);
    return true;
}

// Closes database connection
void closeDatabase(sqlite3* db) {
    if (db != NULL) {
        sqlite3_close(db);
    }
}

// Atomic SQLite Transaction Helpers
bool beginTransaction(sqlite3* db) {
    char* err = NULL;
    int rc = sqlite3_exec(db, "BEGIN IMMEDIATE TRANSACTION;", NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
        return false;
    }
    return true;
}

bool commitTransaction(sqlite3* db) {
    char* err = NULL;
    int rc = sqlite3_exec(db, "COMMIT;", NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
        return false;
    }
    return true;
}

bool rollbackTransaction(sqlite3* db) {
    char* err = NULL;
    int rc = sqlite3_exec(db, "ROLLBACK;", NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
        return false;
    }
    return true;
}

// Creates the 3 essential relational tables: trains, passengers, waiting_list
bool createTables(sqlite3* db) {
    const char* sql = 
        "CREATE TABLE IF NOT EXISTS trains ("
        "  train_no INTEGER PRIMARY KEY,"
        "  name TEXT NOT NULL,"
        "  source TEXT NOT NULL,"
        "  destination TEXT NOT NULL,"
        "  departure TEXT NOT NULL,"
        "  total_seats INTEGER NOT NULL,"
        "  available_seats INTEGER NOT NULL,"
        "  fare REAL NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS passengers ("
        "  pnr INTEGER PRIMARY KEY,"
        "  name TEXT NOT NULL,"
        "  age INTEGER NOT NULL,"
        "  gender TEXT NOT NULL,"
        "  train_no INTEGER NOT NULL,"
        "  seat_no INTEGER NOT NULL,"
        "  day INTEGER NOT NULL,"
        "  month INTEGER NOT NULL,"
        "  year INTEGER NOT NULL,"
        "  status TEXT NOT NULL,"
        "  concession TEXT DEFAULT 'GENERAL',"
        "  fare_paid REAL DEFAULT 0.0"
        ");"
        "CREATE TABLE IF NOT EXISTS waiting_list ("
        "  wait_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name TEXT NOT NULL,"
        "  age INTEGER NOT NULL,"
        "  gender TEXT NOT NULL,"
        "  train_no INTEGER NOT NULL,"
        "  day INTEGER NOT NULL,"
        "  month INTEGER NOT NULL,"
        "  year INTEGER NOT NULL"
        ");";

    char* errMsg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        cout << "[Database Error] Table creation failed: " << errMsg << "\n";
        sqlite3_free(errMsg);
        return false;
    }

    // Dynamic schema migrations for existing databases
    sqlite3_exec(db, "ALTER TABLE passengers ADD COLUMN concession TEXT DEFAULT 'GENERAL';", NULL, NULL, NULL);
    sqlite3_exec(db, "ALTER TABLE passengers ADD COLUMN fare_paid REAL DEFAULT 0.0;", NULL, NULL, NULL);
    return true;
}

// Inserts a new train into database using prepared statements
bool insertTrain(sqlite3* db, const Train& t) {
    const char* sql = "INSERT INTO trains (train_no, name, source, destination, departure, total_seats, available_seats, fare) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, t.trainNo);
    sqlite3_bind_text(stmt, 2, t.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, t.source.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, t.destination.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, t.departure.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, t.totalSeats);
    sqlite3_bind_int(stmt, 7, t.availableSeats);
    sqlite3_bind_double(stmt, 8, (double)t.fare);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

// Updates available seats count of a train in database
bool updateTrainSeats(sqlite3* db, int trainNo, int availableSeats) {
    const char* sql = "UPDATE trains SET available_seats = ? WHERE train_no = ?;";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, availableSeats);
    sqlite3_bind_int(stmt, 2, trainNo);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

// Loads all trains from database sorted by train_no
bool loadTrains(sqlite3* db, vector<Train>& trains) {
    trains.clear();
    const char* sql = "SELECT train_no, name, source, destination, departure, total_seats, available_seats, fare "
                      "FROM trains ORDER BY train_no ASC;";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Train t;
        t.trainNo = sqlite3_column_int(stmt, 0);
        t.name = (const char*)sqlite3_column_text(stmt, 1);
        t.source = (const char*)sqlite3_column_text(stmt, 2);
        t.destination = (const char*)sqlite3_column_text(stmt, 3);
        t.departure = (const char*)sqlite3_column_text(stmt, 4);
        t.totalSeats = sqlite3_column_int(stmt, 5);
        t.availableSeats = sqlite3_column_int(stmt, 6);
        t.fare = (float)sqlite3_column_double(stmt, 7);
        trains.push_back(t);
    }
    sqlite3_finalize(stmt);
    return true;
}

// Inserts a new booked passenger record into database
bool insertPassenger(sqlite3* db, const Passenger& p) {
    const char* sql = "INSERT INTO passengers (pnr, name, age, gender, train_no, seat_no, day, month, year, status, concession, fare_paid) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    char gStr[2] = { p.gender, '\0' };
    sqlite3_bind_int(stmt, 1, p.pnr);
    sqlite3_bind_text(stmt, 2, p.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, p.age);
    sqlite3_bind_text(stmt, 4, gStr, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, p.trainNo);
    sqlite3_bind_int(stmt, 6, p.seatNo);
    sqlite3_bind_int(stmt, 7, p.travelDate.day);
    sqlite3_bind_int(stmt, 8, p.travelDate.month);
    sqlite3_bind_int(stmt, 9, p.travelDate.year);
    sqlite3_bind_text(stmt, 10, p.status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 11, p.concession.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 12, (double)p.farePaid);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

// Updates status of a ticket (e.g. CONFIRMED <-> CANCELLED)
bool updatePassengerStatus(sqlite3* db, int pnr, const string& status) {
    const char* sql = "UPDATE passengers SET status = ? WHERE pnr = ?;";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, pnr);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

// Loads all passenger records from database
bool loadPassengers(sqlite3* db, vector<Passenger>& list) {
    list.clear();
    const char* sql = "SELECT pnr, name, age, gender, train_no, seat_no, day, month, year, status, "
                      "COALESCE(concession, 'GENERAL'), COALESCE(fare_paid, 0.0) "
                      "FROM passengers ORDER BY pnr ASC;";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Passenger p;
        p.pnr = sqlite3_column_int(stmt, 0);
        p.name = (const char*)sqlite3_column_text(stmt, 1);
        p.age = sqlite3_column_int(stmt, 2);
        const char* gStr = (const char*)sqlite3_column_text(stmt, 3);
        p.gender = (gStr != NULL && gStr[0] != '\0') ? gStr[0] : 'O';
        p.trainNo = sqlite3_column_int(stmt, 4);
        p.seatNo = sqlite3_column_int(stmt, 5);
        p.travelDate.day = sqlite3_column_int(stmt, 6);
        p.travelDate.month = sqlite3_column_int(stmt, 7);
        p.travelDate.year = sqlite3_column_int(stmt, 8);
        p.status = (const char*)sqlite3_column_text(stmt, 9);
        const char* conc = (const char*)sqlite3_column_text(stmt, 10);
        p.concession = (conc != NULL) ? conc : "GENERAL";
        p.farePaid = (float)sqlite3_column_double(stmt, 11);
        list.push_back(p);
    }
    sqlite3_finalize(stmt);
    return true;
}

// Inserts a passenger into the waiting list table
bool insertWaiting(sqlite3* db, WaitingEntry& w) {
    const char* sql = "INSERT INTO waiting_list (name, age, gender, train_no, day, month, year) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    char gStr[2] = { w.gender, '\0' };
    sqlite3_bind_text(stmt, 1, w.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, w.age);
    sqlite3_bind_text(stmt, 3, gStr, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, w.trainNo);
    sqlite3_bind_int(stmt, 5, w.travelDate.day);
    sqlite3_bind_int(stmt, 6, w.travelDate.month);
    sqlite3_bind_int(stmt, 7, w.travelDate.year);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE) {
        w.waitId = (int)sqlite3_last_insert_rowid(db);
    }
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

// Deletes a promoted or cancelled passenger from waiting_list table
bool deleteWaiting(sqlite3* db, int waitId) {
    const char* sql = "DELETE FROM waiting_list WHERE wait_id = ?;";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_int(stmt, 1, waitId);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

// Loads waiting lists grouped by train_no in FIFO order
bool loadWaitingList(sqlite3* db, map<int, queue<WaitingEntry> >& waitingLists) {
    waitingLists.clear();
    const char* sql = "SELECT wait_id, name, age, gender, train_no, day, month, year "
                      "FROM waiting_list ORDER BY wait_id ASC;";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return false;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        WaitingEntry w;
        w.waitId = sqlite3_column_int(stmt, 0);
        w.name = (const char*)sqlite3_column_text(stmt, 1);
        w.age = sqlite3_column_int(stmt, 2);
        const char* gStr = (const char*)sqlite3_column_text(stmt, 3);
        w.gender = (gStr != NULL && gStr[0] != '\0') ? gStr[0] : 'O';
        w.trainNo = sqlite3_column_int(stmt, 4);
        w.travelDate.day = sqlite3_column_int(stmt, 5);
        w.travelDate.month = sqlite3_column_int(stmt, 6);
        w.travelDate.year = sqlite3_column_int(stmt, 7);
        waitingLists[w.trainNo].push(w); // FIFO queue push
    }
    sqlite3_finalize(stmt);
    return true;
}


// ========================================================================================================
// SECTION 4: CORE RAILWAY DSA ALGORITHMS (MODULES III, IV, VII & VIII)
// ========================================================================================================

/*
  FUNCTION: binarySearchTrain
  PURPOSE: Locates a train in the sorted trains vector in O(log N) time.
  TIME COMPLEXITY: O(log N) where N is number of trains.
  ALGORITHM NOTE: Divides search range in half iteratively based on trainNo key comparison.
*/
int binarySearchTrain(const vector<Train>& trains, int trainNo) {
    int low = 0;
    int high = (int)trains.size() - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2; // Prevents integer overflow
        if (trains[mid].trainNo == trainNo) {
            return mid; // Target found
        }
        if (trains[mid].trainNo < trainNo) {
            low = mid + 1; // Search right half
        } else {
            high = mid - 1; // Search left half
        }
    }
    return -1; // Target does not exist
}

// Wrapper to search train in system
int findTrainIndex(const RailwaySystem& sys, int trainNo) {
    return binarySearchTrain(sys.trains, trainNo);
}

/*
  FUNCTION: insertTrainSorted
  PURPOSE: Inserts a new train into vector while maintaining sorted order by trainNo.
  TIME COMPLEXITY: O(N) due to vector element shifting.
*/
void insertTrainSorted(vector<Train>& trains, const Train& t) {
    size_t i = 0;
    while (i < trains.size() && trains[i].trainNo < t.trainNo) {
        i++;
    }
    trains.insert(trains.begin() + i, t);
}

/*
  FUNCTION: addTrain
  PURPOSE: Prompts admin for new train details, validates, and stores in SQLite and vector.
*/
void addTrain(RailwaySystem& sys, sqlite3* db) {
    cout << "\n--- Add New Train Record ---\n";
    if ((int)sys.trains.size() >= MAX_TRAINS) {
        cout << "[Error] System capacity reached (" << MAX_TRAINS << " trains max).\n";
        return;
    }

    int trainNo = readInt("Enter Train Number (1000 - 99999): ", 1000, 99999);
    if (findTrainIndex(sys, trainNo) != -1) {
        cout << "[Error] Train #" << trainNo << " already exists in the system!\n";
        return;
    }

    Train t;
    t.trainNo = trainNo;
    t.name = readNonEmptyString("Enter Train Name: ");
    t.source = readNonEmptyString("Enter Source Station: ");
    t.destination = readNonEmptyString("Enter Destination Station: ");
    t.departure = readNonEmptyString("Enter Departure Time (HH:MM): ");
    t.totalSeats = readInt("Enter Total Seats (1 - " + to_string(MAX_SEATS) + "): ", 1, MAX_SEATS);
    t.availableSeats = t.totalSeats;
    t.fare = (float)readInt("Enter Ticket Base Fare in Rs (50 - 10000): ", 50, 10000);

    std::lock_guard<std::mutex> lock(g_sysMutex);

    if (insertTrain(db, t)) {
        insertTrainSorted(sys.trains, t);
        // Initialize seat map for the newly inserted train index
        int idx = findTrainIndex(sys, t.trainNo);
        for (int s = 0; s < MAX_SEATS; s++) {
            sys.seatMap[idx][s] = 0;
        }
        cout << "[Success] Train #" << t.trainNo << " (" << t.name << ") added successfully!\n";
    } else {
        cout << "[Error] Failed to insert train into database.\n";
    }
}

/*
  FUNCTION: displayTrains
  PURPOSE: Formats and prints all trains in tabular layout using setw.
  TIME COMPLEXITY: O(N)
*/
void displayTrains(const RailwaySystem& sys) {
    if (sys.trains.empty()) {
        cout << "\n[Notice] No trains currently available in the system.\n";
        return;
    }

    cout << "\n====================================================================================================\n";
    cout << setw(8)  << "Train#"
         << setw(22) << "Train Name"
         << setw(16) << "Source"
         << setw(16) << "Destination"
         << setw(12) << "Departure"
         << setw(8)  << "Total"
         << setw(8)  << "Avail"
         << setw(10) << "Fare (Rs)" << "\n";
    cout << "====================================================================================================\n";

    for (size_t i = 0; i < sys.trains.size(); i++) {
        const Train& t = sys.trains[i];
        cout << setw(8)  << t.trainNo
             << setw(22) << t.name
             << setw(16) << t.source
             << setw(16) << t.destination
             << setw(12) << t.departure
             << setw(8)  << t.totalSeats
             << setw(8)  << t.availableSeats
             << setw(10) << fixed << setprecision(2) << t.fare << "\n";
    }
    cout << "====================================================================================================\n";
}

/*
  FUNCTION: searchTrainByNumber
  PURPOSE: Fast lookup using Binary Search (O(log N)).
*/
void searchTrainByNumber(const RailwaySystem& sys) {
    int trainNo = readInt("\nEnter Train Number to search: ", 1000, 99999);
    int idx = findTrainIndex(sys, trainNo);

    if (idx != -1) {
        const Train& t = sys.trains[idx];
        cout << "\n[Train Found via Binary Search (O(log N))]:\n";
        cout << "  Train Number   : " << t.trainNo << "\n";
        cout << "  Train Name     : " << t.name << "\n";
        cout << "  Route          : " << t.source << " --> " << t.destination << "\n";
        cout << "  Departure      : " << t.departure << "\n";
        cout << "  Total Seats    : " << t.totalSeats << "\n";
        cout << "  Available Seats: " << t.availableSeats << "\n";
        cout << "  Ticket Fare    : Rs. " << fixed << setprecision(2) << t.fare << "\n";
    } else {
        cout << "[Notice] Train #" << trainNo << " was not found in the system.\n";
    }
}

/*
  FUNCTION: searchTrainByDestination
  PURPOSE: Case-insensitive substring search using Linear Search (O(N)).
*/
void searchTrainByDestination(const RailwaySystem& sys) {
    string dest = readNonEmptyString("\nEnter Destination Station to search: ");
    bool found = false;

    for (size_t i = 0; i < sys.trains.size(); i++) {
        if (containsIgnoreCase(sys.trains[i].destination, dest)) {
            if (!found) {
                cout << "\n[Matching Trains (Linear Search O(N))]:\n";
                cout << setw(8)  << "Train#"
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

/*
  FUNCTION: sortTrains
  PURPOSE: Sorts trains for display using manual Bubble Sort on a COPY of the vector.
  TIME COMPLEXITY: O(N^2)
  DESIGN NOTE: Sorts a detached vector copy to preserve primary binary search order (by trainNo) in system state.
*/
void sortTrains(RailwaySystem& sys) {
    if (sys.trains.empty()) {
        cout << "\n[Notice] No trains to sort.\n";
        return;
    }

    cout << "\n--- Sort Trains for Display (Bubble Sort) ---\n";
    cout << "1. Sort by Ticket Fare (Lowest to Highest)\n";
    cout << "2. Sort by Train Name (Alphabetical A-Z)\n";
    int choice = readInt("Select sorting criteria (1 or 2): ", 1, 2);

    vector<Train> copyList = sys.trains;
    int n = (int)copyList.size();

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
    cout << "\n[Displaying Sorted List of Trains]:\n";
    displayTrains(tempSys);
}

/*
  FUNCTION: findFreeSeat
  PURPOSE: Scans the 2D seatMap row to locate the first free seat (value 0).
  TIME COMPLEXITY: O(S) where S is totalSeats.
*/
int findFreeSeat(const RailwaySystem& sys, int trainIndex) {
    int total = sys.trains[trainIndex].totalSeats;
    for (int s = 0; s < total; s++) {
        if (sys.seatMap[trainIndex][s] == 0) {
            return s + 1; // 1-based seat number
        }
    }
    return -1;
}

/*
  FUNCTION: displayTrainSeats
  PURPOSE: Renders the 2D seating matrix layout for a specific train index.
*/
void displayTrainSeats(const RailwaySystem& sys, int trainIdx) {
    const Train& t = sys.trains[trainIdx];
    cout << "\nCoach Layout for Train #" << t.trainNo << " (" << t.name << ") [XX = Booked]:\n\n";
    for (int s = 0; s < t.totalSeats; s++) {
        if (sys.seatMap[trainIdx][s] == 1) {
            cout << "[ XX ] ";
        } else {
            cout << "[ " << setw(2) << (s + 1) << " ] ";
        }
        if ((s + 1) % 4 == 0) cout << "   ";
        if ((s + 1) % 6 == 0) cout << "\n";
    }
    if (t.totalSeats % 6 != 0) cout << "\n";
    cout << "\n";
}

/*
  FUNCTION: generatePNR
  PURPOSE: Generates the next unique PNR (max PNR + 1 starting from 1001).
*/
int generatePNR(const RailwaySystem& sys) {
    int maxPnr = 1000;
    for (size_t i = 0; i < sys.passengers.size(); i++) {
        if (sys.passengers[i].pnr > maxPnr) {
            maxPnr = sys.passengers[i].pnr;
        }
    }
    return maxPnr + 1;
}

/*
  FUNCTION: bookTicket
  PURPOSE: Books a ticket, allocating a seat via manual pick or auto-assign,
           applying age concessions, persisting via atomic SQLite transaction,
           and generating an E-Ticket receipt file.
*/
void bookTicket(RailwaySystem& sys, sqlite3* db) {
    cout << "\n--- Book a Ticket ---\n";
    if (sys.trains.empty()) {
        cout << "[Notice] No trains available for booking. Add trains first.\n";
        return;
    }

    int trainNo = readInt("Enter Train Number: ", 1000, 99999);
    int trainIdx = findTrainIndex(sys, trainNo);

    if (trainIdx == -1) {
        cout << "[Error] Train #" << trainNo << " does not exist.\n";
        return;
    }

    string name = readNonEmptyString("Enter Passenger Full Name: ");
    int age = readInt("Enter Passenger Age: ", 1, 120);

    char gender = 'O';
    while (true) {
        cout << "Enter Gender (M = Male, F = Female, O = Other): ";
        string gStr;
        getline(cin, gStr);
        if (!gStr.empty()) {
            char c = (char)toupper(gStr[0]);
            if (c == 'M' || c == 'F' || c == 'O') {
                gender = c;
                break;
            }
        }
        cout << "[Error] Invalid gender! Enter M, F, or O.\n";
    }

    Date travelDate = readDate("Enter Date of Travel:");

    // Duplicate check: Same passenger already confirmed on this train & date
    for (size_t i = 0; i < sys.passengers.size(); i++) {
        const Passenger& p = sys.passengers[i];
        if (p.status == "CONFIRMED" && p.trainNo == trainNo &&
            toLowerCase(p.name) == toLowerCase(name) &&
            p.travelDate.day == travelDate.day &&
            p.travelDate.month == travelDate.month &&
            p.travelDate.year == travelDate.year) {
            cout << "\n[Error] Duplicate booking! " << name << " already has a confirmed ticket on train #"
                 << trainNo << " for this travel date.\n";
            return;
        }
    }

    // Check availability
    int freeSeat = findFreeSeat(sys, trainIdx);

    // CASE 1: Seats available -> Allocate seat
    if (freeSeat != -1 && sys.trains[trainIdx].availableSeats > 0) {
        int allocatedSeat = -1;

        cout << "\nSeat Allocation Preference:\n";
        cout << "  1. Auto-Assign First Available Seat\n";
        cout << "  2. Choose Seat Number Manually from Visual Seat Map\n";
        int allocMode = readInt("Select mode (1 or 2): ", 1, 2);

        if (allocMode == 2) {
            displayTrainSeats(sys, trainIdx);
            while (true) {
                int chosen = readInt("Enter desired seat number (1 - " + to_string(sys.trains[trainIdx].totalSeats) + "): ",
                                     1, sys.trains[trainIdx].totalSeats);
                if (sys.seatMap[trainIdx][chosen - 1] == 0) {
                    allocatedSeat = chosen;
                    cout << "[Confirmed] Seat #" << allocatedSeat << " is free and allocated to you!\n";
                    break;
                } else {
                    cout << "[Occupied] Seat #" << chosen << " is already booked!\n";
                    cout << "1. Choose a different seat\n";
                    cout << "2. Fallback to auto-assign\n";
                    int fallback = readInt("Select (1 or 2): ", 1, 2);
                    if (fallback == 2) {
                        allocatedSeat = findFreeSeat(sys, trainIdx);
                        cout << "[Auto-Assigned] Allocated Seat #" << allocatedSeat << ".\n";
                        break;
                    }
                }
            }
        } else {
            allocatedSeat = freeSeat;
        }

        // Calculate Age-Based Concession
        string concessionTier;
        float finalFare = 0.0f;
        calculateConcession(age, sys.trains[trainIdx].fare, concessionTier, finalFare);

        Passenger p;
        p.pnr = generatePNR(sys);
        p.name = name;
        p.age = age;
        p.gender = gender;
        p.trainNo = trainNo;
        p.seatNo = allocatedSeat;
        p.travelDate = travelDate;
        p.status = "CONFIRMED";
        p.concession = concessionTier;
        p.farePaid = finalFare;

        // Atomic SQLite Transaction
        std::lock_guard<std::mutex> lock(g_sysMutex);
        beginTransaction(db);

        int seatIdx = allocatedSeat - 1;
        sys.seatMap[trainIdx][seatIdx] = 1;
        sys.trains[trainIdx].availableSeats--;

        bool ok1 = updateTrainSeats(db, trainNo, sys.trains[trainIdx].availableSeats);
        bool ok2 = insertPassenger(db, p);

        if (ok1 && ok2) {
            commitTransaction(db);
            sys.passengers.push_back(p);

            cout << "\n========================================================\n";
            cout << "               TICKET BOOKED SUCCESSFULLY!             \n";
            cout << "========================================================\n";
            cout << "PNR Number       : " << p.pnr << "\n";
            cout << "Passenger Name   : " << p.name << " (Age: " << p.age << ", Gender: " << p.gender << ")\n";
            cout << "Train            : " << sys.trains[trainIdx].name << " (#" << trainNo << ")\n";
            cout << "Seat Number      : " << p.seatNo << "\n";
            cout << "Travel Date      : " << p.travelDate.day << "/" << p.travelDate.month << "/" << p.travelDate.year << "\n";
            cout << "Concession Tier  : " << p.concession << "\n";
            cout << "Base Train Fare  : Rs. " << fixed << setprecision(2) << sys.trains[trainIdx].fare << "\n";
            cout << "Final Fare Paid  : Rs. " << fixed << setprecision(2) << p.farePaid << "\n";
            cout << "Status           : CONFIRMED\n";
            cout << "========================================================\n";

            if (exportTicketToFile(p, sys.trains[trainIdx])) {
                cout << "[E-Ticket Generated] Saved to file: ticket_" << p.pnr << ".txt\n";
            }
        } else {
            rollbackTransaction(db);
            // Revert in-memory seat allocation
            sys.seatMap[trainIdx][seatIdx] = 0;
            sys.trains[trainIdx].availableSeats++;
            cout << "[Database Error] Transaction aborted! Failed to record booking.\n";
        }
    }
    // CASE 2: Train is full -> Queue into FIFO waiting list
    else {
        cout << "\n[Notice] Train #" << trainNo << " is fully booked! (Available Seats: 0)\n";
        
        std::lock_guard<std::mutex> lock(g_sysMutex);
        queue<WaitingEntry>& wQueue = sys.waitingLists[trainNo];
        if ((int)wQueue.size() >= MAX_WAITING) {
            cout << "[Error] Waiting list is also FULL (max " << MAX_WAITING << " passengers). Booking closed.\n";
            return;
        }

        WaitingEntry w;
        w.name = name;
        w.age = age;
        w.gender = gender;
        w.trainNo = trainNo;
        w.travelDate = travelDate;

        if (insertWaiting(db, w)) {
            wQueue.push(w); // FIFO Queue push

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
        } else {
            cout << "[Database Error] Failed to record waiting list entry.\n";
        }
    }
}

/*
  FUNCTION: findPassengerByPNR
  PURPOSE: Performs Linear Search to find a passenger by PNR number.
  TIME COMPLEXITY: O(P)
*/
int findPassengerByPNR(const RailwaySystem& sys, int pnr) {
    for (size_t i = 0; i < sys.passengers.size(); i++) {
        if (sys.passengers[i].pnr == pnr) {
            return (int)i;
        }
    }
    return -1;
}

/*
  FUNCTION: promoteFromWaitingList
  PURPOSE: Automatically promotes the head of the waiting queue into a freed seat.
  TIME COMPLEXITY: O(1)
  ALGORITHM NOTE: Pops FIFO queue front element and reassigns newly vacant seat in O(1) time.
*/
bool promoteFromWaitingList(RailwaySystem& sys, sqlite3* db, int trainIndex, int seatNo) {
    int trainNo = sys.trains[trainIndex].trainNo;
    queue<WaitingEntry>& wQueue = sys.waitingLists[trainNo];

    if (!wQueue.empty()) {
        WaitingEntry topWait = wQueue.front(); // Peek FIFO head
        wQueue.pop();                          // Remove from queue

        deleteWaiting(db, topWait.waitId);

        string concessionTier;
        float finalFare = 0.0f;
        calculateConcession(topWait.age, sys.trains[trainIndex].fare, concessionTier, finalFare);

        Passenger promoted;
        promoted.pnr = generatePNR(sys);
        promoted.name = topWait.name;
        promoted.age = topWait.age;
        promoted.gender = topWait.gender;
        promoted.trainNo = topWait.trainNo;
        promoted.seatNo = seatNo;
        promoted.travelDate = topWait.travelDate;
        promoted.status = "CONFIRMED";
        promoted.concession = concessionTier;
        promoted.farePaid = finalFare;

        sys.seatMap[trainIndex][seatNo - 1] = 1; // Mark re-booked
        sys.passengers.push_back(promoted);
        insertPassenger(db, promoted);
        exportTicketToFile(promoted, sys.trains[trainIndex]);

        cout << "\n>>> [AUTO-PROMOTION EVENT: FIFO QUEUE IN ACTION] <<<\n";
        cout << "Waiting passenger " << promoted.name << " was promoted to Seat #" << seatNo
             << " with newly generated PNR: " << promoted.pnr << "!\n";
        return true;
    } else {
        // No one was waiting; seat remains vacant
        sys.trains[trainIndex].availableSeats++;
        updateTrainSeats(db, trainNo, sys.trains[trainIndex].availableSeats);
        return false;
    }
}

/*
  FUNCTION: cancelTicket
  PURPOSE: Cancels a ticket by PNR, frees the 2D seatMap, pushes to cancellation stack,
           and triggers auto-promotion if passengers are queued.
  TIME COMPLEXITY: O(P + log N)
*/
void cancelTicket(RailwaySystem& sys, sqlite3* db) {
    cout << "\n--- Cancel Confirmed Ticket ---\n";
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

    std::lock_guard<std::mutex> lock(g_sysMutex);
    beginTransaction(db);

    p.status = "CANCELLED";
    updatePassengerStatus(db, pnr, "CANCELLED");

    // Push cancelled ticket onto LIFO stack (Module VIII)
    sys.recentCancellations.push(p);

    int trainIdx = findTrainIndex(sys, p.trainNo);
    int freedSeat = p.seatNo;
    sys.seatMap[trainIdx][freedSeat - 1] = 0; // Free seat in 2D array

    commitTransaction(db);
    cout << "\n[Success] Ticket PNR " << pnr << " has been CANCELLED successfully.\n";

    // Auto-promote waiting passenger if one exists
    promoteFromWaitingList(sys, db, trainIdx, freedSeat);
}

/*
  FUNCTION: cancelWaitingListEntry
  PURPOSE: Cancels a passenger currently queued in the FIFO waiting list.
  ALGORITHM:
    1. Finds the queue for the specified train.
    2. Uses a secondary temporary queue to filter out the matching entry.
    3. Deletes the row from SQLite waiting_list table.
    4. Restores the remaining entries to the main queue in preserved order.
  ALGORITHM NOTE: Queue filtering preserves strict FIFO ordering for all remaining waiting entries.
*/
void cancelWaitingListEntry(RailwaySystem& sys, sqlite3* db) {
    cout << "\n--- Cancel Waiting List Ticket ---\n";
    int trainNo = readInt("Enter Train Number: ", 1000, 99999);
    string passengerName = readNonEmptyString("Enter Passenger Name: ");

    std::lock_guard<std::mutex> lock(g_sysMutex);
    if (sys.waitingLists.find(trainNo) == sys.waitingLists.end() || sys.waitingLists[trainNo].empty()) {
        cout << "[Notice] No passengers currently on the waiting list for Train #" << trainNo << ".\n";
        return;
    }

    queue<WaitingEntry>& wQueue = sys.waitingLists[trainNo];
    queue<WaitingEntry> tempQueue;
    bool found = false;
    WaitingEntry cancelledItem;

    while (!wQueue.empty()) {
        WaitingEntry top = wQueue.front();
        wQueue.pop();

        if (!found && toLowerCase(top.name) == toLowerCase(passengerName)) {
            found = true;
            cancelledItem = top; // Skip pushing to tempQueue to remove it
        } else {
            tempQueue.push(top);
        }
    }

    // Restore queue
    wQueue = tempQueue;

    if (found) {
        deleteWaiting(db, cancelledItem.waitId);
        cout << "\n[Success] Waiting list entry for " << cancelledItem.name 
             << " on Train #" << trainNo << " has been CANCELLED successfully.\n";
    } else {
        cout << "[Notice] No waiting passenger named '" << passengerName 
             << "' found on train #" << trainNo << ".\n";
    }
}

/*
  FUNCTION: viewLastCancelledTicket
  PURPOSE: Inspects the top of the recent cancellations stack (LIFO).
  TIME COMPLEXITY: O(1)
*/
void viewLastCancelledTicket(const RailwaySystem& sys) {
    if (sys.recentCancellations.empty()) {
        cout << "\n[Notice] No recent cancellations in the stack.\n";
        return;
    }

    const Passenger& p = sys.recentCancellations.top();
    cout << "\n--- Most Recently Cancelled Ticket (Stack Top - LIFO) ---\n";
    cout << "PNR Number     : " << p.pnr << "\n";
    cout << "Passenger Name : " << p.name << "\n";
    cout << "Train Number   : " << p.trainNo << "\n";
    cout << "Seat Number    : " << p.seatNo << "\n";
    cout << "Travel Date    : " << p.travelDate.day << "/" << p.travelDate.month << "/" << p.travelDate.year << "\n";
    cout << "Status         : " << p.status << "\n";
    cout << "---------------------------------------------------------\n";
}

/*
  FUNCTION: undoLastCancellation
  PURPOSE: Pops the top of the cancellation stack and restores the booking if seat is still free.
  TIME COMPLEXITY: O(1)
*/
void undoLastCancellation(RailwaySystem& sys, sqlite3* db) {
    if (sys.recentCancellations.empty()) {
        cout << "\n[Notice] No cancellations available to undo.\n";
        return;
    }

    std::lock_guard<std::mutex> lock(g_sysMutex);
    Passenger last = sys.recentCancellations.top();
    int trainIdx = findTrainIndex(sys, last.trainNo);
    if (trainIdx == -1) {
        cout << "[Error] Associated train #" << last.trainNo << " not found.\n";
        return;
    }

    // Only restore if seat is still vacant
    if (sys.seatMap[trainIdx][last.seatNo - 1] == 0 && sys.trains[trainIdx].availableSeats > 0) {
        sys.recentCancellations.pop(); // Pop from stack
        sys.seatMap[trainIdx][last.seatNo - 1] = 1; // Mark re-booked
        sys.trains[trainIdx].availableSeats--;
        updateTrainSeats(db, last.trainNo, sys.trains[trainIdx].availableSeats);

        int pIdx = findPassengerByPNR(sys, last.pnr);
        if (pIdx != -1) {
            sys.passengers[pIdx].status = "CONFIRMED";
        }
        updatePassengerStatus(db, last.pnr, "CONFIRMED");

        cout << "\n[Success] Cancellation UNDONE successfully!\n";
        cout << "Passenger " << last.name << " restored to Seat #" << last.seatNo
             << " on Train #" << last.trainNo << " with PNR: " << last.pnr << ".\n";
    } else {
        cout << "\n[Error] Cannot undo cancellation for PNR " << last.pnr << ":\n"
             << "Seat #" << last.seatNo << " has already been reallocated to a waiting passenger or another booking!\n";
    }
}

/*
  FUNCTION: displayAvailableSeats
  PURPOSE: Shows seat availability counts and prints a visual 2D seating layout grid.
  TIME COMPLEXITY: O(S) where S is totalSeats.
*/
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
    
    displayTrainSeats(sys, trainIdx);
    cout << "=======================================================\n";
}

/*
  FUNCTION: displayPassengerDetails
  PURPOSE: Submenu to view passenger records by PNR, by train, or all passengers.
  TIME COMPLEXITY: O(P)
*/
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
        if (idx != -1) {
            const Passenger& p = sys.passengers[idx];
            cout << "\n[Passenger Record Found]:\n";
            cout << "  PNR Number       : " << p.pnr << "\n";
            cout << "  Name             : " << p.name << "\n";
            cout << "  Age / Gender     : " << p.age << " / " << p.gender << "\n";
            cout << "  Train Number     : " << p.trainNo << "\n";
            cout << "  Seat Number      : " << p.seatNo << "\n";
            cout << "  Travel Date      : " << p.travelDate.day << "/" << p.travelDate.month << "/" << p.travelDate.year << "\n";
            cout << "  Booking Status   : " << p.status << "\n";
            cout << "  Concession Tier  : " << p.concession << "\n";
            cout << "  Fare Paid        : Rs. " << fixed << setprecision(2) << p.farePaid << "\n";

            int tIdx = findTrainIndex(sys, p.trainNo);
            if (tIdx != -1) {
                cout << "  Would you like to export/print E-Ticket receipt file? (Y/N): ";
                string exportChoice;
                getline(cin, exportChoice);
                if (!exportChoice.empty() && (exportChoice[0] == 'y' || exportChoice[0] == 'Y')) {
                    if (exportTicketToFile(p, sys.trains[tIdx])) {
                        cout << "[Success] E-Ticket exported to file: ticket_" << p.pnr << ".txt\n";
                    }
                }
            }
        } else {
            cout << "[Notice] No passenger found with PNR " << pnr << ".\n";
        }
    } else if (choice == 2) {
        int trainNo = readInt("Enter Train Number: ", 1000, 99999);
        bool found = false;
        cout << "\n====================================================================================================\n";
        cout << setw(8)  << "PNR"
             << setw(20) << "Name"
             << setw(6)  << "Age"
             << setw(6)  << "Gen"
             << setw(6)  << "Seat"
             << setw(14) << "Date"
             << setw(12) << "Status"
             << setw(16) << "Fare Paid" << "\n";
        cout << "====================================================================================================\n";
        for (size_t i = 0; i < sys.passengers.size(); i++) {
            const Passenger& p = sys.passengers[i];
            if (p.trainNo == trainNo) {
                found = true;
                string dateStr = to_string(p.travelDate.day) + "/" + to_string(p.travelDate.month) + "/" + to_string(p.travelDate.year);
                cout << setw(8)  << p.pnr
                     << setw(20) << p.name
                     << setw(6)  << p.age
                     << setw(6)  << p.gender
                     << setw(6)  << p.seatNo
                     << setw(14) << dateStr
                     << setw(12) << p.status
                     << setw(16) << fixed << setprecision(2) << p.farePaid << "\n";
            }
        }
        if (!found) cout << "[Notice] No passenger bookings found on Train #" << trainNo << ".\n";
        cout << "====================================================================================================\n";
    } else {
        cout << "\n====================================================================================================\n";
        cout << setw(8)  << "PNR"
             << setw(20) << "Name"
             << setw(6)  << "Age"
             << setw(6)  << "Gen"
             << setw(8)  << "Train#"
             << setw(6)  << "Seat"
             << setw(12) << "Status"
             << setw(16) << "Fare Paid" << "\n";
        cout << "====================================================================================================\n";
        for (size_t i = 0; i < sys.passengers.size(); i++) {
            const Passenger& p = sys.passengers[i];
            cout << setw(8)  << p.pnr
                 << setw(20) << p.name
                 << setw(6)  << p.age
                 << setw(6)  << p.gender
                 << setw(8)  << p.trainNo
                 << setw(6)  << p.seatNo
                 << setw(12) << p.status
                 << setw(16) << fixed << setprecision(2) << p.farePaid << "\n";
        }
        cout << "====================================================================================================\n";
    }
}

/*
  FUNCTION: displayWaitingList
  PURPOSE: Displays all active waiting queues in First-Come, First-Served order.
  TIME COMPLEXITY: O(W) where W is total waiting passengers.
*/
void displayWaitingList(const RailwaySystem& sys) {
    bool hasWaiting = false;
    cout << "\n--- Current Waiting Lists (FIFO Queues) ---\n";

    for (map<int, queue<WaitingEntry> >::const_iterator it = sys.waitingLists.begin(); 
         it != sys.waitingLists.end(); ++it) {
        
        int trainNo = it->first;
        queue<WaitingEntry> copyQueue = it->second;

        if (!copyQueue.empty()) {
            hasWaiting = true;
            int tIdx = findTrainIndex(sys, trainNo);
            string trainName = (tIdx != -1) ? sys.trains[tIdx].name : "Unknown Train";

            cout << "\nTrain #" << trainNo << " (" << trainName << ") - Waiting Queue (" 
                 << copyQueue.size() << " passengers):\n";
            cout << "----------------------------------------------------------------------\n";
            cout << setw(8)  << "Pos"
                 << setw(22) << "Passenger Name"
                 << setw(6)  << "Age"
                 << setw(6)  << "Gen"
                 << setw(14) << "Travel Date" << "\n";
            cout << "----------------------------------------------------------------------\n";

            int pos = 1;
            while (!copyQueue.empty()) {
                WaitingEntry w = copyQueue.front();
                copyQueue.pop(); // Dequeue next item from copy
                string dateStr = to_string(w.travelDate.day) + "/" + to_string(w.travelDate.month) + "/" + to_string(w.travelDate.year);
                cout << setw(8)  << ("WL-" + to_string(pos++))
                     << setw(22) << w.name
                     << setw(6)  << w.age
                     << setw(6)  << w.gender
                     << setw(14) << dateStr << "\n";
            }
        }
    }

    if (!hasWaiting) {
        cout << "[Notice] All waiting lists are currently empty across the entire network!\n";
    }
}

/*
  FUNCTION: calculateStats
  PURPOSE: Aggregates system metrics (Trains, Bookings, Revenue, Waitlists).
*/
SystemStats calculateStats(const RailwaySystem& sys) {
    SystemStats s;
    s.totalTrains = (int)sys.trains.size();
    s.totalBookings = (int)sys.passengers.size();
    s.confirmedBookings = 0;
    s.cancelledBookings = 0;
    s.totalRevenue = 0.0f;

    for (size_t i = 0; i < sys.passengers.size(); i++) {
        if (sys.passengers[i].status == "CONFIRMED") {
            s.confirmedBookings++;
            s.totalRevenue += sys.passengers[i].farePaid;
        } else if (sys.passengers[i].status == "CANCELLED") {
            s.cancelledBookings++;
        }
    }

    s.totalWaitlisted = 0;
    for (map<int, queue<WaitingEntry> >::const_iterator it = sys.waitingLists.begin(); 
         it != sys.waitingLists.end(); ++it) {
        s.totalWaitlisted += (int)it->second.size();
    }
    return s;
}

/*
  FUNCTION: displaySystemStats
  PURPOSE: Executive report for Administrator Portal.
*/
void displaySystemStats(const RailwaySystem& sys) {
    SystemStats s = calculateStats(sys);
    cout << "\n=======================================================\n";
    cout << "          SYSTEM REVENUE & BOOKING ANALYTICS           \n";
    cout << "=======================================================\n";
    cout << "  Operational Trains       : " << s.totalTrains << "\n";
    cout << "  Total Handled Bookings   : " << s.totalBookings << "\n";
    cout << "  Confirmed Tickets        : " << s.confirmedBookings << "\n";
    cout << "  Cancelled Tickets        : " << s.cancelledBookings << "\n";
    cout << "  Passengers on Waitlist   : " << s.totalWaitlisted << "\n";
    cout << "  Net Realized Revenue     : Rs. " << fixed << setprecision(2) << s.totalRevenue << "\n";
    cout << "=======================================================\n";
}

/*
  FUNCTION: loadSystem
  PURPOSE: Reads active database state into working memory.
*/
void loadSystem(RailwaySystem& sys, sqlite3* db) {
    for (int t = 0; t < MAX_TRAINS; t++) {
        for (int s = 0; s < MAX_SEATS; s++) {
            sys.seatMap[t][s] = 0;
        }
    }

    loadTrains(db, sys.trains);
    loadPassengers(db, sys.passengers);
    loadWaitingList(db, sys.waitingLists);

    // Rebuild 2D seatMap from confirmed passengers
    for (size_t i = 0; i < sys.passengers.size(); i++) {
        const Passenger& p = sys.passengers[i];
        if (p.status == "CONFIRMED") {
            int trainIdx = findTrainIndex(sys, p.trainNo);
            if (trainIdx != -1 && p.seatNo >= 1 && p.seatNo <= MAX_SEATS) {
                sys.seatMap[trainIdx][p.seatNo - 1] = 1;
            }
        }
    }
}


// ========================================================================================================
// SECTION 5: EMBEDDED WINSOCK HTTP SERVER & REST API (FULL-STACK INTEGRATION)
// ========================================================================================================

/*
  INSTRUCTION / EXPLANATION:
  - Embeds a lightweight HTTP/1.1 server inside C++ using standard Windows Sockets (winsock2.h).
  - Listens on Port 8080 and serves the Single-Page Application (HTML/CSS/JS).
  - Handles REST endpoints: /api/trains, /api/seats, /api/pnr, /api/stats, /api/manifest, /api/ticket, /api/book, /api/cancel.
  - Allows simultaneous terminal and browser interaction via std::mutex thread synchronization.
*/

// Helper to extract query parameters: /path?key=value
string getQueryParam(const string& url, const string& param) {
    size_t qPos = url.find('?');
    if (qPos == string::npos) return "";
    string query = url.substr(qPos + 1);
    size_t pPos = query.find(param + "=");
    if (pPos == string::npos) return "";
    size_t valStart = pPos + param.length() + 1;
    size_t valEnd = query.find('&', valStart);
    if (valEnd == string::npos) valEnd = query.length();
    return query.substr(valStart, valEnd - valStart);
}

// Simple JSON field extractor for basic types without external dependencies
string extractJsonField(const string& json, const string& key) {
    string target = "\"" + key + "\":";
    size_t pos = json.find(target);
    if (pos == string::npos) return "";
    pos += target.length();
    while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
    if (pos >= json.length()) return "";

    if (json[pos] == '\"') {
        pos++;
        size_t endPos = json.find('\"', pos);
        if (endPos == string::npos) return "";
        return json.substr(pos, endPos - pos);
    } else {
        size_t endPos = pos;
        while (endPos < json.length() && json[endPos] != ',' && json[endPos] != '}' && json[endPos] != '\r' && json[endPos] != '\n' && json[endPos] != ' ') {
            endPos++;
        }
        return json.substr(pos, endPos - pos);
    }
}

// In-memory HTML fallback in case web/index.html is not present on disk
const char* FALLBACK_HTML = 
    "<!DOCTYPE html><html><head><title>Railway Reservation</title></head>"
    "<body style='font-family:sans-serif;background:#0f172a;color:#fff;padding:2rem;text-align:center;'>"
    "<h2>Indian Railways Reservation System</h2>"
    "<p>Embedded C++ Winsock Backend is running online on port 8080.</p>"
    "<p>Please ensure <code>web/index.html</code> is present in the working directory for the full dashboard.</p>"
    "</body></html>";

// Handles incoming client HTTP request
void handleHttpClient(SOCKET clientSocket, RailwaySystem& sys, sqlite3* db) {
    char buffer[8192];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) return;
    
    string request(buffer, bytesReceived);
    size_t headerEnd = request.find("\r\n\r\n");
    while (headerEnd == string::npos) {
        int more = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (more <= 0) break;
        request.append(buffer, more);
        headerEnd = request.find("\r\n\r\n");
    }

    // Read full body if Content-Length specified
    size_t clPos = request.find("Content-Length:");
    if (clPos == string::npos) clPos = request.find("content-length:");
    if (clPos != string::npos && headerEnd != string::npos) {
        size_t valStart = clPos + 15;
        while (valStart < request.length() && (request[valStart] == ' ' || request[valStart] == '\t')) valStart++;
        size_t valEnd = request.find("\r\n", valStart);
        if (valEnd != string::npos) {
            int contentLength = atoi(request.substr(valStart, valEnd - valStart).c_str());
            size_t currentBodyLen = request.length() - (headerEnd + 4);
            while ((int)currentBodyLen < contentLength) {
                int more = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
                if (more <= 0) break;
                request.append(buffer, more);
                currentBodyLen += more;
            }
        }
    }

    istringstream reqStream(request);
    string method, path, protocol;
    reqStream >> method >> path >> protocol;

    string responseHeaders = "";
    string responseBody = "";

    // 1. SERVE FRONTEND (Single Page Application)
    if (path == "/" || path == "/index.html") {
        ifstream htmlFile("web/index.html");
        if (htmlFile.is_open()) {
            stringstream ss;
            ss << htmlFile.rdbuf();
            responseBody = ss.str();
            htmlFile.close();
        } else {
            responseBody = FALLBACK_HTML;
        }
        responseHeaders = "HTTP/1.1 200 OK\r\n"
                          "Content-Type: text/html; charset=utf-8\r\n"
                          "Content-Length: " + to_string(responseBody.length()) + "\r\n"
                          "Access-Control-Allow-Origin: *\r\n"
                          "Connection: close\r\n\r\n";
    }
    // 2. REST API: GET ALL TRAINS
    else if (path == "/api/trains") {
        std::lock_guard<std::mutex> lock(g_sysMutex);
        stringstream json;
        json << "[";
        for (size_t i = 0; i < sys.trains.size(); i++) {
            const Train& t = sys.trains[i];
            json << "{\"trainNo\":" << t.trainNo
                 << ",\"name\":\"" << t.name << "\""
                 << ",\"source\":\"" << t.source << "\""
                 << ",\"destination\":\"" << t.destination << "\""
                 << ",\"departure\":\"" << t.departure << "\""
                 << ",\"totalSeats\":" << t.totalSeats
                 << ",\"availableSeats\":" << t.availableSeats
                 << ",\"fare\":" << fixed << setprecision(2) << t.fare
                 << "}";
            if (i + 1 < sys.trains.size()) json << ",";
        }
        json << "]";
        responseBody = json.str();
        responseHeaders = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " 
                          + to_string(responseBody.length()) + "\r\nConnection: close\r\n\r\n";
    }
    // 3. REST API: GET SEAT MAP FOR TRAIN
    else if (path.find("/api/seats") == 0) {
        string tNoStr = getQueryParam(path, "trainNo");
        int tNo = atoi(tNoStr.c_str());

        std::lock_guard<std::mutex> lock(g_sysMutex);
        int tIdx = findTrainIndex(sys, tNo);
        if (tIdx != -1) {
            stringstream json;
            json << "{\"trainNo\":" << tNo
                 << ",\"totalSeats\":" << sys.trains[tIdx].totalSeats
                 << ",\"availableSeats\":" << sys.trains[tIdx].availableSeats
                 << ",\"seats\":[";
            for (int s = 0; s < sys.trains[tIdx].totalSeats; s++) {
                json << sys.seatMap[tIdx][s];
                if (s + 1 < sys.trains[tIdx].totalSeats) json << ",";
            }
            json << "]}";
            responseBody = json.str();
        } else {
            responseBody = "{\"error\":\"Train not found\"}";
        }
        responseHeaders = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " 
                          + to_string(responseBody.length()) + "\r\nConnection: close\r\n\r\n";
    }
    // 4. REST API: GET PNR STATUS
    else if (path.find("/api/pnr") == 0) {
        string pnrStr = getQueryParam(path, "pnr");
        int pnr = atoi(pnrStr.c_str());

        std::lock_guard<std::mutex> lock(g_sysMutex);
        int pIdx = findPassengerByPNR(sys, pnr);
        if (pIdx != -1) {
            const Passenger& p = sys.passengers[pIdx];
            stringstream json;
            json << "{\"found\":true,\"pnr\":" << p.pnr
                 << ",\"name\":\"" << p.name << "\""
                 << ",\"age\":" << p.age
                 << ",\"gender\":\"" << p.gender << "\""
                 << ",\"trainNo\":" << p.trainNo
                 << ",\"seatNo\":" << p.seatNo
                 << ",\"date\":\"" << p.travelDate.day << "/" << p.travelDate.month << "/" << p.travelDate.year << "\""
                 << ",\"status\":\"" << p.status << "\""
                 << ",\"concession\":\"" << p.concession << "\""
                 << ",\"farePaid\":" << fixed << setprecision(2) << p.farePaid
                 << "}";
            responseBody = json.str();
        } else {
            responseBody = "{\"found\":false}";
        }
        responseHeaders = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " 
                          + to_string(responseBody.length()) + "\r\nConnection: close\r\n\r\n";
    }
    // 5. REST API: GET EXECUTIVE SYSTEM STATS
    else if (path == "/api/stats") {
        std::lock_guard<std::mutex> lock(g_sysMutex);
        SystemStats s = calculateStats(sys);
        stringstream json;
        json << "{\"totalTrains\":" << s.totalTrains
             << ",\"totalBookings\":" << s.totalBookings
             << ",\"confirmedBookings\":" << s.confirmedBookings
             << ",\"cancelledBookings\":" << s.cancelledBookings
             << ",\"totalWaitlisted\":" << s.totalWaitlisted
             << ",\"totalRevenue\":" << fixed << setprecision(2) << s.totalRevenue
             << "}";
        responseBody = json.str();
        responseHeaders = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " 
                          + to_string(responseBody.length()) + "\r\nConnection: close\r\n\r\n";
    }
    // 6. REST API: GET PASSENGER MANIFEST
    else if (path == "/api/manifest") {
        std::lock_guard<std::mutex> lock(g_sysMutex);
        stringstream json;
        json << "[";
        for (size_t i = 0; i < sys.passengers.size(); i++) {
            const Passenger& p = sys.passengers[i];
            json << "{\"pnr\":" << p.pnr
                 << ",\"name\":\"" << p.name << "\""
                 << ",\"age\":" << p.age
                 << ",\"gender\":\"" << p.gender << "\""
                 << ",\"trainNo\":" << p.trainNo
                 << ",\"seatNo\":" << p.seatNo
                 << ",\"date\":\"" << p.travelDate.day << "/" << p.travelDate.month << "/" << p.travelDate.year << "\""
                 << ",\"status\":\"" << p.status << "\""
                 << ",\"concession\":\"" << p.concession << "\""
                 << ",\"farePaid\":" << fixed << setprecision(2) << p.farePaid
                 << "}";
            if (i + 1 < sys.passengers.size()) json << ",";
        }
        json << "]";
        responseBody = json.str();
        responseHeaders = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " 
                          + to_string(responseBody.length()) + "\r\nConnection: close\r\n\r\n";
    }
    // 7. REST API: DOWNLOAD E-TICKET PLAIN TEXT
    else if (path.find("/api/ticket") == 0) {
        string pnrStr = getQueryParam(path, "pnr");
        int pnr = atoi(pnrStr.c_str());
        string fileName = "ticket_" + to_string(pnr) + ".txt";

        ifstream tFile(fileName.c_str());
        if (tFile.is_open()) {
            stringstream ss;
            ss << tFile.rdbuf();
            responseBody = ss.str();
            tFile.close();
        } else {
            responseBody = "Ticket receipt file not found on disk.";
        }
        responseHeaders = "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=utf-8\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " 
                          + to_string(responseBody.length()) + "\r\nConnection: close\r\n\r\n";
    }
    // 8. REST API: POST BOOK TICKET
    else if (method == "POST" && path == "/api/book") {
        size_t bodyPos = request.find("\r\n\r\n");
        string body = (bodyPos != string::npos) ? request.substr(bodyPos + 4) : "";

        int trainNo = atoi(extractJsonField(body, "trainNo").c_str());
        string name = extractJsonField(body, "name");
        int age = atoi(extractJsonField(body, "age").c_str());
        string gStr = extractJsonField(body, "gender");
        char gender = (!gStr.empty()) ? gStr[0] : 'M';
        string dateStr = extractJsonField(body, "dateStr");
        int seatNo = atoi(extractJsonField(body, "seatNo").c_str());

        // Parse YYYY-MM-DD
        Date travelDate;
        travelDate.year = 2026; travelDate.month = 10; travelDate.day = 15;
        if (dateStr.length() >= 10) {
            travelDate.year = atoi(dateStr.substr(0, 4).c_str());
            travelDate.month = atoi(dateStr.substr(5, 2).c_str());
            travelDate.day = atoi(dateStr.substr(8, 2).c_str());
        }

        std::lock_guard<std::mutex> lock(g_sysMutex);
        int trainIdx = findTrainIndex(sys, trainNo);
        if (trainIdx == -1) {
            responseBody = "{\"success\":false,\"error\":\"Train does not exist\"}";
        } else {
            // Check availability
            int freeSeat = findFreeSeat(sys, trainIdx);
            if (freeSeat != -1 && sys.trains[trainIdx].availableSeats > 0) {
                int allocated = (seatNo >= 1 && seatNo <= sys.trains[trainIdx].totalSeats && sys.seatMap[trainIdx][seatNo - 1] == 0)
                                ? seatNo : freeSeat;

                string concessionTier;
                float finalFare = 0.0f;
                calculateConcession(age, sys.trains[trainIdx].fare, concessionTier, finalFare);

                Passenger p;
                p.pnr = generatePNR(sys);
                p.name = name;
                p.age = age;
                p.gender = gender;
                p.trainNo = trainNo;
                p.seatNo = allocated;
                p.travelDate = travelDate;
                p.status = "CONFIRMED";
                p.concession = concessionTier;
                p.farePaid = finalFare;

                beginTransaction(db);
                sys.seatMap[trainIdx][allocated - 1] = 1;
                sys.trains[trainIdx].availableSeats--;
                updateTrainSeats(db, trainNo, sys.trains[trainIdx].availableSeats);
                insertPassenger(db, p);
                commitTransaction(db);

                sys.passengers.push_back(p);
                exportTicketToFile(p, sys.trains[trainIdx]);

                stringstream json;
                json << "{\"success\":true,\"status\":\"CONFIRMED\",\"pnr\":" << p.pnr
                     << ",\"name\":\"" << p.name << "\""
                     << ",\"age\":" << p.age
                     << ",\"seatNo\":" << p.seatNo
                     << ",\"concession\":\"" << p.concession << "\""
                     << ",\"farePaid\":" << fixed << setprecision(2) << p.farePaid
                     << "}";
                responseBody = json.str();
            } else {
                queue<WaitingEntry>& wQueue = sys.waitingLists[trainNo];
                if ((int)wQueue.size() >= MAX_WAITING) {
                    responseBody = "{\"success\":false,\"error\":\"Train and Waiting List are both FULL.\"}";
                } else {
                    WaitingEntry w;
                    w.name = name;
                    w.age = age;
                    w.gender = gender;
                    w.trainNo = trainNo;
                    w.travelDate = travelDate;

                    insertWaiting(db, w);
                    wQueue.push(w);

                    stringstream json;
                    json << "{\"success\":true,\"status\":\"WAITING\",\"waitPos\":" << wQueue.size()
                         << ",\"name\":\"" << w.name << "\"}";
                    responseBody = json.str();
                }
            }
        }
        responseHeaders = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " 
                          + to_string(responseBody.length()) + "\r\nConnection: close\r\n\r\n";
    }
    // 9. REST API: POST CANCEL TICKET
    else if (method == "POST" && path == "/api/cancel") {
        size_t bodyPos = request.find("\r\n\r\n");
        string body = (bodyPos != string::npos) ? request.substr(bodyPos + 4) : "";
        int pnr = atoi(extractJsonField(body, "pnr").c_str());

        std::lock_guard<std::mutex> lock(g_sysMutex);
        int pIdx = findPassengerByPNR(sys, pnr);

        if (pIdx == -1) {
            responseBody = "{\"success\":false,\"error\":\"Ticket PNR not found\"}";
        } else if (sys.passengers[pIdx].status == "CANCELLED") {
            responseBody = "{\"success\":false,\"error\":\"Ticket already cancelled\"}";
        } else {
            Passenger& p = sys.passengers[pIdx];
            beginTransaction(db);
            p.status = "CANCELLED";
            updatePassengerStatus(db, pnr, "CANCELLED");
            sys.recentCancellations.push(p);

            int trainIdx = findTrainIndex(sys, p.trainNo);
            int freedSeat = p.seatNo;
            sys.seatMap[trainIdx][freedSeat - 1] = 0;
            commitTransaction(db);

            bool promoted = promoteFromWaitingList(sys, db, trainIdx, freedSeat);
            string msg = promoted ? "Freed seat was automatically allocated to the waiting list passenger!"
                                  : "Seat is now free for booking.";
            responseBody = "{\"success\":true,\"message\":\"" + msg + "\"}";
        }
        responseHeaders = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " 
                          + to_string(responseBody.length()) + "\r\nConnection: close\r\n\r\n";
    }
    // 404 NOT FOUND
    else {
        responseBody = "{\"error\":\"Not Found\"}";
        responseHeaders = "HTTP/1.1 404 Not Found\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " 
                          + to_string(responseBody.length()) + "\r\nConnection: close\r\n\r\n";
    }

    send(clientSocket, responseHeaders.c_str(), (int)responseHeaders.length(), 0);
    send(clientSocket, responseBody.c_str(), (int)responseBody.length(), 0);
}

// Background thread loop listening on port 8080
void runHttpServer(RailwaySystem* sysPtr, sqlite3* dbPtr, int port) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return;
#endif

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        CLOSE_SOCKET(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }

    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        CLOSE_SOCKET(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }

    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        if (clientSocket == INVALID_SOCKET) break;

        handleHttpClient(clientSocket, *sysPtr, dbPtr);
        CLOSE_SOCKET(clientSocket);
    }

    CLOSE_SOCKET(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif
}


// ========================================================================================================
// SECTION 6: PORTAL MENUS & MAIN ENTRY POINT (MODULE II)
// ========================================================================================================

/*
  FUNCTION: displayPortalSelectionMenu
  PURPOSE: Top-level entry selector between Passenger Kiosk, Admin Portal, and Web Dashboard.
*/
void displayPortalSelectionMenu() {
    cout << "\n=======================================================\n";
    cout << "           RAILWAY TICKET RESERVATION SYSTEM           \n";
    cout << "=======================================================\n";
    cout << "  1. Passenger Portal (Book, Cancel, Status, E-Ticket)\n";
    cout << "  2. Administrator Portal (PIN Protected: Manifest, Stats)\n";
    cout << "  3. Launch Modern Web Dashboard (http://localhost:8080)\n";
    cout << "  0. Exit Application\n";
    cout << "=======================================================\n";
}

/*
  FUNCTION: displayPassengerMenu
  PURPOSE: Dedicated passenger portal options.
*/
void displayPassengerMenu() {
    cout << "\n-------------------------------------------------------\n";
    cout << "                   PASSENGER PORTAL                    \n";
    cout << "-------------------------------------------------------\n";
    cout << "  1. View Train Schedules & Fares\n";
    cout << "  2. Search Train (By Number or Destination)\n";
    cout << "  3. Check Available Seats & 2D Coach Map\n";
    cout << "  4. Book a Ticket (Manual Seat & Age Concession)\n";
    cout << "  5. Cancel Confirmed Ticket (by PNR)\n";
    cout << "  6. Cancel Waiting List Entry\n";
    cout << "  7. View PNR Status & Print E-Ticket Slip\n";
    cout << "  8. Sort Trains for Display (Bubble Sort)\n";
    cout << "  0. Return to Main Portal Menu\n";
    cout << "-------------------------------------------------------\n";
}

/*
  FUNCTION: displayAdminMenu
  PURPOSE: Dedicated administrator management options.
*/
void displayAdminMenu() {
    cout << "\n-------------------------------------------------------\n";
    cout << "              ADMINISTRATOR MANAGEMENT PORTAL          \n";
    cout << "-------------------------------------------------------\n";
    cout << "  1. Add New Train Record\n";
    cout << "  2. View All Trains\n";
    cout << "  3. View Complete Passenger Manifest (All Records)\n";
    cout << "  4. View All Waiting List Queues\n";
    cout << "  5. View Complete Seat Grid Matrix\n";
    cout << "  6. Revenue & Booking Statistics Summary\n";
    cout << "  7. Recent Cancellations (Stack LIFO) & Undo\n";
    cout << "  0. Return to Main Portal Menu\n";
    cout << "-------------------------------------------------------\n";
}

/*
  FUNCTION: runPassengerPortal
  PURPOSE: Manages the Passenger Portal interactive loop.
*/
void runPassengerPortal(RailwaySystem& sys, sqlite3* db) {
    int pChoice = -1;
    do {
        displayPassengerMenu();
        pChoice = readInt("Select an option (0 - 8): ", 0, 8);

        switch (pChoice) {
            case 1:
                displayTrains(sys);
                break;
            case 2: {
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
            case 3:
                displayAvailableSeats(sys);
                break;
            case 4:
                bookTicket(sys, db);
                break;
            case 5:
                cancelTicket(sys, db);
                break;
            case 6:
                cancelWaitingListEntry(sys, db);
                break;
            case 7:
                displayPassengerDetails(sys);
                break;
            case 8:
                sortTrains(sys);
                break;
            case 0:
                cout << "\nReturning to Main Portal Menu...\n";
                break;
            default:
                break;
        }
    } while (pChoice != 0);
}

/*
  FUNCTION: runAdminPortal
  PURPOSE: Authenticates administrator PIN and runs management loop.
*/
void runAdminPortal(RailwaySystem& sys, sqlite3* db) {
    cout << "\n[Security Authentication Required]\n";
    string pin = readNonEmptyString("Enter Administrator PIN: ");

    const char* envPin = getenv("ADMIN_PIN");
    string requiredPin = (envPin != NULL && string(envPin).length() > 0) ? string(envPin) : "admin123";

    if (pin != requiredPin) {
        cout << "[Access Denied] Incorrect administrator PIN!\n";
        return;
    }

    cout << "[Access Granted] Welcome, Administrator.\n";

    int aChoice = -1;
    do {
        displayAdminMenu();
        aChoice = readInt("Select an option (0 - 7): ", 0, 7);

        switch (aChoice) {
            case 1:
                addTrain(sys, db);
                break;
            case 2:
                displayTrains(sys);
                break;
            case 3:
                displayPassengerDetails(sys);
                break;
            case 4:
                displayWaitingList(sys);
                break;
            case 5:
                displayAvailableSeats(sys);
                break;
            case 6:
                displaySystemStats(sys);
                break;
            case 7: {
                cout << "\n--- Recent Cancellations (Stack LIFO) ---\n";
                cout << "1. View Most Recently Cancelled Ticket (Stack Top)\n";
                cout << "2. Undo Last Cancellation (Restore Seat)\n";
                int sChoice = readInt("Select option (1 or 2): ", 1, 2);
                if (sChoice == 1) {
                    viewLastCancelledTicket(sys);
                } else {
                    undoLastCancellation(sys, db);
                }
                break;
            }
            case 0:
                cout << "\nReturning to Main Portal Menu...\n";
                break;
            default:
                break;
        }
    } while (aChoice != 0);
}

/*
  FUNCTION: main
  PURPOSE: Application entry point.
*/
int main(int argc, char* argv[]) {
    // Command-line flag inspection
    if (argc > 1) {
        string arg = argv[1];
        if (arg == "--version" || arg == "-v") {
            cout << "Railway Ticket Reservation System v2.0.0 (C++11/SQLite3/Winsock)\n";
            cout << "High-Performance In-Memory Data Structures & Embedded REST Architecture\n";
            return 0;
        }
        if (arg == "--help" || arg == "-h") {
            cout << "Usage: railway.exe [OPTIONS]\n";
            cout << "  --version, -v   Display software version and environment information\n";
            cout << "  --help, -h      Display command-line help flags\n";
            return 0;
        }
    }

    cout << "\n=======================================================\n";
    cout << " >>> Starting Railway Ticket Reservation System v2 <<< \n";
    cout << "=======================================================\n";

    sqlite3* db = NULL;
    // Step 1: Open SQLite Database connection
    if (!openDatabase(db, "railway.db")) {
        cout << "[Fatal Error] Unable to connect to railway.db. Exiting.\n";
        return 1;
    }

    // Step 2: Ensure tables exist
    if (!createTables(db)) {
        cout << "[Fatal Error] Unable to initialize database tables. Exiting.\n";
        closeDatabase(db);
        return 1;
    }

    // Step 3: Load active data from SQLite into working memory
    RailwaySystem sys;
    loadSystem(sys, db);
    cout << "[System Ready] Loaded " << sys.trains.size() << " trains, " 
         << sys.passengers.size() << " passenger records from railway.db.\n";

    // Step 4: Launch embedded background HTTP server on port 8080
    std::thread webServerThread(runHttpServer, &sys, db, 8080);
    webServerThread.detach();
    cout << "[Web Server] Online and listening on: http://localhost:8080\n";
    cout << "             (Open this URL in any web browser to view the modern UI!)\n";

    // Step 5: Interactive portal selection loop
    int mainChoice = -1;
    do {
        displayPortalSelectionMenu();
        mainChoice = readInt("Select an option (0 - 3): ", 0, 3);

        switch (mainChoice) {
            case 1:
                runPassengerPortal(sys, db);
                break;
            case 2:
                runAdminPortal(sys, db);
                break;
            case 3:
                cout << "\nOpening Web Dashboard in default browser: http://localhost:8080\n";
#ifdef _WIN32
                system("start http://localhost:8080");
#elif __APPLE__
                system("open http://localhost:8080");
#else
                system("xdg-open http://localhost:8080");
#endif
                break;
            case 0:
                cout << "\nSaving system state and shutting down. Have a safe journey!\n";
                break;
            default:
                break;
        }

    } while (mainChoice != 0);

    // Step 6: Close SQLite database connection cleanly
    closeDatabase(db);
    return 0;
}
