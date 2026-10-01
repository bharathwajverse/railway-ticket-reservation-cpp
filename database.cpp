#include "database.h"
#include <iostream>

using namespace std;

bool dbOpen(sqlite3*& db, const char* fileName) {
    if (sqlite3_open(fileName, &db) != SQLITE_OK) return false;
    sqlite3_exec(db, "PRAGMA foreign_keys = ON; PRAGMA synchronous = NORMAL;", NULL, NULL, NULL);
    return true;
}

void dbClose(sqlite3* db) {
    if (db) sqlite3_close(db);
}

bool dbCreateTables(sqlite3* db) {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS trains ("
        "  train_no INTEGER PRIMARY KEY, name TEXT NOT NULL, source TEXT NOT NULL,"
        "  destination TEXT NOT NULL, departure TEXT NOT NULL, total_seats INTEGER NOT NULL,"
        "  available_seats INTEGER NOT NULL, fare REAL NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS passengers ("
        "  pnr INTEGER PRIMARY KEY, name TEXT NOT NULL, age INTEGER NOT NULL,"
        "  gender TEXT NOT NULL, train_no INTEGER NOT NULL, seat_no INTEGER NOT NULL,"
        "  day INTEGER NOT NULL, month INTEGER NOT NULL, year INTEGER NOT NULL,"
        "  status TEXT NOT NULL, concession TEXT DEFAULT 'GENERAL', fare_paid REAL DEFAULT 0.0"
        ");"
        "CREATE TABLE IF NOT EXISTS waiting_list ("
        "  wait_id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, age INTEGER NOT NULL,"
        "  gender TEXT NOT NULL, train_no INTEGER NOT NULL, day INTEGER NOT NULL,"
        "  month INTEGER NOT NULL, year INTEGER NOT NULL"
        ");";

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) return false;
    sqlite3_exec(db, "ALTER TABLE passengers ADD COLUMN concession TEXT DEFAULT 'GENERAL';", NULL, NULL, NULL);
    sqlite3_exec(db, "ALTER TABLE passengers ADD COLUMN fare_paid REAL DEFAULT 0.0;", NULL, NULL, NULL);
    return true;
}

bool dbInsertTrain(sqlite3* db, const Train& t) {
    const char* sql = "INSERT INTO trains VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, t.trainNo);
    sqlite3_bind_text(stmt, 2, t.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, t.source.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, t.destination.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, t.departure.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, t.totalSeats);
    sqlite3_bind_int(stmt, 7, t.availableSeats);
    sqlite3_bind_double(stmt, 8, t.fare);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool dbUpdateTrainSeats(sqlite3* db, int trainNo, int availableSeats) {
    const char* sql = "UPDATE trains SET available_seats = ? WHERE train_no = ?;";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, availableSeats);
    sqlite3_bind_int(stmt, 2, trainNo);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool dbLoadTrains(sqlite3* db, vector<Train>& trains) {
    trains.clear();
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, "SELECT * FROM trains ORDER BY train_no ASC;", -1, &stmt, NULL) != SQLITE_OK) {
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

bool dbInsertPassenger(sqlite3* db, const Passenger& p) {
    const char* sql = "INSERT INTO passengers VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, p.pnr);
    sqlite3_bind_text(stmt, 2, p.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, p.age);
    string gStr(1, p.gender);
    sqlite3_bind_text(stmt, 4, gStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, p.trainNo);
    sqlite3_bind_int(stmt, 6, p.seatNo);
    sqlite3_bind_int(stmt, 7, p.travelDate.day);
    sqlite3_bind_int(stmt, 8, p.travelDate.month);
    sqlite3_bind_int(stmt, 9, p.travelDate.year);
    sqlite3_bind_text(stmt, 10, p.status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 11, p.concession.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 12, p.farePaid);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool dbUpdatePassengerStatus(sqlite3* db, int pnr, const string& status) {
    const char* sql = "UPDATE passengers SET status = ? WHERE pnr = ?;";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, pnr);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool dbLoadPassengers(sqlite3* db, vector<Passenger>& passengers) {
    passengers.clear();
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, "SELECT * FROM passengers ORDER BY pnr ASC;", -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Passenger p;
        p.pnr = sqlite3_column_int(stmt, 0);
        p.name = (const char*)sqlite3_column_text(stmt, 1);
        p.age = sqlite3_column_int(stmt, 2);
        const char* gText = (const char*)sqlite3_column_text(stmt, 3);
        p.gender = (gText && gText[0]) ? gText[0] : 'M';
        p.trainNo = sqlite3_column_int(stmt, 4);
        p.seatNo = sqlite3_column_int(stmt, 5);
        p.travelDate.day = sqlite3_column_int(stmt, 6);
        p.travelDate.month = sqlite3_column_int(stmt, 7);
        p.travelDate.year = sqlite3_column_int(stmt, 8);
        p.status = (const char*)sqlite3_column_text(stmt, 9);
        const char* cText = (const char*)sqlite3_column_text(stmt, 10);
        p.concession = cText ? cText : "GENERAL";
        p.farePaid = (float)sqlite3_column_double(stmt, 11);
        passengers.push_back(p);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool dbInsertWaiting(sqlite3* db, WaitingEntry& w) {
    const char* sql = "INSERT INTO waiting_list (name, age, gender, train_no, day, month, year) VALUES (?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, w.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, w.age);
    string gStr(1, w.gender);
    sqlite3_bind_text(stmt, 3, gStr.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, w.trainNo);
    sqlite3_bind_int(stmt, 5, w.travelDate.day);
    sqlite3_bind_int(stmt, 6, w.travelDate.month);
    sqlite3_bind_int(stmt, 7, w.travelDate.year);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    if (ok) w.waitId = (int)sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);
    return ok;
}

bool dbDeleteWaiting(sqlite3* db, int waitId) {
    const char* sql = "DELETE FROM waiting_list WHERE wait_id = ?;";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, waitId);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool dbLoadWaiting(sqlite3* db, vector<WaitingEntry>& waitingList) {
    waitingList.clear();
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, "SELECT * FROM waiting_list ORDER BY wait_id ASC;", -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        WaitingEntry w;
        w.waitId = sqlite3_column_int(stmt, 0);
        w.name = (const char*)sqlite3_column_text(stmt, 1);
        w.age = sqlite3_column_int(stmt, 2);
        const char* gText = (const char*)sqlite3_column_text(stmt, 3);
        w.gender = (gText && gText[0]) ? gText[0] : 'M';
        w.trainNo = sqlite3_column_int(stmt, 4);
        w.travelDate.day = sqlite3_column_int(stmt, 5);
        w.travelDate.month = sqlite3_column_int(stmt, 6);
        w.travelDate.year = sqlite3_column_int(stmt, 7);
        waitingList.push_back(w);
    }
    sqlite3_finalize(stmt);
    return true;
}
