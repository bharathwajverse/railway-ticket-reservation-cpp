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
#include "sqlite3.h"

using namespace std;

const int MAX_TRAINS = 20;
const int MAX_SEATS = 60;
const int MAX_WAITING = 10;

struct Date {
    int day;
    int month;
    int year;
};

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

struct Passenger {
    int pnr;
    string name;
    int age;
    char gender;
    int trainNo;
    int seatNo;
    Date travelDate;
    string status;
    string concession;
    float farePaid;
};

struct WaitingEntry {
    int waitId;
    string name;
    int age;
    char gender;
    int trainNo;
    Date travelDate;
};

struct RailwaySystem {
    vector<Train> trains;
    vector<Passenger> passengers;
    map<int, queue<WaitingEntry>> waitingLists;
    int seatMap[MAX_TRAINS][MAX_SEATS];
    stack<Passenger> recentCancellations;
};

bool isLeapYear(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

bool isValidDate(int d, int m, int y) {
    if (y < 2024 || y > 2035 || m < 1 || m > 12) return false;
    int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (m == 2 && isLeapYear(y)) daysInMonth[1] = 29;
    return d >= 1 && d <= daysInMonth[m - 1];
}

string toLowerCase(string s) {
    for (size_t i = 0; i < s.length(); i++) s[i] = (char)tolower(s[i]);
    return s;
}

bool containsIgnoreCase(const string& text, const string& pattern) {
    return toLowerCase(text).find(toLowerCase(pattern)) != string::npos;
}

int readInt(const string& prompt, int minVal, int maxVal) {
    int val;
    while (true) {
        cout << prompt;
        if (cin >> val) {
            string dummy;
            getline(cin, dummy);
            if (val >= minVal && val <= maxVal) return val;
            cout << "Input out of range (" << minVal << " - " << maxVal << ").\n";
        } else {
            cin.clear();
            string bad;
            cin >> bad;
            cout << "Invalid input. Please enter a number.\n";
        }
    }
}

string readNonEmptyString(const string& prompt) {
    string val;
    while (true) {
        cout << prompt;
        getline(cin, val);
        size_t start = val.find_first_not_of(" \t\r\n");
        size_t end = val.find_last_not_of(" \t\r\n");
        if (start != string::npos && end != string::npos) return val.substr(start, end - start + 1);
        cout << "Input cannot be empty.\n";
    }
}

Date readDate(const string& prompt) {
    cout << prompt << "\n";
    Date d;
    while (true) {
        d.day = readInt("  Day (1-31): ", 1, 31);
        d.month = readInt("  Month (1-12): ", 1, 12);
        d.year = readInt("  Year (2024-2035): ", 2024, 2035);
        if (isValidDate(d.day, d.month, d.year)) return d;
        cout << "Invalid date. Try again.\n";
    }
}

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

bool exportTicketToFile(const Passenger& p, const Train& t) {
    string fileName = "ticket_" + to_string(p.pnr) + ".txt";
    ofstream fout(fileName.c_str());
    if (!fout.is_open()) return false;

    float discount = t.fare - p.farePaid;
    if (discount < 0.0f) discount = 0.0f;
    string berth = (p.seatNo % 4 == 1) ? "Window (Lower)" : (p.seatNo % 4 == 2) ? "Aisle" : (p.seatNo % 4 == 3) ? "Middle" : "Window (Upper)";

    fout << "======================================================================\n";
    fout << "                 INDIAN RAILWAY PASSENGER RESERVATION                 \n";
    fout << "                      ELECTRONIC RESERVATION SLIP                     \n";
    fout << "======================================================================\n";
    fout << "PNR NUMBER        : " << p.pnr << "\n";
    fout << "BOOKING STATUS    : " << p.status << "\n";
    fout << "PASSENGER NAME    : " << p.name << "\n";
    fout << "AGE / GENDER      : " << p.age << " yrs / " << p.gender << "\n";
    fout << "CONCESSION TIER   : " << p.concession << "\n";
    fout << "----------------------------------------------------------------------\n";
    fout << "TRAIN NUMBER      : " << t.trainNo << "\n";
    fout << "TRAIN NAME        : " << t.name << "\n";
    fout << "ROUTE             : " << t.source << " -> " << t.destination << "\n";
    fout << "DEPARTURE         : " << t.departure << "\n";
    fout << "TRAVEL DATE       : " << setfill('0') << setw(2) << p.travelDate.day << "/"
                                  << setfill('0') << setw(2) << p.travelDate.month << "/"
                                  << p.travelDate.year << setfill(' ') << "\n";
    fout << "ALLOCATED SEAT    : Coach C1, Seat #" << p.seatNo << " (" << berth << ")\n";
    fout << "----------------------------------------------------------------------\n";
    fout << "BASE FARE         : Rs. " << fixed << setprecision(2) << t.fare << "\n";
    fout << "DISCOUNT          : Rs. " << fixed << setprecision(2) << discount << "\n";
    fout << "TOTAL FARE PAID   : Rs. " << fixed << setprecision(2) << p.farePaid << "\n";
    fout << "======================================================================\n";
    fout.close();
    return true;
}

bool openDatabase(sqlite3*& db, const char* fileName) {
    if (sqlite3_open(fileName, &db) != SQLITE_OK) return false;
    sqlite3_exec(db, "PRAGMA foreign_keys = ON; PRAGMA synchronous = NORMAL;", NULL, NULL, NULL);
    return true;
}

void closeDatabase(sqlite3* db) {
    if (db) sqlite3_close(db);
}

bool createTables(sqlite3* db) {
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

bool insertTrain(sqlite3* db, const Train& t) {
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

bool updateTrainSeats(sqlite3* db, int trainNo, int availableSeats) {
    const char* sql = "UPDATE trains SET available_seats = ? WHERE train_no = ?;";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, availableSeats);
    sqlite3_bind_int(stmt, 2, trainNo);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool insertPassenger(sqlite3* db, const Passenger& p) {
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

bool updatePassengerStatus(sqlite3* db, int pnr, const string& status) {
    const char* sql = "UPDATE passengers SET status = ? WHERE pnr = ?;";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, pnr);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

bool insertWaiting(sqlite3* db, WaitingEntry& w) {
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

bool deleteWaiting(sqlite3* db, int waitId) {
    const char* sql = "DELETE FROM waiting_list WHERE wait_id = ?;";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, waitId);
    bool ok = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return ok;
}

int binarySearchTrain(const vector<Train>& trains, int trainNo) {
    int low = 0, high = (int)trains.size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (trains[mid].trainNo == trainNo) return mid;
        if (trains[mid].trainNo < trainNo) low = mid + 1;
        else high = mid - 1;
    }
    return -1;
}

int findTrainIndex(const RailwaySystem& sys, int trainNo) {
    return binarySearchTrain(sys.trains, trainNo);
}

void insertTrainSorted(vector<Train>& trains, const Train& t) {
    size_t i = 0;
    while (i < trains.size() && trains[i].trainNo < t.trainNo) i++;
    trains.insert(trains.begin() + i, t);
}

int findPassengerByPNR(const RailwaySystem& sys, int pnr) {
    for (size_t i = 0; i < sys.passengers.size(); i++) {
        if (sys.passengers[i].pnr == pnr) return (int)i;
    }
    return -1;
}

int generatePNR(const RailwaySystem& sys) {
    int maxPnr = 1000;
    for (size_t i = 0; i < sys.passengers.size(); i++) {
        if (sys.passengers[i].pnr > maxPnr) maxPnr = sys.passengers[i].pnr;
    }
    return maxPnr + 1;
}

void buildSeatMap(RailwaySystem& sys) {
    for (int i = 0; i < MAX_TRAINS; i++) {
        for (int j = 0; j < MAX_SEATS; j++) sys.seatMap[i][j] = 0;
    }
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

void displaySeatMap(const RailwaySystem& sys, int trainIndex) {
    const Train& t = sys.trains[trainIndex];
    cout << "\nSeat Map for Train #" << t.trainNo << " (" << t.name << "):\n";
    cout << "[.] = Free  [X] = Booked\n\n";
    for (int s = 1; s <= t.totalSeats; s++) {
        int occupied = sys.seatMap[trainIndex][s - 1];
        cout << "[" << (occupied ? "X" : ".") << " " << setw(2) << s << "] ";
        if (s % 4 == 2) cout << "   ";
        if (s % 4 == 0) cout << "\n";
    }
    if (t.totalSeats % 4 != 0) cout << "\n";
    cout << "Total Seats: " << t.totalSeats << " | Available: " << t.availableSeats << "\n";
}

int findFreeSeat(const RailwaySystem& sys, int trainIndex) {
    int total = sys.trains[trainIndex].totalSeats;
    for (int s = 0; s < total; s++) {
        if (sys.seatMap[trainIndex][s] == 0) return s + 1;
    }
    return -1;
}

bool loadSystem(RailwaySystem& sys, sqlite3* db) {
    sys.trains.clear();
    sys.passengers.clear();
    sys.waitingLists.clear();

    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, "SELECT * FROM trains ORDER BY train_no ASC;", -1, &stmt, NULL) == SQLITE_OK) {
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
            sys.trains.push_back(t);
        }
        sqlite3_finalize(stmt);
    }

    if (sqlite3_prepare_v2(db, "SELECT * FROM passengers ORDER BY pnr ASC;", -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Passenger p;
            p.pnr = sqlite3_column_int(stmt, 0);
            p.name = (const char*)sqlite3_column_text(stmt, 1);
            p.age = sqlite3_column_int(stmt, 2);
            const char* g = (const char*)sqlite3_column_text(stmt, 3);
            p.gender = (g && g[0]) ? g[0] : 'M';
            p.trainNo = sqlite3_column_int(stmt, 4);
            p.seatNo = sqlite3_column_int(stmt, 5);
            p.travelDate.day = sqlite3_column_int(stmt, 6);
            p.travelDate.month = sqlite3_column_int(stmt, 7);
            p.travelDate.year = sqlite3_column_int(stmt, 8);
            p.status = (const char*)sqlite3_column_text(stmt, 9);
            const char* conc = (const char*)sqlite3_column_text(stmt, 10);
            p.concession = conc ? conc : "GENERAL";
            p.farePaid = (float)sqlite3_column_double(stmt, 11);
            sys.passengers.push_back(p);
        }
        sqlite3_finalize(stmt);
    }

    if (sqlite3_prepare_v2(db, "SELECT * FROM waiting_list ORDER BY wait_id ASC;", -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            WaitingEntry w;
            w.waitId = sqlite3_column_int(stmt, 0);
            w.name = (const char*)sqlite3_column_text(stmt, 1);
            w.age = sqlite3_column_int(stmt, 2);
            const char* g = (const char*)sqlite3_column_text(stmt, 3);
            w.gender = (g && g[0]) ? g[0] : 'M';
            w.trainNo = sqlite3_column_int(stmt, 4);
            w.travelDate.day = sqlite3_column_int(stmt, 5);
            w.travelDate.month = sqlite3_column_int(stmt, 6);
            w.travelDate.year = sqlite3_column_int(stmt, 7);
            sys.waitingLists[w.trainNo].push(w);
        }
        sqlite3_finalize(stmt);
    }

    buildSeatMap(sys);
    return true;
}

void displayTrains(const RailwaySystem& sys) {
    if (sys.trains.empty()) {
        cout << "\nNo trains available.\n";
        return;
    }
    cout << "\n" << setw(8) << "Train#" << " | " << setw(18) << left << "Name" << " | "
         << setw(12) << "Source" << " | " << setw(12) << "Destination" << " | "
         << setw(8) << "Depart" << " | " << setw(6) << "Avail" << " | " << setw(8) << "Fare" << right << "\n";
    cout << "----------------------------------------------------------------------------------\n";
    for (size_t i = 0; i < sys.trains.size(); i++) {
        const Train& t = sys.trains[i];
        cout << setw(8) << t.trainNo << " | " << setw(18) << left << t.name << " | "
             << setw(12) << t.source << " | " << setw(12) << t.destination << " | "
             << setw(8) << t.departure << " | " << setw(6) << t.availableSeats << " | "
             << setw(8) << fixed << setprecision(2) << t.fare << right << "\n";
    }
}

void searchTrain(const RailwaySystem& sys) {
    int trainNo = readInt("\nEnter Train Number: ", 1000, 99999);
    int idx = findTrainIndex(sys, trainNo);
    if (idx == -1) {
        cout << "Train #" << trainNo << " not found.\n";
        return;
    }
    const Train& t = sys.trains[idx];
    cout << "\nTrain #" << t.trainNo << ": " << t.name << "\n"
         << "Route: " << t.source << " -> " << t.destination << " | Time: " << t.departure << "\n"
         << "Available Seats: " << t.availableSeats << " / " << t.totalSeats << " | Fare: Rs. " << fixed << setprecision(2) << t.fare << "\n";
}

void searchTrainByDestination(const RailwaySystem& sys) {
    string dest = readNonEmptyString("\nEnter Destination city: ");
    bool found = false;
    for (size_t i = 0; i < sys.trains.size(); i++) {
        if (containsIgnoreCase(sys.trains[i].destination, dest)) {
            if (!found) {
                cout << "\nMatching Trains for \"" << dest << "\":\n";
                found = true;
            }
            const Train& t = sys.trains[i];
            cout << "  #" << t.trainNo << " " << t.name << " (" << t.source << " -> " << t.destination
                 << ") | Dept: " << t.departure << " | Avail: " << t.availableSeats << " | Rs. " << t.fare << "\n";
        }
    }
    if (!found) cout << "No trains found for destination \"" << dest << "\".\n";
}

void sortTrains(RailwaySystem& sys) {
    if (sys.trains.empty()) {
        cout << "\nNo trains to sort.\n";
        return;
    }
    cout << "\n1. Sort by Fare (Lowest to Highest)\n2. Sort by Train Name (A-Z)\n";
    int choice = readInt("Choice (1 or 2): ", 1, 2);
    vector<Train> copyList = sys.trains;
    int n = (int)copyList.size();
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            bool swapNeeded = (choice == 1) ? (copyList[j].fare > copyList[j + 1].fare)
                                            : (toLowerCase(copyList[j].name) > toLowerCase(copyList[j + 1].name));
            if (swapNeeded) {
                Train temp = copyList[j];
                copyList[j] = copyList[j + 1];
                copyList[j + 1] = temp;
            }
        }
    }
    cout << "\nSorted Trains:\n";
    for (size_t i = 0; i < copyList.size(); i++) {
        const Train& t = copyList[i];
        cout << "  #" << t.trainNo << " " << setw(18) << left << t.name << " | "
             << t.source << " -> " << t.destination << " | Rs. " << fixed << setprecision(2) << t.fare << right << "\n";
    }
}

bool promoteFromWaitingList(RailwaySystem& sys, sqlite3* db, int trainIndex, int seatNo) {
    int trainNo = sys.trains[trainIndex].trainNo;
    queue<WaitingEntry>& wQueue = sys.waitingLists[trainNo];
    if (wQueue.empty()) return false;

    WaitingEntry topWait = wQueue.front();
    wQueue.pop();
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

    sys.seatMap[trainIndex][seatNo - 1] = 1;
    insertPassenger(db, promoted);

    sys.passengers.push_back(promoted);
    exportTicketToFile(promoted, sys.trains[trainIndex]);
    cout << "\n[Waiting List Promotion] Passenger \"" << promoted.name << "\" allocated Seat #" << seatNo << " (PNR: " << promoted.pnr << ").\n";
    return true;
}

void bookTicket(RailwaySystem& sys, sqlite3* db) {
    cout << "\n--- Book Ticket ---\n";
    int trainNo = readInt("Enter Train Number: ", 1000, 99999);
    int trainIdx = findTrainIndex(sys, trainNo);
    if (trainIdx == -1) {
        cout << "Train #" << trainNo << " does not exist.\n";
        return;
    }

    Train& train = sys.trains[trainIdx];
    string name = readNonEmptyString("Passenger Name: ");
    int age = readInt("Age: ", 1, 120);
    string gStr = readNonEmptyString("Gender (M/F/O): ");
    char gender = toupper(gStr[0]);
    Date travelDate = readDate("Travel Date:");

    string concessionTier;
    float finalFare = 0.0f;
    calculateConcession(age, train.fare, concessionTier, finalFare);

    cout << "\nFare: Rs. " << train.fare << " | Concession: " << concessionTier
         << " | Payable: Rs. " << fixed << setprecision(2) << finalFare << "\n";

    int freeSeat = findFreeSeat(sys, trainIdx);

    if (freeSeat != -1 && train.availableSeats > 0) {
        displaySeatMap(sys, trainIdx);
        cout << "1. Choose specific seat\n2. Auto-assign Seat #" << freeSeat << "\n";
        int seatChoice = readInt("Option (1 or 2): ", 1, 2);
        int allocatedSeat = freeSeat;

        if (seatChoice == 1) {
            int desiredSeat = readInt("Desired Seat Number: ", 1, train.totalSeats);
            if (sys.seatMap[trainIdx][desiredSeat - 1] == 0) allocatedSeat = desiredSeat;
            else cout << "Seat #" << desiredSeat << " is booked. Using Seat #" << freeSeat << " instead.\n";
        }

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

        sys.seatMap[trainIdx][allocatedSeat - 1] = 1;
        train.availableSeats--;
        updateTrainSeats(db, trainNo, train.availableSeats);
        insertPassenger(db, p);

        sys.passengers.push_back(p);
        exportTicketToFile(p, train);

        cout << "\n[Confirmed] PNR: " << p.pnr << " | Seat: #" << p.seatNo << " | E-Ticket generated.\n";
    } else {
        queue<WaitingEntry>& wQueue = sys.waitingLists[trainNo];
        if ((int)wQueue.size() >= MAX_WAITING) {
            cout << "Train and Waiting List are both FULL.\n";
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
        cout << "\n[Waitlisted] Added to Waiting List: WL-" << wQueue.size() << "\n";
    }
}

void cancelTicket(RailwaySystem& sys, sqlite3* db) {
    cout << "\n--- Cancel Ticket ---\n";
    int pnr = readInt("Enter PNR number: ", 1000, 999999);

    int pIdx = findPassengerByPNR(sys, pnr);
    if (pIdx == -1) {
        cout << "No booking record found for PNR " << pnr << ".\n";
        return;
    }

    Passenger& p = sys.passengers[pIdx];
    if (p.status == "CANCELLED") {
        cout << "Ticket PNR " << pnr << " is already cancelled.\n";
        return;
    }

    int trainIdx = findTrainIndex(sys, p.trainNo);
    int freedSeat = p.seatNo;

    p.status = "CANCELLED";
    updatePassengerStatus(db, pnr, "CANCELLED");

    if (trainIdx != -1) {
        if (freedSeat >= 1 && freedSeat <= sys.trains[trainIdx].totalSeats) {
            sys.seatMap[trainIdx][freedSeat - 1] = 0;
        }
        sys.trains[trainIdx].availableSeats++;
        updateTrainSeats(db, p.trainNo, sys.trains[trainIdx].availableSeats);
    }

    sys.recentCancellations.push(p);
    cout << "\n[Cancelled] Ticket PNR " << pnr << " has been cancelled.\n";

    if (trainIdx != -1) promoteFromWaitingList(sys, db, trainIdx, freedSeat);
}

void cancelWaitingListEntry(RailwaySystem& sys, sqlite3* db) {
    cout << "\n--- Cancel Waiting List Entry ---\n";
    int trainNo = readInt("Enter Train Number: ", 1000, 99999);
    string passengerName = readNonEmptyString("Passenger Name: ");

    if (sys.waitingLists.find(trainNo) == sys.waitingLists.end() || sys.waitingLists[trainNo].empty()) {
        cout << "No waiting passengers for Train #" << trainNo << ".\n";
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
            cancelledItem = top;
        } else {
            tempQueue.push(top);
        }
    }
    wQueue = tempQueue;

    if (found) {
        deleteWaiting(db, cancelledItem.waitId);
        cout << "\n[Cancelled] Waiting entry for \"" << passengerName << "\" has been removed.\n";
    } else {
        cout << "No matching passenger found with name \"" << passengerName << "\".\n";
    }
}

void undoLastCancellation(RailwaySystem& sys, sqlite3* db) {
    cout << "\n--- Undo Last Cancellation ---\n";

    if (sys.recentCancellations.empty()) {
        cout << "No recent cancellations to undo.\n";
        return;
    }

    Passenger lastCancelled = sys.recentCancellations.top();
    sys.recentCancellations.pop();

    int trainIdx = findTrainIndex(sys, lastCancelled.trainNo);
    if (trainIdx == -1 || sys.trains[trainIdx].availableSeats <= 0) {
        cout << "Cannot undo: Train #" << lastCancelled.trainNo << " is fully booked.\n";
        return;
    }

    int pIdx = findPassengerByPNR(sys, lastCancelled.pnr);
    if (pIdx == -1) return;

    int newSeat = findFreeSeat(sys, trainIdx);
    if (newSeat == -1) return;

    sys.passengers[pIdx].status = "CONFIRMED";
    sys.passengers[pIdx].seatNo = newSeat;
    updatePassengerStatus(db, lastCancelled.pnr, "CONFIRMED");

    sys.seatMap[trainIdx][newSeat - 1] = 1;
    sys.trains[trainIdx].availableSeats--;
    updateTrainSeats(db, lastCancelled.trainNo, sys.trains[trainIdx].availableSeats);

    exportTicketToFile(sys.passengers[pIdx], sys.trains[trainIdx]);
    cout << "\n[Restored] Ticket PNR " << lastCancelled.pnr << " reinstated (Seat #" << newSeat << ").\n";
}

void checkPNRStatus(const RailwaySystem& sys) {
    int pnr = readInt("\nEnter PNR Number: ", 1000, 999999);
    int idx = findPassengerByPNR(sys, pnr);
    if (idx == -1) {
        cout << "No booking record found for PNR " << pnr << ".\n";
        return;
    }
    const Passenger& p = sys.passengers[idx];
    cout << "\nBooking Details for PNR " << p.pnr << ":\n"
         << "  Name: " << p.name << " (" << p.age << " yrs, " << p.gender << ")\n"
         << "  Train: #" << p.trainNo << " | Seat: #" << p.seatNo << "\n"
         << "  Date: " << p.travelDate.day << "/" << p.travelDate.month << "/" << p.travelDate.year << "\n"
         << "  Status: " << p.status << " | Concession: " << p.concession << " | Fare Paid: Rs. " << fixed << setprecision(2) << p.farePaid << "\n";
}

void addTrain(RailwaySystem& sys, sqlite3* db) {
    cout << "\n--- Add New Train ---\n";
    int trainNo = readInt("Train Number (1000 - 99999): ", 1000, 99999);
    if (findTrainIndex(sys, trainNo) != -1) {
        cout << "Train #" << trainNo << " already exists!\n";
        return;
    }
    Train t;
    t.trainNo = trainNo;
    t.name = readNonEmptyString("Train Name: ");
    t.source = readNonEmptyString("Departure Station: ");
    t.destination = readNonEmptyString("Destination Station: ");
    t.departure = readNonEmptyString("Departure Time (e.g. 06:30 AM): ");
    t.totalSeats = readInt("Total Seats (1 - 60): ", 1, MAX_SEATS);
    t.availableSeats = t.totalSeats;
    cout << "Base Fare (Rs): ";
    cin >> t.fare;
    string dummy;
    getline(cin, dummy);

    insertTrain(db, t);
    insertTrainSorted(sys.trains, t);
    cout << "\n[Added] Train #" << t.trainNo << " (" << t.name << ") successfully registered.\n";
}

int main() {
    sqlite3* db = NULL;
    if (!openDatabase(db, "railway.db")) {
        cout << "Unable to connect to database.\n";
        return 1;
    }

    if (!createTables(db)) {
        cout << "Unable to initialize tables.\n";
        closeDatabase(db);
        return 1;
    }

    RailwaySystem sys;
    loadSystem(sys, db);

    if (sys.trains.empty()) {
        Train t1 = {10101, "Rajdhani Express", "Delhi", "Mumbai", "06:00 AM", 4, 4, 1500.0f};
        Train t2 = {10202, "Vande Bharat", "Chennai", "Bangalore", "05:50 AM", 5, 5, 950.0f};
        Train t3 = {10303, "Shatabdi Express", "Kolkata", "Patna", "02:15 PM", 3, 3, 750.0f};
        Train t4 = {10404, "Tejas Express", "Ahmedabad", "Mumbai", "06:40 AM", 4, 4, 1100.0f};

        insertTrain(db, t1); insertTrainSorted(sys.trains, t1);
        insertTrain(db, t2); insertTrainSorted(sys.trains, t2);
        insertTrain(db, t3); insertTrainSorted(sys.trains, t3);
        insertTrain(db, t4); insertTrainSorted(sys.trains, t4);
        buildSeatMap(sys);
    }

    int choice = -1;
    do {
        cout << "\n=======================================================\n"
             << "           RAILWAY TICKET RESERVATION SYSTEM           \n"
             << "=======================================================\n"
             << "  1. View Train Schedules & Fares\n"
             << "  2. Search Train by Train Number (Binary Search)\n"
             << "  3. Search Train by Destination (Linear Search)\n"
             << "  4. Check Available Seats & Coach Seat Layout (2D Array)\n"
             << "  5. Book a Ticket (Seat Allocation & Concession)\n"
             << "  6. Cancel Confirmed Ticket (Auto-Promotes Waiting List)\n"
             << "  7. Cancel Waiting List Entry\n"
             << "  8. Undo Last Cancellation (Stack LIFO)\n"
             << "  9. View PNR Status & Print E-Ticket Slip\n"
             << " 10. Sort Trains for Display (Bubble Sort)\n"
             << " 11. Add New Train to Fleet (Admin)\n"
             << "  0. Exit Application\n"
             << "=======================================================\n";

        choice = readInt("Select an option (0 - 11): ", 0, 11);
        switch (choice) {
            case 1: displayTrains(sys); break;
            case 2: searchTrain(sys); break;
            case 3: searchTrainByDestination(sys); break;
            case 4: {
                int tNo = readInt("Enter Train Number: ", 1000, 99999);
                int idx = findTrainIndex(sys, tNo);
                if (idx != -1) displaySeatMap(sys, idx);
                else cout << "Train #" << tNo << " not found.\n";
                break;
            }
            case 5: bookTicket(sys, db); break;
            case 6: cancelTicket(sys, db); break;
            case 7: cancelWaitingListEntry(sys, db); break;
            case 8: undoLastCancellation(sys, db); break;
            case 9: checkPNRStatus(sys); break;
            case 10: sortTrains(sys); break;
            case 11: addTrain(sys, db); break;
            case 0: cout << "\nThank you for using the Railway Reservation System. Goodbye!\n"; break;
        }
    } while (choice != 0);

    closeDatabase(db);
    return 0;
}
