#include "database.h"
#include <iostream>

using namespace std;

/*
  ==============================================================================
  SQLITE 3 API CONCEPTS EXPLAINED FOR BEGINNERS:
  ------------------------------------------------------------------------------
  1. sqlite3_open(fileName, &db):
     Opens the database file. If the file does not exist, SQLite creates it.
     'db' is our handle/pointer to the open database connection.

  2. sqlite3_prepare_v2(db, sql, -1, &stmt, NULL):
     Compiles an SQL string into bytecode (prepared statement). '?' are placeholders
     for parameters that will be bound safely later.

  3. sqlite3_bind_int / text / double(stmt, index, value, ...):
     Fills in the '?' placeholders with real data safely. This protects against
     SQL injection attacks (e.g. funny names breaking the SQL command).

  4. sqlite3_step(stmt):
     Executes the compiled statement. For INSERT/UPDATE/DELETE, it returns
     SQLITE_DONE. For SELECT queries, each call returns SQLITE_ROW until all rows
     are fetched.

  5. sqlite3_column_int / text / double(stmt, colIndex):
     Retrieves the value of a specific column from the current row.

  6. sqlite3_finalize(stmt):
     Frees memory associated with the prepared statement. ALWAYS called to prevent
     memory leaks.

  7. sqlite3_close(db):
     Closes the database file and frees connection resources.
  ==============================================================================
*/

// Opens database file and sets handle
// Time Complexity: O(1)
bool openDatabase(sqlite3*& db, const char* fileName) {
    int rc = sqlite3_open(fileName, &db);
    if (rc != SQLITE_OK) {
        cout << "[Database Error] Cannot open database: " << sqlite3_errmsg(db) << "\n";
        return false;
    }
    return true;
}

// Closes open database connection
// Time Complexity: O(1)
void closeDatabase(sqlite3* db) {
    if (db != NULL) {
        sqlite3_close(db);
    }
}

// Creates the 3 essential relational tables if they do not already exist
// Time Complexity: O(1)
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
        cout << "[Database Error] Failed to create tables: " << errMsg << "\n";
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

// Inserts a new train record
// Time Complexity: O(1)
bool insertTrain(sqlite3* db, const Train& t) {
    const char* sql = "INSERT INTO trains (train_no, name, source, destination, departure, total_seats, available_seats, fare) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        cout << "[Database Error] Prepare insertTrain failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

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

    if (rc != SQLITE_DONE) {
        cout << "[Database Error] Step insertTrain failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }
    return true;
}

// Updates available seat count for a train
// Time Complexity: O(1)
bool updateTrainSeats(sqlite3* db, int trainNo, int availableSeats) {
    const char* sql = "UPDATE trains SET available_seats = ? WHERE train_no = ?;";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        cout << "[Database Error] Prepare updateTrainSeats failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_int(stmt, 1, availableSeats);
    sqlite3_bind_int(stmt, 2, trainNo);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        cout << "[Database Error] Step updateTrainSeats failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }
    return true;
}

// Loads all trains sorted by train_no
// Time Complexity: O(N) where N is number of trains
bool loadTrains(sqlite3* db, vector<Train>& trains) {
    trains.clear();
    const char* sql = "SELECT train_no, name, source, destination, departure, total_seats, available_seats, fare "
                      "FROM trains ORDER BY train_no ASC;";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        cout << "[Database Error] Prepare loadTrains failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

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

// Inserts a new passenger booking record
// Time Complexity: O(1)
bool insertPassenger(sqlite3* db, const Passenger& p) {
    const char* sql = "INSERT INTO passengers (pnr, name, age, gender, train_no, seat_no, day, month, year, status) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        cout << "[Database Error] Prepare insertPassenger failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    char genderStr[2] = { p.gender, '\0' };

    sqlite3_bind_int(stmt, 1, p.pnr);
    sqlite3_bind_text(stmt, 2, p.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, p.age);
    sqlite3_bind_text(stmt, 4, genderStr, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, p.trainNo);
    sqlite3_bind_int(stmt, 6, p.seatNo);
    sqlite3_bind_int(stmt, 7, p.travelDate.day);
    sqlite3_bind_int(stmt, 8, p.travelDate.month);
    sqlite3_bind_int(stmt, 9, p.travelDate.year);
    sqlite3_bind_text(stmt, 10, p.status.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        cout << "[Database Error] Step insertPassenger failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }
    return true;
}

// Updates booking status of a passenger (e.g. CONFIRMED -> CANCELLED)
// Time Complexity: O(1)
bool updatePassengerStatus(sqlite3* db, int pnr, const string& status) {
    const char* sql = "UPDATE passengers SET status = ? WHERE pnr = ?;";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        cout << "[Database Error] Prepare updatePassengerStatus failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, pnr);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        cout << "[Database Error] Step updatePassengerStatus failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }
    return true;
}

// Loads all passengers from database
// Time Complexity: O(P) where P is number of passengers
bool loadPassengers(sqlite3* db, vector<Passenger>& list) {
    list.clear();
    const char* sql = "SELECT pnr, name, age, gender, train_no, seat_no, day, month, year, status "
                      "FROM passengers ORDER BY pnr ASC;";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        cout << "[Database Error] Prepare loadPassengers failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

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

// Inserts a new waiting list entry and updates w.waitId with autoincremented ID
// Time Complexity: O(1)
bool insertWaiting(sqlite3* db, WaitingEntry& w) {
    const char* sql = "INSERT INTO waiting_list (name, age, gender, train_no, day, month, year) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        cout << "[Database Error] Prepare insertWaiting failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    char genderStr[2] = { w.gender, '\0' };

    sqlite3_bind_text(stmt, 1, w.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, w.age);
    sqlite3_bind_text(stmt, 3, genderStr, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, w.trainNo);
    sqlite3_bind_int(stmt, 5, w.travelDate.day);
    sqlite3_bind_int(stmt, 6, w.travelDate.month);
    sqlite3_bind_int(stmt, 7, w.travelDate.year);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        cout << "[Database Error] Step insertWaiting failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    w.waitId = (int)sqlite3_last_insert_rowid(db);
    return true;
}

// Deletes a waiting list entry by wait_id (used when promoted or cancelled)
// Time Complexity: O(1)
bool deleteWaiting(sqlite3* db, int waitId) {
    const char* sql = "DELETE FROM waiting_list WHERE wait_id = ?;";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        cout << "[Database Error] Prepare deleteWaiting failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    sqlite3_bind_int(stmt, 1, waitId);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        cout << "[Database Error] Step deleteWaiting failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }
    return true;
}

// Loads waiting lists into memory map of queues ordered by wait_id (preserving FIFO)
// Time Complexity: O(W) where W is number of waiting entries
bool loadWaiting(sqlite3* db, map<int, queue<WaitingEntry> >& lists) {
    lists.clear();
    const char* sql = "SELECT wait_id, name, age, gender, train_no, day, month, year "
                      "FROM waiting_list ORDER BY wait_id ASC;";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        cout << "[Database Error] Prepare loadWaiting failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

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

        // Push into the queue corresponding to this train
        lists[w.trainNo].push(w);
    }
    sqlite3_finalize(stmt);
    return true;
}
