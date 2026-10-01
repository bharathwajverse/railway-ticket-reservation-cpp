#include "dsa_manager.h"
#include "database.h"
#include <iomanip>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>

using namespace std;

// ============================================================================
// MODULE I & II: BASIC C++ AND CONTROL STATEMENTS
// Type conversions, leap year logic, input validation, age-based concessions
// ============================================================================

bool isLeapYear(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

bool isValidDate(int d, int m, int y) {
    if (y < 2024 || y > 2035 || m < 1 || m > 12) return false;
    // Module III: 1D Numeric Array initialization
    int daysInMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (m == 2 && isLeapYear(y)) daysInMonth[1] = 29;
    return d >= 1 && d <= daysInMonth[m - 1];
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
            if (cin.eof()) return minVal;
            cin.clear();
            string bad;
            cin >> bad;
            cout << "Invalid input. Please enter a number.\n";
        }
    }
}

float readFloat(const string& prompt, float minVal, float maxVal) {
    float val;
    while (true) {
        cout << prompt;
        if (cin >> val) {
            string dummy;
            getline(cin, dummy);
            if (val >= minVal && val <= maxVal) return val;
            cout << "Input out of range (" << minVal << " - " << maxVal << ").\n";
        } else {
            if (cin.eof()) return minVal;
            cin.clear();
            string bad;
            cin >> bad;
            cout << "Invalid input. Please enter a valid number.\n";
        }
    }
}

string readNonEmptyString(const string& prompt) {
    string val;
    while (true) {
        cout << prompt;
        if (!getline(cin, val)) return "";
        size_t start = val.find_first_not_of(" \t\r\n");
        size_t end = val.find_last_not_of(" \t\r\n");
        if (start != string::npos && end != string::npos) return val.substr(start, end - start + 1);
        if (cin.eof()) return "";
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

// ============================================================================
// MODULE III: ARRAYS – 1D
// Operations on 1D arrays, passing 1D arrays to functions
// ============================================================================

int sum1DArray(const int arr[], int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) sum += arr[i];
    return sum;
}

void reset1DArray(int arr[], int size, int defaultVal) {
    for (int i = 0; i < size; i++) arr[i] = defaultVal;
}

// ============================================================================
// MODULE IV: ARRAYS – 2D
// 2D numeric seat matrix operations (initialization, rendering, searching)
// ============================================================================

void initSeatMatrix(int matrix[MAX_TRAINS][MAX_SEATS]) {
    for (int i = 0; i < MAX_TRAINS; i++) {
        for (int j = 0; j < MAX_SEATS; j++) {
            matrix[i][j] = 0; // 0 = Free, 1 = Booked
        }
    }
}

void displaySeatMatrix(const int matrix[MAX_TRAINS][MAX_SEATS], int trainIdx, int totalSeats, int trainNo, const string& trainName) {
    cout << "\nCoach Seat Layout for Train #" << trainNo << " (" << trainName << "):\n";
    cout << "[.] = Available  [X] = Booked\n\n";
    for (int s = 1; s <= totalSeats; s++) {
        int occupied = matrix[trainIdx][s - 1];
        cout << "[" << (occupied ? "X" : ".") << " " << setw(2) << s << "] ";
        if (s % 4 == 2) cout << "   ";
        if (s % 4 == 0) cout << "\n";
    }
    if (totalSeats % 4 != 0) cout << "\n";
}

int findFirstFreeSeatInMatrix(const int matrix[MAX_TRAINS][MAX_SEATS], int trainIdx, int totalSeats) {
    for (int s = 0; s < totalSeats; s++) {
        if (matrix[trainIdx][s] == 0) return s + 1;
    }
    return -1;
}

int countOccupiedInMatrix(const int matrix[MAX_TRAINS][MAX_SEATS], int trainIdx, int totalSeats) {
    int count = 0;
    for (int s = 0; s < totalSeats; s++) {
        if (matrix[trainIdx][s] == 1) count++;
    }
    return count;
}

// ============================================================================
// MODULE V: STRING ARRAYS & MANIPULATION
// Methods, traversal, character frequency, reversal, word tokenization
// ============================================================================

string toLowerCase(string s) {
    for (size_t i = 0; i < s.length(); i++) s[i] = (char)tolower(s[i]);
    return s;
}

bool containsIgnoreCase(const string& text, const string& pattern) {
    return toLowerCase(text).find(toLowerCase(pattern)) != string::npos;
}

int countCharFrequency(const string& str, char ch) {
    int count = 0;
    char target = (char)tolower(ch);
    for (size_t i = 0; i < str.length(); i++) {
        if ((char)tolower(str[i]) == target) count++;
    }
    return count;
}

string reverseString(const string& str) {
    string rev = "";
    for (int i = (int)str.length() - 1; i >= 0; i--) {
        rev += str[i];
    }
    return rev;
}

vector<string> tokenizeRoute(const string& routeStr, char delimiter) {
    vector<string> tokens;
    stringstream ss(routeStr);
    string token;
    while (getline(ss, token, delimiter)) {
        size_t start = token.find_first_not_of(" \t");
        size_t end = token.find_last_not_of(" \t");
        if (start != string::npos && end != string::npos) {
            tokens.push_back(token.substr(start, end - start + 1));
        }
    }
    return tokens;
}

// ============================================================================
// MODULE VII: DATA STRUCTURES PERFORMANCE ANALYSIS
// Binary Search O(log N), Linear Search O(N), Bubble Sort O(N^2)
// ============================================================================

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

vector<int> linearSearchByDestination(const vector<Train>& trains, const string& dest) {
    vector<int> matches;
    for (size_t i = 0; i < trains.size(); i++) {
        if (containsIgnoreCase(trains[i].destination, dest)) {
            matches.push_back((int)i);
        }
    }
    return matches;
}

void bubbleSortTrainsByFare(vector<Train>& trains) {
    int n = (int)trains.size();
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (trains[j].fare > trains[j + 1].fare) {
                Train temp = trains[j];
                trains[j] = trains[j + 1];
                trains[j + 1] = temp;
            }
        }
    }
}

void bubbleSortTrainsByName(vector<Train>& trains) {
    int n = (int)trains.size();
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (trains[j].name > trains[j + 1].name) {
                Train temp = trains[j];
                trains[j] = trains[j + 1];
                trains[j + 1] = temp;
            }
        }
    }
}

void bubbleSortTrainsByNumber(vector<Train>& trains) {
    int n = (int)trains.size();
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (trains[j].trainNo > trains[j + 1].trainNo) {
                Train temp = trains[j];
                trains[j] = trains[j + 1];
                trains[j + 1] = temp;
            }
        }
    }
}

void insertTrainSorted(vector<Train>& trains, const Train& t) {
    size_t i = 0;
    while (i < trains.size() && trains[i].trainNo < t.trainNo) i++;
    trains.insert(trains.begin() + i, t);
}

// ============================================================================
// MODULE VIII: STACKS (STACK IMPLEMENTATION USING ARRAYS)
// LIFO operations for Ticket Cancellation History and Undo
// ============================================================================

ArrayStack::ArrayStack() : topIndex(-1) {}

bool ArrayStack::push(const Passenger& p) {
    if (isFull()) return false;
    data[++topIndex] = p;
    return true;
}

bool ArrayStack::pop(Passenger& out) {
    if (isEmpty()) return false;
    out = data[topIndex--];
    return true;
}

bool ArrayStack::peek(Passenger& out) const {
    if (isEmpty()) return false;
    out = data[topIndex];
    return true;
}

bool ArrayStack::isEmpty() const { return topIndex == -1; }
bool ArrayStack::isFull() const { return topIndex == MAX_STACK - 1; }
int ArrayStack::size() const { return topIndex + 1; }

// ============================================================================
// MODULE IX: QUEUES (QUEUES IMPLEMENTATION USING ARRAYS)
// Circular Array Queue for FIFO Waiting List with Auto-Promotion
// ============================================================================

ArrayQueue::ArrayQueue() : frontIndex(0), rearIndex(-1), count(0) {}

bool ArrayQueue::enqueue(const WaitingEntry& w) {
    if (isFull()) return false;
    rearIndex = (rearIndex + 1) % MAX_QUEUE;
    data[rearIndex] = w;
    count++;
    return true;
}

bool ArrayQueue::dequeue(WaitingEntry& out) {
    if (isEmpty()) return false;
    out = data[frontIndex];
    frontIndex = (frontIndex + 1) % MAX_QUEUE;
    count--;
    return true;
}

bool ArrayQueue::peek(WaitingEntry& out) const {
    if (isEmpty()) return false;
    out = data[frontIndex];
    return true;
}

bool ArrayQueue::isEmpty() const { return count == 0; }
bool ArrayQueue::isFull() const { return count == MAX_QUEUE; }
int ArrayQueue::size() const { return count; }

bool ArrayQueue::removeById(int waitId, WaitingEntry& out) {
    if (isEmpty()) return false;
    ArrayQueue temp;
    bool found = false;
    while (!isEmpty()) {
        WaitingEntry curr;
        dequeue(curr);
        if (!found && curr.waitId == waitId) {
            out = curr;
            found = true;
        } else {
            temp.enqueue(curr);
        }
    }
    while (!temp.isEmpty()) {
        WaitingEntry curr;
        temp.dequeue(curr);
        enqueue(curr);
    }
    return found;
}

// ============================================================================
// MODULE X: STL FUNDAMENTALS & LARGE DSA MANAGER
// Pairs, Vectors, Maps, Sets, Deque, coordinating all 10 modules
// ============================================================================

DSAManager::DSAManager() {
    initSeatMatrix(seatMap);
}

int DSAManager::getTrainCount() const {
    return (int)trains.size();
}

int DSAManager::findTrainIndex(int trainNo) const {
    return binarySearchTrain(trains, trainNo);
}

int DSAManager::findPassengerIndex(int pnr) const {
    for (size_t i = 0; i < passengers.size(); i++) {
        if (passengers[i].pnr == pnr) return (int)i;
    }
    return -1;
}

int DSAManager::generatePNR() const {
    int maxPnr = 1000;
    for (size_t i = 0; i < passengers.size(); i++) {
        if (passengers[i].pnr > maxPnr) maxPnr = passengers[i].pnr;
    }
    return maxPnr + 1;
}

pair<int, string> DSAManager::getTrainSummary(int trainNo) const {
    int idx = findTrainIndex(trainNo);
    if (idx != -1) return make_pair(trains[idx].trainNo, trains[idx].name);
    return make_pair(-1, "Not Found");
}

void DSAManager::rebuildSeatMap() {
    initSeatMatrix(seatMap);
    for (size_t i = 0; i < passengers.size(); i++) {
        const Passenger& p = passengers[i];
        if (p.status == "CONFIRMED") {
            int trainIdx = findTrainIndex(p.trainNo);
            if (trainIdx != -1 && p.seatNo >= 1 && p.seatNo <= MAX_SEATS) {
                seatMap[trainIdx][p.seatNo - 1] = 1;
            }
        }
    }
}

void DSAManager::loadFromDatabase() {
    dbLoadTrains(trains);
    bubbleSortTrainsByNumber(trains);
    dbLoadPassengers(passengers);

    vector<WaitingEntry> dbWaiting;
    dbLoadWaiting(dbWaiting);

    waitingLists.clear();
    for (size_t i = 0; i < dbWaiting.size(); i++) {
        waitingLists[dbWaiting[i].trainNo].enqueue(dbWaiting[i]);
    }

    uniqueStations.clear();
    for (size_t i = 0; i < trains.size(); i++) {
        uniqueStations.insert(trains[i].source);
        uniqueStations.insert(trains[i].destination);
    }

    rebuildSeatMap();
    auditLog.push_back("[Init] System state loaded from MongoDB document collections.");
}

bool DSAManager::exportTicketToFile(const Passenger& p, const Train& t) const {
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
    string trainPrefix = (t.name.length() >= 3) ? t.name.substr(0, 3) : t.name;
    fout << "VERIFICATION HASH : " << reverseString(to_string(p.pnr) + trainPrefix) << "\n";
    fout << "======================================================================\n";
    fout.close();
    return true;
}

void DSAManager::displayTrains() const {
    if (trains.empty()) {
        cout << "No trains currently registered in system.\n";
        return;
    }
    cout << "\n" << string(76, '=') << "\n";
    cout << "                         AVAILABLE TRAIN SCHEDULES                   \n";
    cout << string(76, '=') << "\n";
    cout << left << setw(8)  << "Trn#"
         << setw(20) << "Name"
         << setw(13) << "Source"
         << setw(13) << "Destination"
         << setw(10) << "Departs"
         << setw(6)  << "Seats"
         << right << setw(6) << "Fare" << "\n";
    cout << string(76, '-') << "\n";

    for (vector<Train>::const_iterator it = trains.begin(); it != trains.end(); ++it) {
        cout << left << setw(8)  << it->trainNo
             << setw(20) << it->name
             << setw(13) << it->source
             << setw(13) << it->destination
             << setw(10) << it->departure
             << setw(6)  << (to_string(it->availableSeats) + "/" + to_string(it->totalSeats))
             << right << setw(6) << fixed << setprecision(0) << it->fare << "\n";
    }
    cout << string(76, '=') << "\n";
}

void DSAManager::searchTrainByNumber() const {
    int tNo = readInt("Enter Train Number to search (Binary Search): ", 1000, 99999);
    int idx = findTrainIndex(tNo);
    if (idx != -1) {
        const Train& t = trains[idx];
        cout << "\n[Match Found via O(log N) Binary Search]\n"
             << "Train: #" << t.trainNo << " - " << t.name << "\n"
             << "Route: " << t.source << " -> " << t.destination << "\n"
             << "Departure Time: " << t.departure << "\n"
             << "Available Seats: " << t.availableSeats << "/" << t.totalSeats << "\n"
             << "Base Fare: Rs. " << fixed << setprecision(2) << t.fare << "\n";
    } else {
        cout << "Train #" << tNo << " not found in schedule.\n";
    }
}

void DSAManager::searchTrainByDestination() const {
    string dest = readNonEmptyString("Enter Destination Station (Linear Search): ");
    vector<int> matches = linearSearchByDestination(trains, dest);
    if (matches.empty()) {
        cout << "No trains found terminating at '" << dest << "'.\n";
        return;
    }
    cout << "\nFound " << matches.size() << " train(s) to '" << dest << "':\n";
    for (size_t i = 0; i < matches.size(); i++) {
        const Train& t = trains[matches[i]];
        cout << "  - Train #" << t.trainNo << " " << t.name << " (" << t.source << " -> " << t.destination
             << "), Departs: " << t.departure << ", Fare: Rs. " << t.fare << "\n";
    }
}

void DSAManager::displayCoachLayout() const {
    int tNo = readInt("Enter Train Number: ", 1000, 99999);
    int idx = findTrainIndex(tNo);
    if (idx != -1) {
        displaySeatMatrix(seatMap, idx, trains[idx].totalSeats, trains[idx].trainNo, trains[idx].name);
        cout << "Total Seats: " << trains[idx].totalSeats << " | Available: " << trains[idx].availableSeats << "\n";
    } else {
        cout << "Train #" << tNo << " not found.\n";
    }
}

void DSAManager::bookTicket() {
    int tNo = readInt("Enter Train Number to Book: ", 1000, 99999);
    int trainIdx = findTrainIndex(tNo);
    if (trainIdx == -1) {
        cout << "Train #" << tNo << " does not exist.\n";
        return;
    }

    Train& t = trains[trainIdx];
    string name = readNonEmptyString("Passenger Name: ");
    int age = readInt("Age (1-120): ", 1, 120);
    char gender = 'M';
    while (true) {
        string gStr = readNonEmptyString("Gender (M/F/O): ");
        char c = (char)toupper(gStr[0]);
        if (c == 'M' || c == 'F' || c == 'O') { gender = c; break; }
        cout << "Invalid gender. Please enter M, F, or O.\n";
    }

    Date d = readDate("Enter Travel Date:");

    // Duplicate booking check
    for (size_t i = 0; i < passengers.size(); i++) {
        if (passengers[i].trainNo == t.trainNo &&
            passengers[i].travelDate.day == d.day &&
            passengers[i].travelDate.month == d.month &&
            passengers[i].travelDate.year == d.year &&
            passengers[i].status == "CONFIRMED" &&
            toLowerCase(passengers[i].name) == toLowerCase(name)) {
            cout << "\nError: Passenger '" << name << "' is already booked on this train for this date.\n";
            return;
        }
    }

    string concession;
    float finalFare;
    calculateConcession(age, t.fare, concession, finalFare);

    // Case A: Confirmed booking
    if (t.availableSeats > 0) {
        int seatChoice = -1;
        cout << "\n1. Automatically assign first available seat\n"
             << "2. Choose specific seat from Coach Map\n";
        int mode = readInt("Choice (1-2): ", 1, 2);

        if (mode == 2) {
            displaySeatMatrix(seatMap, trainIdx, t.totalSeats, t.trainNo, t.name);
            while (true) {
                seatChoice = readInt("Enter desired seat number: ", 1, t.totalSeats);
                if (seatMap[trainIdx][seatChoice - 1] == 0) break;
                cout << "Seat #" << seatChoice << " is already occupied! Choose another.\n";
            }
        } else {
            seatChoice = findFirstFreeSeatInMatrix(seatMap, trainIdx, t.totalSeats);
        }

        Passenger p;
        p.pnr = generatePNR();
        p.name = name;
        p.age = age;
        p.gender = gender;
        p.trainNo = t.trainNo;
        p.seatNo = seatChoice;
        p.travelDate = d;
        p.status = "CONFIRMED";
        p.concession = concession;
        p.farePaid = finalFare;

        seatMap[trainIdx][seatChoice - 1] = 1;
        t.availableSeats--;
        passengers.push_back(p);

        dbInsertPassenger(p);
        dbUpdateTrainSeats(t.trainNo, t.availableSeats);
        exportTicketToFile(p, t);

        cout << "\n" << string(55, '=') << "\n"
             << "           BOOKING CONFIRMED (SEAT ALLOCATED)          \n"
             << string(55, '=') << "\n"
             << "  PNR Number      : " << p.pnr << "\n"
             << "  Passenger Name  : " << p.name << " (" << p.age << "/" << p.gender << ")\n"
             << "  Train           : #" << t.trainNo << " (" << t.name << ")\n"
             << "  Allocated Seat  : Coach C1, Seat #" << p.seatNo << "\n"
             << "  Concession      : " << p.concession << "\n"
             << "  Fare Paid       : Rs. " << fixed << setprecision(2) << p.farePaid << "\n"
             << "  E-Ticket Slip   : ticket_" << p.pnr << ".txt generated on disk.\n"
             << string(55, '=') << "\n";
    }
    // Case B: Waiting List (Queue ADT)
    else {
        if (waitingLists[t.trainNo].size() >= MAX_WAITING) {
            cout << "\nTrain #" << t.trainNo << " is fully booked and waiting list is full (" << MAX_WAITING << " max).\n";
            return;
        }

        WaitingEntry w;
        w.waitId = 0;
        w.name = name;
        w.age = age;
        w.gender = gender;
        w.trainNo = t.trainNo;
        w.travelDate = d;

        dbInsertWaiting(w);
        waitingLists[t.trainNo].enqueue(w);

        cout << "\n" << string(55, '=') << "\n"
             << "            ADDED TO WAITING LIST (FIFO QUEUE)         \n"
             << string(55, '=') << "\n"
             << "  Waitlist ID     : WL-" << w.waitId << "\n"
             << "  Queue Position  : Position #" << waitingLists[t.trainNo].size() << "\n"
             << "  Passenger       : " << w.name << " (" << w.age << "/" << w.gender << ")\n"
             << "  Train           : #" << t.trainNo << " (" << t.name << ")\n"
             << "  Notice          : Auto-promoted when a confirmed seat cancels.\n"
             << string(55, '=') << "\n";
    }
}

void DSAManager::cancelTicket() {
    int pnr = readInt("Enter PNR to cancel: ", 1000, 999999);
    int pIdx = findPassengerIndex(pnr);

    if (pIdx == -1) {
        cout << "PNR #" << pnr << " not found in system.\n";
        return;
    }

    Passenger& p = passengers[pIdx];
    if (p.status != "CONFIRMED") {
        cout << "PNR #" << pnr << " is already marked as " << p.status << ".\n";
        return;
    }

    int trainIdx = findTrainIndex(p.trainNo);
    if (trainIdx == -1) {
        cout << "Associated train not found.\n";
        return;
    }

    Train& t = trains[trainIdx];
    seatMap[trainIdx][p.seatNo - 1] = 0;
    p.status = "CANCELLED";
    dbUpdatePassengerStatus(p.pnr, "CANCELLED");

    // Module VIII: Push onto LIFO Stack for Undo
    recentCancellations.push(p);

    cout << "\n[Success] PNR #" << p.pnr << " (" << p.name << ") cancelled. Seat #" << p.seatNo << " freed.\n";

    // Module IX: Queue Auto-Promotion
    if (!waitingLists[t.trainNo].isEmpty()) {
        WaitingEntry promoted;
        waitingLists[t.trainNo].dequeue(promoted);
        dbDeleteWaiting(promoted.waitId);

        string conc;
        float fare;
        calculateConcession(promoted.age, t.fare, conc, fare);

        Passenger newP;
        newP.pnr = generatePNR();
        newP.name = promoted.name;
        newP.age = promoted.age;
        newP.gender = promoted.gender;
        newP.trainNo = t.trainNo;
        newP.seatNo = p.seatNo;
        newP.travelDate = promoted.travelDate;
        newP.status = "CONFIRMED";
        newP.concession = conc;
        newP.farePaid = fare;

        seatMap[trainIdx][p.seatNo - 1] = 1;
        passengers.push_back(newP);
        dbInsertPassenger(newP);
        exportTicketToFile(newP, t);

        cout << ">>> FIFO Auto-Promotion: Waitlist passenger '" << promoted.name
             << "' promoted to Coach C1, Seat #" << p.seatNo << "! (New PNR: " << newP.pnr << ")\n";
    } else {
        t.availableSeats++;
        dbUpdateTrainSeats(t.trainNo, t.availableSeats);
    }
}

void DSAManager::cancelWaitingEntry() {
    int waitId = readInt("Enter Waitlist ID (numeric): ", 1, 999999);
    bool found = false;

    for (map<int, ArrayQueue>::iterator it = waitingLists.begin(); it != waitingLists.end(); ++it) {
        WaitingEntry removed;
        if (it->second.removeById(waitId, removed)) {
            dbDeleteWaiting(waitId);
            cout << "\n[Removed] Waitlist entry WL-" << waitId << " for passenger '" << removed.name
                 << "' on Train #" << it->first << " successfully removed.\n";
            found = true;
            break;
        }
    }
    if (!found) cout << "Waitlist entry WL-" << waitId << " not found in active queues.\n";
}

void DSAManager::undoLastCancellation() {
    if (recentCancellations.isEmpty()) {
        cout << "\nNo recent cancellations available to undo (Stack is empty).\n";
        return;
    }

    Passenger last;
    recentCancellations.peek(last);

    int trainIdx = findTrainIndex(last.trainNo);
    if (trainIdx == -1) {
        cout << "Train #" << last.trainNo << " no longer exists.\n";
        return;
    }

    if (seatMap[trainIdx][last.seatNo - 1] != 0) {
        cout << "\nCannot undo cancellation: Seat #" << last.seatNo
             << " on Train #" << last.trainNo << " is already reoccupied!\n";
        return;
    }

    recentCancellations.pop(last);
    int pIdx = findPassengerIndex(last.pnr);
    if (pIdx != -1) {
        passengers[pIdx].status = "CONFIRMED";
        seatMap[trainIdx][last.seatNo - 1] = 1;
        trains[trainIdx].availableSeats--;

        dbUpdatePassengerStatus(last.pnr, "CONFIRMED");
        dbUpdateTrainSeats(last.trainNo, trains[trainIdx].availableSeats);
        exportTicketToFile(passengers[pIdx], trains[trainIdx]);

        cout << "\n" << string(55, '=') << "\n"
             << "            CANCELLATION UNDONE (RESTORED)             \n"
             << string(55, '=') << "\n"
             << "  Restored PNR    : " << last.pnr << "\n"
             << "  Passenger       : " << last.name << "\n"
             << "  Restored Seat   : Coach C1, Seat #" << last.seatNo << "\n"
             << "  Stack Status    : " << recentCancellations.size() << " cancellation(s) remaining in stack.\n"
             << string(55, '=') << "\n";
    }
}

void DSAManager::checkPNRStatus() const {
    int pnr = readInt("Enter PNR Number: ", 1000, 999999);
    int idx = findPassengerIndex(pnr);
    if (idx != -1) {
        const Passenger& p = passengers[idx];
        int tIdx = findTrainIndex(p.trainNo);
        string tName = (tIdx != -1) ? trains[tIdx].name : "Unknown Train";

        cout << "\n" << string(55, '=') << "\n"
             << "                    PNR STATUS ENQUIRY                 \n"
             << string(55, '=') << "\n"
             << "  PNR Number      : " << p.pnr << "\n"
             << "  Passenger Name  : " << p.name << "\n"
             << "  Age / Gender    : " << p.age << " / " << p.gender << "\n"
             << "  Train           : #" << p.trainNo << " (" << tName << ")\n"
             << "  Seat Number     : " << (p.status == "CONFIRMED" ? ("Coach C1, Seat #" + to_string(p.seatNo)) : "N/A") << "\n"
             << "  Booking Status  : " << p.status << "\n"
             << "  Concession      : " << p.concession << "\n"
             << "  Fare Paid       : Rs. " << fixed << setprecision(2) << p.farePaid << "\n"
             << "  Reversed Check  : " << reverseString(to_string(p.pnr)) << " (Module V String Reversal)\n"
             << string(55, '=') << "\n";
    } else {
        cout << "PNR #" << pnr << " not found in system.\n";
    }
}

void DSAManager::sortTrainsMenu() {
    if (trains.empty()) {
        cout << "No trains to sort.\n";
        return;
    }
    cout << "\n1. Sort by Fare (Lowest first - O(N^2) Bubble Sort)\n"
         << "2. Sort by Train Name (Alphabetical - O(N^2) Bubble Sort)\n";
    int opt = readInt("Select option (1-2): ", 1, 2);

    vector<Train> copy = trains;
    if (opt == 1) bubbleSortTrainsByFare(copy);
    else bubbleSortTrainsByName(copy);

    cout << "\n" << string(70, '=') << "\n"
         << "                       SORTED TRAIN DISPLAY                   \n"
         << string(70, '=') << "\n";
    for (size_t i = 0; i < copy.size(); i++) {
        cout << "Train #" << copy[i].trainNo << " " << left << setw(20) << copy[i].name
             << " Fare: Rs." << fixed << setprecision(0) << setw(6) << copy[i].fare
             << " Route: " << copy[i].source << " -> " << copy[i].destination << "\n";
    }
    cout << string(70, '=') << "\n";
}

void DSAManager::addTrain() {
    if ((int)trains.size() >= MAX_TRAINS) {
        cout << "Cannot add more trains. Maximum capacity (" << MAX_TRAINS << ") reached.\n";
        return;
    }
    int trainNo = readInt("Enter New Train Number (1000 - 99999): ", 1000, 99999);
    if (findTrainIndex(trainNo) != -1) {
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
    t.fare = readFloat("Base Fare (Rs 50 - 20000): ", 50.0f, 20000.0f);

    dbInsertTrain(t);
    insertTrainSorted(trains, t);
    uniqueStations.insert(t.source);
    uniqueStations.insert(t.destination);

    cout << "\n[Added] Train #" << t.trainNo << " (" << t.name << ") registered successfully in MongoDB collection.\n";
}

void DSAManager::displayUniqueStations() const {
    cout << "\n" << string(60, '=') << "\n"
         << "        UNIQUE NETWORK STATIONS (MODULE X: STL SET)    \n"
         << string(60, '=') << "\n";
    int idx = 1;
    for (set<string>::const_iterator it = uniqueStations.begin(); it != uniqueStations.end(); ++it) {
        cout << "  " << setw(2) << idx++ << ". " << *it << "\n";
    }

    cout << "\n--- Route Word Tokenization Demonstration (Module V) ---\n";
    for (size_t i = 0; i < trains.size(); i++) {
        string fullRoute = trains[i].source + " - " + trains[i].destination;
        vector<string> tokens = tokenizeRoute(fullRoute, '-');
        cout << "Train #" << trains[i].trainNo << " Route tokens (" << tokens.size() << "): ";
        for (size_t j = 0; j < tokens.size(); j++) {
            cout << "[" << tokens[j] << "] ";
        }
        cout << "\n";
    }
    cout << string(60, '=') << "\n";
}

void DSAManager::exportMongoScript() const {
    if (dbExportMongoScript("database/scripts/mongo_seed.js")) {
        cout << "\n" << string(65, '=') << "\n"
             << " MONGODB SEED SCRIPT GENERATED (database/scripts/mongo_seed.js) \n"
             << string(65, '=') << "\n"
             << "  File Location   : database/scripts/mongo_seed.js\n"
             << "  Target Database : " << dbGetDatabaseName() << "\n"
             << "  Collections     : trains, passengers, waiting_list\n"
             << "  Run in mongosh  : mongosh \"" << dbGetAtlasUri() << "\" database/scripts/mongo_seed.js\n"
             << string(65, '=') << "\n";
    } else {
        cout << "\n[Error] Failed to generate MongoDB script.\n";
    }
}

void DSAManager::syncWithAtlas() const {
    if (dbSyncToAtlas()) {
        cout << "\n[Success] Collections successfully pushed to MongoDB Atlas (datadb)!\n";
    } else {
        cout << "\n[Notice] Could not complete direct connection to Atlas.\n"
             << "Please make sure your IP is whitelisted on MongoDB Atlas:\n"
             << "1. Go to cloud.mongodb.com -> Network Access\n"
             << "2. Add IP Address -> 'Allow Access from Anywhere' (0.0.0.0/0)\n"
             << "Or run: database\\scripts\\sync_to_atlas.bat\n";
    }
}

