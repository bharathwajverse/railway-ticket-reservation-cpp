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

const int MAX_TRAINS = 20;
const int MAX_SEATS = 60;
const int MAX_WAITING = 10;

mutex g_sysMutex;

struct Date {
    int day, month, year;
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

struct SystemStats {
    int totalTrains;
    int totalBookings;
    int confirmedBookings;
    int cancelledBookings;
    int totalWaitlisted;
    float totalRevenue;
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
    int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (m == 2 && isLeapYear(y)) days[1] = 29;
    return d >= 1 && d <= days[m - 1];
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

bool beginTransaction(sqlite3* db) {
    return sqlite3_exec(db, "BEGIN IMMEDIATE TRANSACTION;", NULL, NULL, NULL) == SQLITE_OK;
}

bool commitTransaction(sqlite3* db) {
    return sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) == SQLITE_OK;
}

bool rollbackTransaction(sqlite3* db) {
    return sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL) == SQLITE_OK;
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

    beginTransaction(db);
    sys.seatMap[trainIndex][seatNo - 1] = 1;
    insertPassenger(db, promoted);
    commitTransaction(db);

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

    lock_guard<mutex> lock(g_sysMutex);
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

        beginTransaction(db);
        sys.seatMap[trainIdx][allocatedSeat - 1] = 1;
        train.availableSeats--;
        updateTrainSeats(db, trainNo, train.availableSeats);
        insertPassenger(db, p);
        commitTransaction(db);

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

    lock_guard<mutex> lock(g_sysMutex);
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

    beginTransaction(db);
    p.status = "CANCELLED";
    updatePassengerStatus(db, pnr, "CANCELLED");

    if (trainIdx != -1) {
        if (freedSeat >= 1 && freedSeat <= sys.trains[trainIdx].totalSeats) {
            sys.seatMap[trainIdx][freedSeat - 1] = 0;
        }
        sys.trains[trainIdx].availableSeats++;
        updateTrainSeats(db, p.trainNo, sys.trains[trainIdx].availableSeats);
    }
    commitTransaction(db);

    sys.recentCancellations.push(p);
    cout << "\n[Cancelled] Ticket PNR " << pnr << " has been cancelled.\n";

    if (trainIdx != -1) promoteFromWaitingList(sys, db, trainIdx, freedSeat);
}

void cancelWaitingListEntry(RailwaySystem& sys, sqlite3* db) {
    cout << "\n--- Cancel Waiting List Entry ---\n";
    int trainNo = readInt("Enter Train Number: ", 1000, 99999);
    string passengerName = readNonEmptyString("Passenger Name: ");

    lock_guard<mutex> lock(g_sysMutex);
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
    lock_guard<mutex> lock(g_sysMutex);

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

    beginTransaction(db);
    sys.passengers[pIdx].status = "CONFIRMED";
    sys.passengers[pIdx].seatNo = newSeat;
    updatePassengerStatus(db, lastCancelled.pnr, "CONFIRMED");

    sys.seatMap[trainIdx][newSeat - 1] = 1;
    sys.trains[trainIdx].availableSeats--;
    updateTrainSeats(db, lastCancelled.trainNo, sys.trains[trainIdx].availableSeats);
    commitTransaction(db);

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

SystemStats calculateStats(const RailwaySystem& sys) {
    SystemStats s;
    s.totalTrains = (int)sys.trains.size();
    s.totalBookings = (int)sys.passengers.size();
    s.confirmedBookings = 0;
    s.cancelledBookings = 0;
    s.totalWaitlisted = 0;
    s.totalRevenue = 0.0f;

    for (size_t i = 0; i < sys.passengers.size(); i++) {
        if (sys.passengers[i].status == "CONFIRMED") {
            s.confirmedBookings++;
            s.totalRevenue += sys.passengers[i].farePaid;
        } else {
            s.cancelledBookings++;
        }
    }
    for (auto it = sys.waitingLists.begin(); it != sys.waitingLists.end(); ++it) {
        s.totalWaitlisted += (int)it->second.size();
    }
    return s;
}

void displaySystemStats(const RailwaySystem& sys) {
    SystemStats s = calculateStats(sys);
    cout << "\n---------------- System Analytics ----------------\n"
         << "  Total Trains in Fleet  : " << s.totalTrains << "\n"
         << "  Total Tickets Handled  : " << s.totalBookings << "\n"
         << "  Confirmed Bookings     : " << s.confirmedBookings << "\n"
         << "  Cancelled Tickets      : " << s.cancelledBookings << "\n"
         << "  Active Waiting Queue   : " << s.totalWaitlisted << "\n"
         << "  Total Revenue Collected: Rs. " << fixed << setprecision(2) << s.totalRevenue << "\n"
         << "--------------------------------------------------\n";
}

void displayManifest(const RailwaySystem& sys) {
    if (sys.passengers.empty()) {
        cout << "\nNo passenger bookings recorded.\n";
        return;
    }
    cout << "\n" << setw(6) << "PNR" << " | " << setw(18) << left << "Passenger" << " | "
         << setw(6) << "Age/G" << " | " << setw(7) << "Train#" << " | " << setw(5) << "Seat" << " | "
         << setw(11) << "Status" << " | " << setw(10) << "Fare Paid" << right << "\n";
    cout << "----------------------------------------------------------------------------\n";
    for (size_t i = 0; i < sys.passengers.size(); i++) {
        const Passenger& p = sys.passengers[i];
        string ageGen = to_string(p.age) + "/" + string(1, p.gender);
        cout << setw(6) << p.pnr << " | " << setw(18) << left << p.name << " | "
             << setw(6) << ageGen << " | " << setw(7) << p.trainNo << " | " << setw(5) << p.seatNo << " | "
             << setw(11) << p.status << " | Rs. " << setw(7) << fixed << setprecision(2) << p.farePaid << right << "\n";
    }
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

    lock_guard<mutex> lock(g_sysMutex);
    insertTrain(db, t);
    insertTrainSorted(sys.trains, t);
    cout << "\n[Added] Train #" << t.trainNo << " (" << t.name << ") successfully registered.\n";
}

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
        return (endPos != string::npos) ? json.substr(pos, endPos - pos) : "";
    } else {
        size_t endPos = pos;
        while (endPos < json.length() && json[endPos] != ',' && json[endPos] != '}' && json[endPos] != '\r' && json[endPos] != '\n' && json[endPos] != ' ') {
            endPos++;
        }
        return json.substr(pos, endPos - pos);
    }
}

string getQueryParam(const string& path, const string& key) {
    string target = key + "=";
    size_t pos = path.find(target);
    if (pos == string::npos) return "";
    pos += target.length();
    size_t endPos = path.find('&', pos);
    if (endPos == string::npos) endPos = path.find(' ', pos);
    if (endPos == string::npos) endPos = path.length();
    return path.substr(pos, endPos - pos);
}

void sendHttpResponse(SOCKET s, const string& contentType, const string& body, int status = 200) {
    string statusText = (status == 200) ? "200 OK" : "404 Not Found";
    string header = "HTTP/1.1 " + statusText + "\r\n"
                    "Content-Type: " + contentType + "\r\n"
                    "Access-Control-Allow-Origin: *\r\n"
                    "Content-Length: " + to_string(body.length()) + "\r\n"
                    "Connection: close\r\n\r\n";
    send(s, header.c_str(), (int)header.length(), 0);
    send(s, body.c_str(), (int)body.length(), 0);
    CLOSE_SOCKET(s);
}

string passengerToJson(const Passenger& p) {
    stringstream json;
    json << "{\"pnr\":" << p.pnr << ",\"name\":\"" << p.name << "\",\"age\":" << p.age
         << ",\"gender\":\"" << p.gender << "\",\"trainNo\":" << p.trainNo << ",\"seatNo\":" << p.seatNo
         << ",\"date\":\"" << p.travelDate.day << "/" << p.travelDate.month << "/" << p.travelDate.year
         << "\",\"status\":\"" << p.status << "\",\"concession\":\"" << p.concession
         << "\",\"farePaid\":" << fixed << setprecision(2) << p.farePaid << "}";
    return json.str();
}

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

    if (path == "/" || path == "/index.html") {
        ifstream htmlFile("web/index.html");
        string body;
        if (htmlFile.is_open()) {
            stringstream ss;
            ss << htmlFile.rdbuf();
            body = ss.str();
            htmlFile.close();
        } else {
            body = "<html><body><h2>Railway Reservation System Backend Online</h2></body></html>";
        }
        sendHttpResponse(clientSocket, "text/html; charset=utf-8", body);
    } else if (path == "/api/trains") {
        lock_guard<mutex> lock(g_sysMutex);
        stringstream json;
        json << "[";
        for (size_t i = 0; i < sys.trains.size(); i++) {
            const Train& t = sys.trains[i];
            json << "{\"trainNo\":" << t.trainNo << ",\"name\":\"" << t.name << "\",\"source\":\"" << t.source
                 << "\",\"destination\":\"" << t.destination << "\",\"departure\":\"" << t.departure
                 << "\",\"totalSeats\":" << t.totalSeats << ",\"availableSeats\":" << t.availableSeats
                 << ",\"fare\":" << fixed << setprecision(2) << t.fare << "}";
            if (i + 1 < sys.trains.size()) json << ",";
        }
        json << "]";
        sendHttpResponse(clientSocket, "application/json", json.str());
    } else if (path.find("/api/seats") == 0) {
        int tNo = atoi(getQueryParam(path, "trainNo").c_str());
        lock_guard<mutex> lock(g_sysMutex);
        int tIdx = findTrainIndex(sys, tNo);
        if (tIdx != -1) {
            stringstream json;
            json << "{\"trainNo\":" << tNo << ",\"totalSeats\":" << sys.trains[tIdx].totalSeats
                 << ",\"availableSeats\":" << sys.trains[tIdx].availableSeats << ",\"seats\":[";
            for (int s = 0; s < sys.trains[tIdx].totalSeats; s++) {
                json << sys.seatMap[tIdx][s];
                if (s + 1 < sys.trains[tIdx].totalSeats) json << ",";
            }
            json << "]}";
            sendHttpResponse(clientSocket, "application/json", json.str());
        } else {
            sendHttpResponse(clientSocket, "application/json", "{\"error\":\"Train not found\"}");
        }
    } else if (path.find("/api/pnr") == 0) {
        int pnr = atoi(getQueryParam(path, "pnr").c_str());
        lock_guard<mutex> lock(g_sysMutex);
        int pIdx = findPassengerByPNR(sys, pnr);
        if (pIdx != -1) {
            sendHttpResponse(clientSocket, "application/json", "{\"found\":true," + passengerToJson(sys.passengers[pIdx]).substr(1));
        } else {
            sendHttpResponse(clientSocket, "application/json", "{\"found\":false}");
        }
    } else if (path == "/api/stats") {
        lock_guard<mutex> lock(g_sysMutex);
        SystemStats s = calculateStats(sys);
        stringstream json;
        json << "{\"totalTrains\":" << s.totalTrains << ",\"totalBookings\":" << s.totalBookings
             << ",\"confirmedBookings\":" << s.confirmedBookings << ",\"cancelledBookings\":" << s.cancelledBookings
             << ",\"totalWaitlisted\":" << s.totalWaitlisted << ",\"totalRevenue\":" << fixed << setprecision(2) << s.totalRevenue << "}";
        sendHttpResponse(clientSocket, "application/json", json.str());
    } else if (path == "/api/manifest") {
        lock_guard<mutex> lock(g_sysMutex);
        stringstream json;
        json << "[";
        for (size_t i = 0; i < sys.passengers.size(); i++) {
            json << passengerToJson(sys.passengers[i]);
            if (i + 1 < sys.passengers.size()) json << ",";
        }
        json << "]";
        sendHttpResponse(clientSocket, "application/json", json.str());
    } else if (path.find("/api/ticket") == 0) {
        int pnr = atoi(getQueryParam(path, "pnr").c_str());
        string fileName = "ticket_" + to_string(pnr) + ".txt";
        ifstream tFile(fileName.c_str());
        string body = "Ticket receipt file not found.";
        if (tFile.is_open()) {
            stringstream ss;
            ss << tFile.rdbuf();
            body = ss.str();
            tFile.close();
        }
        sendHttpResponse(clientSocket, "text/plain; charset=utf-8", body);
    } else if (method == "POST" && path == "/api/book") {
        size_t bodyPos = request.find("\r\n\r\n");
        string body = (bodyPos != string::npos) ? request.substr(bodyPos + 4) : "";

        int trainNo = atoi(extractJsonField(body, "trainNo").c_str());
        string name = extractJsonField(body, "name");
        int age = atoi(extractJsonField(body, "age").c_str());
        string gStr = extractJsonField(body, "gender");
        char gender = (!gStr.empty()) ? gStr[0] : 'M';
        string dateStr = extractJsonField(body, "dateStr");
        int seatNo = atoi(extractJsonField(body, "seatNo").c_str());

        Date travelDate = {15, 10, 2026};
        if (dateStr.length() >= 10) {
            travelDate.year = atoi(dateStr.substr(0, 4).c_str());
            travelDate.month = atoi(dateStr.substr(5, 2).c_str());
            travelDate.day = atoi(dateStr.substr(8, 2).c_str());
        }

        lock_guard<mutex> lock(g_sysMutex);
        int trainIdx = findTrainIndex(sys, trainNo);
        if (trainIdx == -1) {
            sendHttpResponse(clientSocket, "application/json", "{\"success\":false,\"error\":\"Train does not exist\"}");
        } else {
            int freeSeat = findFreeSeat(sys, trainIdx);
            if (freeSeat != -1 && sys.trains[trainIdx].availableSeats > 0) {
                int allocated = (seatNo >= 1 && seatNo <= sys.trains[trainIdx].totalSeats && sys.seatMap[trainIdx][seatNo - 1] == 0) ? seatNo : freeSeat;
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
                     << ",\"name\":\"" << p.name << "\",\"age\":" << p.age << ",\"seatNo\":" << p.seatNo
                     << ",\"concession\":\"" << p.concession << "\",\"farePaid\":" << fixed << setprecision(2) << p.farePaid << "}";
                sendHttpResponse(clientSocket, "application/json", json.str());
            } else {
                queue<WaitingEntry>& wQueue = sys.waitingLists[trainNo];
                if ((int)wQueue.size() >= MAX_WAITING) {
                    sendHttpResponse(clientSocket, "application/json", "{\"success\":false,\"error\":\"Train and Waiting List are both FULL.\"}");
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
                    sendHttpResponse(clientSocket, "application/json", json.str());
                }
            }
        }
    } else if (method == "POST" && path == "/api/cancel") {
        size_t bodyPos = request.find("\r\n\r\n");
        string body = (bodyPos != string::npos) ? request.substr(bodyPos + 4) : "";
        int pnr = atoi(extractJsonField(body, "pnr").c_str());

        lock_guard<mutex> lock(g_sysMutex);
        int pIdx = findPassengerByPNR(sys, pnr);

        if (pIdx == -1) {
            sendHttpResponse(clientSocket, "application/json", "{\"success\":false,\"error\":\"Ticket PNR not found\"}");
        } else if (sys.passengers[pIdx].status == "CANCELLED") {
            sendHttpResponse(clientSocket, "application/json", "{\"success\":false,\"error\":\"Ticket is already cancelled\"}");
        } else {
            Passenger& p = sys.passengers[pIdx];
            int trainIdx = findTrainIndex(sys, p.trainNo);
            int freedSeat = p.seatNo;

            beginTransaction(db);
            p.status = "CANCELLED";
            updatePassengerStatus(db, pnr, "CANCELLED");

            if (trainIdx != -1) {
                if (freedSeat >= 1 && freedSeat <= sys.trains[trainIdx].totalSeats) {
                    sys.seatMap[trainIdx][freedSeat - 1] = 0;
                }
                sys.trains[trainIdx].availableSeats++;
                updateTrainSeats(db, p.trainNo, sys.trains[trainIdx].availableSeats);
            }
            commitTransaction(db);

            sys.recentCancellations.push(p);

            if (trainIdx != -1) promoteFromWaitingList(sys, db, trainIdx, freedSeat);
            sendHttpResponse(clientSocket, "application/json", "{\"success\":true,\"message\":\"Ticket cancelled successfully.\"}");
        }
    } else {
        sendHttpResponse(clientSocket, "application/json", "{\"error\":\"Endpoint not found\"}", 404);
    }
}

void runHttpServer(RailwaySystem* sysPtr, sqlite3* dbPtr, int port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return;
#endif

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons((u_short)port);

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
        if (clientSocket != INVALID_SOCKET) {
            handleHttpClient(clientSocket, *sysPtr, dbPtr);
        }
    }

    CLOSE_SOCKET(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif
}

void displayPortalSelectionMenu() {
    cout << "\n=======================================================\n"
         << "           RAILWAY TICKET RESERVATION SYSTEM           \n"
         << "=======================================================\n"
         << "  1. Passenger Portal (Book, Cancel, Status, E-Ticket)\n"
         << "  2. Administrator Portal (PIN Protected: Fleet, Stats)\n"
         << "  3. Launch Modern Web Dashboard (http://localhost:8080)\n"
         << "  0. Exit Application\n"
         << "=======================================================\n";
}

void displayPassengerMenu() {
    cout << "\n-------------------------------------------------------\n"
         << "                   PASSENGER PORTAL                    \n"
         << "-------------------------------------------------------\n"
         << "  1. View Train Schedules & Fares\n"
         << "  2. Search Train (By Number or Destination)\n"
         << "  3. Check Available Seats & Coach Layout\n"
         << "  4. Book a Ticket (Manual Seat & Concession)\n"
         << "  5. Cancel Confirmed Ticket (by PNR)\n"
         << "  6. Cancel Waiting List Entry\n"
         << "  7. View PNR Status\n"
         << "  8. Sort Trains (Bubble Sort by Fare/Name)\n"
         << "  9. Undo Last Cancellation (Stack LIFO)\n"
         << "  0. Return to Main Portal Menu\n"
         << "-------------------------------------------------------\n";
}

void displayAdminMenu() {
    cout << "\n-------------------------------------------------------\n"
         << "              ADMINISTRATOR MANAGEMENT PORTAL          \n"
         << "-------------------------------------------------------\n"
         << "  1. Add New Train to Fleet\n"
         << "  2. View Passenger Manifest (All Bookings)\n"
         << "  3. View Coach Seat Grid for Train\n"
         << "  4. View Waiting List Queue for Train\n"
         << "  5. View Executive System Analytics (Revenue & Bookings)\n"
         << "  0. Return to Main Portal Menu\n"
         << "-------------------------------------------------------\n";
}

void runPassengerPortal(RailwaySystem& sys, sqlite3* db) {
    int choice = -1;
    do {
        displayPassengerMenu();
        choice = readInt("Select an option (0 - 9): ", 0, 9);
        switch (choice) {
            case 1: displayTrains(sys); break;
            case 2: {
                cout << "\n1. Search by Train Number (Binary Search)\n2. Search by Destination (Linear Search)\n";
                int sChoice = readInt("Choice (1 or 2): ", 1, 2);
                if (sChoice == 1) searchTrain(sys);
                else searchTrainByDestination(sys);
                break;
            }
            case 3: {
                int tNo = readInt("\nEnter Train Number: ", 1000, 99999);
                int idx = findTrainIndex(sys, tNo);
                if (idx != -1) displaySeatMap(sys, idx);
                else cout << "Train #" << tNo << " not found.\n";
                break;
            }
            case 4: bookTicket(sys, db); break;
            case 5: cancelTicket(sys, db); break;
            case 6: cancelWaitingListEntry(sys, db); break;
            case 7: checkPNRStatus(sys); break;
            case 8: sortTrains(sys); break;
            case 9: undoLastCancellation(sys, db); break;
            case 0: cout << "\nReturning to Portal Selector...\n"; break;
        }
    } while (choice != 0);
}

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
        aChoice = readInt("Select an option (0 - 5): ", 0, 5);
        switch (aChoice) {
            case 1: addTrain(sys, db); break;
            case 2: displayManifest(sys); break;
            case 3: {
                int tNo = readInt("\nEnter Train Number: ", 1000, 99999);
                int idx = findTrainIndex(sys, tNo);
                if (idx != -1) displaySeatMap(sys, idx);
                else cout << "Train #" << tNo << " not found.\n";
                break;
            }
            case 4: {
                int tNo = readInt("\nEnter Train Number: ", 1000, 99999);
                lock_guard<mutex> lock(g_sysMutex);
                if (sys.waitingLists.find(tNo) == sys.waitingLists.end() || sys.waitingLists[tNo].empty()) {
                    cout << "Waiting queue is empty for Train #" << tNo << ".\n";
                } else {
                    queue<WaitingEntry> copyQ = sys.waitingLists[tNo];
                    cout << "\nWaiting List Queue for Train #" << tNo << " (FIFO Order):\n";
                    int pos = 1;
                    while (!copyQ.empty()) {
                        WaitingEntry w = copyQ.front();
                        copyQ.pop();
                        cout << "  " << pos++ << ". " << w.name << " (" << w.age << " yrs, " << w.gender << ")\n";
                    }
                }
                break;
            }
            case 5: displaySystemStats(sys); break;
            case 0: cout << "\nReturning to Portal Selector...\n"; break;
        }
    } while (aChoice != 0);
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        string arg = argv[1];
        if (arg == "--version" || arg == "-v") {
            cout << "Railway Ticket Reservation System v2.0.0 (C++11/SQLite3/Winsock)\n";
            cout << "High-Performance In-Memory Data Structures & Embedded REST Architecture\n";
            return 0;
        }
        if (arg == "--help" || arg == "-h") {
            cout << "Usage: railway.exe [OPTIONS]\n"
                 << "  --version, -v   Display software version information\n"
                 << "  --help, -h      Display command-line help flags\n";
            return 0;
        }
    }

    cout << "\n=======================================================\n"
         << " >>> Starting Railway Ticket Reservation System v2 <<< \n"
         << "=======================================================\n";

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

    thread serverThread(runHttpServer, &sys, db, 8080);
    serverThread.detach();

    cout << "[System Online] Winsock HTTP Server running on http://localhost:8080\n";
    cout << "[System Ready] " << sys.trains.size() << " trains loaded into memory.\n";

    int portalChoice = -1;
    do {
        displayPortalSelectionMenu();
        portalChoice = readInt("Select Portal (0 - 3): ", 0, 3);
        switch (portalChoice) {
            case 1: runPassengerPortal(sys, db); break;
            case 2: runAdminPortal(sys, db); break;
            case 3:
                cout << "\nOpening web dashboard at http://localhost:8080 ...\n";
#ifdef _WIN32
                system("start http://localhost:8080");
#elif __APPLE__
                system("open http://localhost:8080");
#else
                system("xdg-open http://localhost:8080");
#endif
                break;
            case 0: cout << "\nThank you for using the Railway Reservation System. Goodbye!\n"; break;
        }
    } while (portalChoice != 0);

    closeDatabase(db);
    return 0;
}
