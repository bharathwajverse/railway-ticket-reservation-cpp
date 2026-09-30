/*
  ========================================================================================================
  PROJECT: Railway Ticket Reservation System (Group 4)
  FILE: main.cpp (Complete All-in-One C++ Implementation)
  
  TARGET AUDIENCE: 1st Year B.Tech CSE (AI/ML)
  SYLLABUS CONSTRAINTS ENFORCED:
    - Pure C++11 procedural paradigm using 'struct' (No 'class', no OOP inheritance, no polymorphism).
    - No templates, no lambdas, no 'auto', no smart pointers, no 'try/catch' exceptions.
    - STL Containers allowed: vector, queue, map, stack, string.
    - Manual DSA: Binary Search, Linear Search, Bubble Sort written from scratch.
    - Dual Storage: Fast in-memory working models (vector, 2D array, queue) synchronized with SQLite 3.
  
  TABLE OF CONTENTS:
    SECTION 1: SYSTEM CONSTANTS & DATA STRUCTURES (Date, Train, Passenger, WaitingEntry, RailwaySystem)
    SECTION 2: INPUT VALIDATION & STRING UTILITIES (Robust cin handling, leap year check, pattern match)
    SECTION 3: SQLITE DATABASE LAYER (Prepared statements, CRUD operations, state persistence)
    SECTION 4: CORE RAILWAY BUSINESS LOGIC & DSA ALGORITHMS (Add, Search, Bubble Sort, Book, Cancel, Queue)
    SECTION 5: MAIN MENU & SYSTEM ENTRY POINT (do-while menu loop, switch dispatch, clean lifecycle)
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
#include "sqlite3.h"

using namespace std;

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
  WHY USED: Nested inside Passenger and WaitingEntry to cleanly group date attributes.
  HOW TO EXPLAIN TO PROFESSOR:
  "Sir, Date is a nested structure. Instead of passing 3 separate integer variables
   everywhere, we bundle day, month, and year into one clean data type."
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
  HOW TO EXPLAIN TO PROFESSOR:
  "Sir, struct Train groups all heterogeneous attributes belonging to a single train entity."
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
  ATTRIBUTES: Unique PNR, name, age, gender, trainNo, seatNo, travelDate, status ("CONFIRMED"/"CANCELLED").
  HOW TO EXPLAIN TO PROFESSOR:
  "Sir, each ticket issued generates a Passenger structure with a unique PNR and assigned seat."
*/
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

/*
  STRUCTURE: WaitingEntry
  PURPOSE: Represents a passenger placed on the waiting list when a train is full.
  HOW TO EXPLAIN TO PROFESSOR:
  "Sir, when all seats are full, passengers do not get a seat number immediately.
   Instead, they are queued as a WaitingEntry in First-Come, First-Served order."
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
  STRUCTURE: RailwaySystem
  PURPOSE: Holds the entire active in-memory state of the railway application.
  DATA STRUCTURES INCLUDED:
    1. vector<Train> trains: Contiguous dynamic array, kept sorted by trainNo for Binary Search.
    2. vector<Passenger> passengers: Dynamic list of all booked and cancelled tickets.
    3. map<int, queue<WaitingEntry> > waitingLists: Associates each trainNo with a FIFO waiting queue.
    4. int seatMap[MAX_TRAINS][MAX_SEATS]: 2D array tracking seat occupancy (0 = free, 1 = booked).
    5. stack<Passenger> recentCancellations: LIFO stack tracking recently cancelled tickets for undo.
  HOW TO EXPLAIN TO PROFESSOR:
  "Sir, RailwaySystem holds our working memory. 2D array gives O(1) seat checks, queue ensures
   FIFO waiting lists, stack gives LIFO undo, and vector keeps trains sorted for Binary Search."
*/
struct RailwaySystem {
    vector<Train> trains;
    vector<Passenger> passengers;
    map<int, queue<WaitingEntry> > waitingLists;
    int seatMap[MAX_TRAINS][MAX_SEATS];
    stack<Passenger> recentCancellations;
};


// ========================================================================================================
// SECTION 2: INPUT VALIDATION & STRING UTILITY FUNCTIONS (MODULES I, II & V)
// ========================================================================================================

/*
  FUNCTION: isLeapYear
  PURPOSE: Determines whether a given year is a leap year.
  TIME COMPLEXITY: O(1)
  HOW TO EXPLAIN: "A year is leap if divisible by 4 but not 100, unless also divisible by 400."
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
  HOW TO EXPLAIN: "We check month range 1-12, days per month using a 1D array, and 29 days for Feb in leap years."
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
  HOW TO EXPLAIN: "We iterate character by character and convert each using tolower() from cctype."
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
  HOW TO EXPLAIN: "Sir, this implements manual substring matching so searching 'mumb' matches 'Mumbai'."
*/
bool containsIgnoreCase(const string& text, const string& pattern) {
    string lowerText = toLowerCase(text);
    string lowerPattern = toLowerCase(pattern);

    if (lowerPattern.length() == 0) {
        return true;
    }
    if (lowerPattern.length() > lowerText.length()) {
        return false;
    }

    for (size_t i = 0; i <= lowerText.length() - lowerPattern.length(); i++) {
        bool match = true;
        for (size_t j = 0; j < lowerPattern.length(); j++) {
            if (lowerText[i + j] != lowerPattern[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            return true;
        }
    }
    return false;
}

/*
  FUNCTION: readInt
  PURPOSE: Reads an integer within [minVal, maxVal], safely handling non-numeric bad input.
  TIME COMPLEXITY: O(1) per valid attempt.
  HOW TO EXPLAIN: "If the user types letters instead of numbers, cin.clear() resets the error state,
   and cin.ignore() flushes bad characters from the stream to prevent infinite loops."
*/
int readInt(const string& prompt, int minVal, int maxVal) {
    int value = 0;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            if (value >= minVal && value <= maxVal) {
                cin.ignore(10000, '\n'); // Clear remaining newline
                return value;
            } else {
                cout << "[Error] Input must be between " << minVal << " and " << maxVal << ". Try again.\n";
            }
        } else {
            cout << "[Error] Invalid input! Please enter a whole number.\n";
            cin.clear();            // Reset cin error state
            cin.ignore(10000, '\n'); // Discard invalid characters
        }
    }
}

/*
  FUNCTION: readFloat
  PURPOSE: Reads a float within [minVal, maxVal], handling invalid characters gracefully.
  TIME COMPLEXITY: O(1) per valid attempt.
*/
float readFloat(const string& prompt, float minVal, float maxVal) {
    float value = 0.0f;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            if (value >= minVal && value <= maxVal) {
                cin.ignore(10000, '\n');
                return value;
            } else {
                cout << "[Error] Value must be between " << minVal << " and " << maxVal << ". Try again.\n";
            }
        } else {
            cout << "[Error] Invalid input! Please enter a decimal number.\n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }
}

/*
  FUNCTION: readName
  PURPOSE: Reads a non-empty name string containing only alphabetical letters and spaces.
  TIME COMPLEXITY: O(N) where N is length of the string.
  HOW TO EXPLAIN: "Uses getline() to allow spaces, trims leading/trailing whitespace, and verifies isalpha()."
*/
string readName(const string& prompt) {
    string name = "";
    while (true) {
        cout << prompt;
        getline(cin, name);

        // Trim leading and trailing whitespace manually
        size_t start = 0;
        while (start < name.length() && isspace(name[start])) {
            start++;
        }
        size_t end = name.length();
        while (end > start && isspace(name[end - 1])) {
            end--;
        }

        if (start >= end) {
            cout << "[Error] Name cannot be blank. Try again.\n";
            continue;
        }

        string trimmed = name.substr(start, end - start);
        bool valid = true;
        for (size_t i = 0; i < trimmed.length(); i++) {
            if (!isalpha(trimmed[i]) && !isspace(trimmed[i])) {
                valid = false;
                break;
            }
        }

        if (!valid) {
            cout << "[Error] Name must contain letters and spaces only. Try again.\n";
            continue;
        }

        return trimmed;
    }
}

/*
  FUNCTION: readGender
  PURPOSE: Reads gender as 'M', 'F', or 'O'.
  TIME COMPLEXITY: O(1)
*/
char readGender(const string& prompt) {
    string input = "";
    while (true) {
        cout << prompt;
        getline(cin, input);
        if (input.length() == 1) {
            char g = (char)toupper(input[0]);
            if (g == 'M' || g == 'F' || g == 'O') {
                return g;
            }
        }
        cout << "[Error] Please enter 'M' (Male), 'F' (Female), or 'O' (Other).\n";
    }
}

/*
  FUNCTION: readDate
  PURPOSE: Interactively reads day, month, and year, verifying real calendar validity.
  TIME COMPLEXITY: O(1)
*/
Date readDate(const string& prompt) {
    Date d;
    cout << prompt << "\n";
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


// ========================================================================================================
// SECTION 3: SQLITE DATABASE LAYER (BACKEND PERSISTENCE)
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
        "  status TEXT NOT NULL"
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

// Updates available seat count for a train
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
    const char* sql = "INSERT INTO passengers (pnr, name, age, gender, train_no, seat_no, day, month, year, status) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
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
    const char* sql = "SELECT pnr, name, age, gender, train_no, seat_no, day, month, year, status "
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
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        w.waitId = (int)sqlite3_last_insert_rowid(db);
        return true;
    }
    return false;
}

// Deletes a waiting passenger record when they get auto-promoted or cancelled
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

// Loads waiting lists into memory ordered by wait_id to preserve exact FIFO order
bool loadWaiting(sqlite3* db, map<int, queue<WaitingEntry> >& lists) {
    lists.clear();
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

        // Enqueue into the queue corresponding to this train
        lists[w.trainNo].push(w);
    }
    sqlite3_finalize(stmt);
    return true;
}


// ========================================================================================================
// SECTION 4: CORE RAILWAY BUSINESS LOGIC & DSA ALGORITHMS (MODULES III, IV, VII, VIII, IX & X)
// ========================================================================================================

/*
  FUNCTION: seedSampleTrains
  PURPOSE: Pre-seeds 4 initial realistic trains with small seat counts (3 to 5 seats).
  WHY: Enables fast testing and instant demonstration of full-train edge cases and queues.
  TIME COMPLEXITY: O(1)
*/
void seedSampleTrains(RailwaySystem& sys, sqlite3* db) {
    if (!sys.trains.empty()) {
        return; // Already populated
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

/*
  FUNCTION: binarySearchTrain
  PURPOSE: Performs iterative Binary Search to find a train index by trainNo.
  TIME COMPLEXITY: O(log N)
  HOW TO EXPLAIN TO PROFESSOR:
  "Sir, because the trains vector is maintained in sorted order by trainNo,
   we divide the search space in half at each iteration, achieving O(log N) lookup."
*/
int binarySearchTrain(const vector<Train>& trains, int trainNo) {
    int low = 0;
    int high = (int)trains.size() - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (trains[mid].trainNo == trainNo) {
            return mid; // Found at index mid
        } else if (trains[mid].trainNo < trainNo) {
            low = mid + 1; // Search right half
        } else {
            high = mid - 1; // Search left half
        }
    }
    return -1; // Train not found
}

/*
  FUNCTION: findTrainIndex
  PURPOSE: Helper that wraps binarySearchTrain for RailwaySystem.
  TIME COMPLEXITY: O(log N)
*/
int findTrainIndex(const RailwaySystem& sys, int trainNo) {
    return binarySearchTrain(sys.trains, trainNo);
}

/*
  FUNCTION: loadSystem
  PURPOSE: Restores entire in-memory working state from SQLite upon application launch.
  KEY OPERATIONS:
    1. Zeroes out 2D seatMap.
    2. Loads trains, passengers, and waiting lists from database.
    3. Reconstructs seatMap by marking confirmed passenger seats as 1 (booked).
  TIME COMPLEXITY: O(T + P + W)
*/
void loadSystem(RailwaySystem& sys, sqlite3* db) {
    for (int i = 0; i < MAX_TRAINS; i++) {
        for (int j = 0; j < MAX_SEATS; j++) {
            sys.seatMap[i][j] = 0; // Initialize all seats as free
        }
    }

    loadTrains(db, sys.trains);
    seedSampleTrains(sys, db);
    loadPassengers(db, sys.passengers);
    loadWaiting(db, sys.waitingLists);

    // Reconstruct 2D seatMap from confirmed passengers
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

/*
  FUNCTION: addTrain
  PURPOSE: Adds a new train, inserting it into the vector IN SORTED ORDER by trainNo.
  TIME COMPLEXITY: O(N) due to vector shift during sorted insertion.
  HOW TO EXPLAIN TO PROFESSOR:
  "Sir, whenever a new train is added, we find its correct position and insert it
   so the vector remains sorted. This guarantees Binary Search always works!"
*/
void addTrain(RailwaySystem& sys, sqlite3* db) {
    if ((int)sys.trains.size() >= MAX_TRAINS) {
        cout << "\n[Error] Maximum capacity of " << MAX_TRAINS << " trains reached!\n";
        return;
    }

    cout << "\n--- Add New Train Record ---\n";
    int trainNo = readInt("Enter Train Number (1000 - 99999): ", 1000, 99999);

    // Check duplicate train number using Binary Search
    if (findTrainIndex(sys, trainNo) != -1) {
        cout << "[Error] Duplicate entry! Train number " << trainNo << " already exists.\n";
        return;
    }

    Train t;
    t.trainNo = trainNo;
    t.name = readName("Enter Train Name: ");
    t.source = readName("Enter Source Station: ");
    t.destination = readName("Enter Destination Station: ");

    cout << "Enter Departure Time (e.g. 09:30 AM): ";
    getline(cin, t.departure);
    if (t.departure.empty()) t.departure = "12:00 PM";

    t.totalSeats = readInt("Enter Total Seats (1 - " + to_string(MAX_SEATS) + "): ", 1, MAX_SEATS);
    t.availableSeats = t.totalSeats;
    t.fare = readFloat("Enter Ticket Fare in INR (10.0 - 10000.0): ", 10.0f, 10000.0f);

    // Find sorted insertion index
    int pos = 0;
    while (pos < (int)sys.trains.size() && sys.trains[pos].trainNo < t.trainNo) {
        pos++;
    }

    // Shift seatMap rows to maintain 1-to-1 index alignment
    for (int i = (int)sys.trains.size(); i > pos; i--) {
        for (int s = 0; s < MAX_SEATS; s++) {
            sys.seatMap[i][s] = sys.seatMap[i - 1][s];
        }
    }
    for (int s = 0; s < MAX_SEATS; s++) {
        sys.seatMap[pos][s] = 0; // Initialize new train's seats as free
    }

    sys.trains.insert(sys.trains.begin() + pos, t);

    if (insertTrain(db, t)) {
        cout << "\n[Success] Train #" << t.trainNo << " (" << t.name << ") added and saved to database successfully!\n";
    }
}

/*
  FUNCTION: displayTrains
  PURPOSE: Displays all trains in a cleanly aligned tabular format using iomanip.
  TIME COMPLEXITY: O(N)
*/
void displayTrains(const RailwaySystem& sys) {
    if (sys.trains.empty()) {
        cout << "\n[Notice] No trains found in the system.\n";
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

/*
  FUNCTION: searchTrainByNumber
  PURPOSE: Uses Binary Search to look up a train by its number in O(log N).
  TIME COMPLEXITY: O(log N)
*/
void searchTrainByNumber(const RailwaySystem& sys) {
    int trainNo = readInt("\nEnter Train Number to search: ", 1000, 99999);
    int idx = findTrainIndex(sys, trainNo);

    if (idx == -1) {
        cout << "[Notice] Train #" << trainNo << " was not found.\n";
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

/*
  FUNCTION: searchTrainByDestination
  PURPOSE: Uses Linear Search to locate trains by destination with case-insensitive matching.
  TIME COMPLEXITY: O(N * M)
  HOW TO EXPLAIN TO PROFESSOR:
  "Sir, multiple trains can share the same destination, and users can type partial names.
   Linear search checks every train to find all partial matches."
*/
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

/*
  FUNCTION: sortTrains
  PURPOSE: Sorts trains for display using manual Bubble Sort on a COPY of the vector.
  TIME COMPLEXITY: O(N^2)
  HOW TO EXPLAIN TO PROFESSOR:
  "Sir, we sort a COPY of the vector for display. If we sorted the main vector by fare,
   it would break Binary Search (which requires sorting by trainNo) and desynchronize
   our 2D seat map rows. Sorting a copy protects data integrity!"
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

    // Make a local copy so main vector remains sorted by trainNo
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
    cout << "\n[Displaying Sorted List of Trains]:\n";
    displayTrains(tempSys);
}

/*
  FUNCTION: findFreeSeat
  PURPOSE: Scans the 2D seatMap row to locate the first free seat (value 0).
  TIME COMPLEXITY: O(S) where S is totalSeats.
  HOW TO EXPLAIN: "Returns 1-based seat number, or -1 if all seats are booked."
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
  FUNCTION: generatePNR
  PURPOSE: Generates the next unique PNR (max PNR + 1 starting from 1001).
  TIME COMPLEXITY: O(P) where P is passenger count.
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
  PURPOSE: Books a ticket, allocating a seat via the 2D seatMap or queueing in FIFO waiting list.
  TIME COMPLEXITY: O(P + log N)
  HOW TO EXPLAIN TO PROFESSOR:
  "Sir, if seats > 0, we take the first free seat from our 2D array, generate a PNR,
   mark seatMap as 1, and save to SQLite. If seats == 0, we push the passenger into
   that train's FIFO queue and assign them a waiting list position."
*/
void bookTicket(RailwaySystem& sys, sqlite3* db) {
    cout << "\n--- Book a Ticket ---\n";
    int trainNo = readInt("Enter Train Number: ", 1000, 99999);
    int trainIdx = findTrainIndex(sys, trainNo);

    if (trainIdx == -1) {
        cout << "[Error] Train #" << trainNo << " does not exist.\n";
        return;
    }

    string name = readName("Enter Passenger Name: ");
    int age = readInt("Enter Age (1 - 120): ", 1, 120);
    char gender = readGender("Enter Gender (M/F/O): ");
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

    int freeSeat = findFreeSeat(sys, trainIdx);

    // CASE 1: Seats available -> Allocate seat
    if (freeSeat != -1 && sys.trains[trainIdx].availableSeats > 0) {
        int seatIdx = freeSeat - 1;
        sys.seatMap[trainIdx][seatIdx] = 1; // Mark seat as booked in 2D array
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
    // CASE 2: Train is full -> Queue into FIFO waiting list
    else {
        cout << "\n[Notice] Train #" << trainNo << " is fully booked! (Available Seats: 0)\n";
        
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

        insertWaiting(db, w);
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
  HOW TO EXPLAIN TO PROFESSOR:
  "Sir, when a seat is freed, we check if the train's queue is empty.
   If not empty, we call front() to inspect the first waiting passenger and pop()
   to remove them (FIFO order), allocating them the freed seat in O(1) time."
*/
bool promoteFromWaitingList(RailwaySystem& sys, sqlite3* db, int trainIndex, int seatNo) {
    int trainNo = sys.trains[trainIndex].trainNo;
    queue<WaitingEntry>& wQueue = sys.waitingLists[trainNo];

    if (!wQueue.empty()) {
        WaitingEntry topWait = wQueue.front(); // Peek FIFO head
        wQueue.pop();                          // Remove from queue

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

        sys.seatMap[trainIndex][seatNo - 1] = 1; // Mark re-booked
        sys.passengers.push_back(promoted);
        insertPassenger(db, promoted);

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

    // Push cancelled ticket onto LIFO stack (Module VIII)
    sys.recentCancellations.push(p);

    int trainIdx = findTrainIndex(sys, p.trainNo);
    int freedSeat = p.seatNo;
    sys.seatMap[trainIdx][freedSeat - 1] = 0; // Free seat in 2D array

    cout << "\n[Success] Ticket PNR " << pnr << " has been CANCELLED successfully.\n";

    // Auto-promote waiting passenger if one exists
    promoteFromWaitingList(sys, db, trainIdx, freedSeat);
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
  HOW TO EXPLAIN TO PROFESSOR:
  "Sir, this demonstrates Stack LIFO behavior. If the freed seat has not been
   allocated to a waiting passenger, we can pop the stack and restore the booking."
*/
void undoLastCancellation(RailwaySystem& sys, sqlite3* db) {
    if (sys.recentCancellations.empty()) {
        cout << "\n[Notice] No cancellations available to undo.\n";
        return;
    }

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
  HOW TO EXPLAIN TO PROFESSOR:
  "Sir, this visualizes our 2D array seatMap. '[XX]' indicates a booked seat,
   while '[ 1]' shows an available seat number."
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

/*
  FUNCTION: displayWaitingList
  PURPOSE: Displays waiting queues for all trains non-destructively.
  TIME COMPLEXITY: O(T * W)
  HOW TO EXPLAIN TO PROFESSOR:
  "Sir, since standard queue does not support random access or index traversal,
   we create a local copy of the queue and call pop() on the copy.
   The real queue in memory remains completely untouched!"
*/
void displayWaitingList(const RailwaySystem& sys) {
    if (sys.waitingLists.empty()) {
        cout << "\n[Notice] No active waiting lists.\n";
        return;
    }

    bool hasAnyWaiting = false;
    map<int, queue<WaitingEntry> >::const_iterator it;

    for (it = sys.waitingLists.begin(); it != sys.waitingLists.end(); ++it) {
        int trainNo = it->first;
        // Non-destructive copy of the train's queue
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


// ========================================================================================================
// SECTION 5: MAIN MENU & SYSTEM ENTRY POINT (MODULE II)
// ========================================================================================================

/*
  FUNCTION: displayMenu
  PURPOSE: Prints the interactive terminal menu options clearly.
*/
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

/*
  FUNCTION: main
  PURPOSE: Application entry point.
  DESIGN RULE: Does NOT contain business logic. Only initializes database, loads working memory,
               runs the do-while menu loop, and safely closes the database connection.
*/
int main() {
    cout << "\n>>> Starting Railway Ticket Reservation System <<<\n";

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

    // Step 4: Interactive menu loop
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

    // Step 5: Close SQLite database connection cleanly
    closeDatabase(db);
    return 0;
}
