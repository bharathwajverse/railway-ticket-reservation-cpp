#ifndef DSA_MANAGER_H
#define DSA_MANAGER_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <stack>
#include <deque>
#include <utility>


// ============================================================================
// MODULE I: INTRODUCTION TO C++ PROGRAMMING
// Tokens, Constants, Types, Operators, Stream I/O Manipulators, Type Conversion
// ============================================================================
const int MAX_TRAINS = 20;
const int MAX_SEATS = 60;
const int MAX_WAITING = 10;
const int MAX_STACK = 50;
const int MAX_QUEUE = 20;

// ============================================================================
// MODULE VI: STRUCTURES
// Need for structures, declaration, initialization, member access, nested structs
// ============================================================================
struct Date {
    int day;
    int month;
    int year;
};

struct Train {
    int trainNo;
    std::string name;
    std::string source;
    std::string destination;
    std::string departure;
    int totalSeats;
    int availableSeats;
    float fare;
};

struct Passenger {
    int pnr;
    std::string name;
    int age;
    char gender;
    int trainNo;
    int seatNo;
    Date travelDate;
    std::string status;
    std::string concession;
    float farePaid;
};

struct WaitingEntry {
    int waitId;
    std::string name;
    int age;
    char gender;
    int trainNo;
    Date travelDate;
};

// ============================================================================
// MODULE II: CONTROL STATEMENTS - UTILITY FUNCTIONS
// Decision making (if-else, switch), loops (for, while), functions (pass by ref)
// ============================================================================
bool isLeapYear(int y);
bool isValidDate(int d, int m, int y);
void calculateConcession(int age, float baseFare, std::string& concession, float& finalFare);
int readInt(const std::string& prompt, int minVal, int maxVal);
std::string readNonEmptyString(const std::string& prompt);
Date readDate(const std::string& prompt);

// ============================================================================
// MODULE III: ARRAYS – 1D
// 1D numeric arrays, passing arrays to functions, array operations
// ============================================================================
int sum1DArray(const int arr[], int size);
void reset1DArray(int arr[], int size, int defaultVal);

// ============================================================================
// MODULE IV: ARRAYS – 2D
// 2D numeric arrays, matrix operations (clear, display, free seat search)
// ============================================================================
void initSeatMatrix(int matrix[MAX_TRAINS][MAX_SEATS]);
void displaySeatMatrix(const int matrix[MAX_TRAINS][MAX_SEATS], int trainIdx, int totalSeats, int trainNo, const std::string& trainName);
int findFirstFreeSeatInMatrix(const int matrix[MAX_TRAINS][MAX_SEATS], int trainIdx, int totalSeats);
int countOccupiedInMatrix(const int matrix[MAX_TRAINS][MAX_SEATS], int trainIdx, int totalSeats);

// ============================================================================
// MODULE V: STRING ARRAYS & MANIPULATION
// Built-in methods, character frequency, traversal, reversal, tokenization, matching
// ============================================================================
std::string toLowerCase(std::string s);
bool containsIgnoreCase(const std::string& text, const std::string& pattern);
int countCharFrequency(const std::string& str, char ch);
std::string reverseString(const std::string& str);
std::vector<std::string> tokenizeRoute(const std::string& routeStr, char delimiter);

// ============================================================================
// MODULE VII: DATA STRUCTURES PERFORMANCE ANALYSIS
// Asymptotic complexity analysis: O(1) Matrix, O(log N) Binary, O(N) Linear, O(N^2) Bubble
// ============================================================================
int binarySearchTrain(const std::vector<Train>& trains, int trainNo);
std::vector<int> linearSearchByDestination(const std::vector<Train>& trains, const std::string& dest);
void bubbleSortTrainsByFare(std::vector<Train>& trains);
void bubbleSortTrainsByName(std::vector<Train>& trains);
void insertTrainSorted(std::vector<Train>& trains, const Train& t);

// ============================================================================
// MODULE VIII: STACKS (STACK IMPLEMENTATION USING ARRAYS)
// Stack ADT, push, pop, peek, isEmpty, isFull using static 1D array
// ============================================================================
struct ArrayStack {
    Passenger data[MAX_STACK];
    int topIndex;

    ArrayStack();
    bool push(const Passenger& p);
    bool pop(Passenger& out);
    bool peek(Passenger& out) const;
    bool isEmpty() const;
    bool isFull() const;
    int size() const;
};

// ============================================================================
// MODULE IX: QUEUES (QUEUES IMPLEMENTATION USING ARRAYS)
// Queue ADT, enqueue, dequeue, peek, isEmpty, isFull using Circular 1D array
// ============================================================================
struct ArrayQueue {
    WaitingEntry data[MAX_QUEUE];
    int frontIndex;
    int rearIndex;
    int count;

    ArrayQueue();
    bool enqueue(const WaitingEntry& w);
    bool dequeue(WaitingEntry& out);
    bool peek(WaitingEntry& out) const;
    bool isEmpty() const;
    bool isFull() const;
    int size() const;
    bool removeById(int waitId, WaitingEntry& out);
};

// ============================================================================
// MODULE X: STL FUNDAMENTALS & CONTAINERS + ORGANIZED DSA MANAGER
// Pairs, vectors, iterators, deque, sets, maps, coordinating all 10 modules
// ============================================================================
class DSAManager {
private:
    std::vector<Train> trains;                         // STL Vector (dynamic array)
    std::vector<Passenger> passengers;                 // STL Vector (dynamic array)
    std::map<int, ArrayQueue> waitingLists;            // STL Map associating trainNo -> ArrayQueue
    std::set<std::string> uniqueStations;              // STL Set (unique network stations)
    std::deque<std::string> auditLog;                  // STL Deque (event history)
    int seatMap[MAX_TRAINS][MAX_SEATS];                // Module IV: 2D Numeric Matrix
    ArrayStack recentCancellations;                    // Module VIII: Array-based Stack

public:
    DSAManager();

    // Data Synchronization
    void loadFromDatabase();
    void rebuildSeatMap();
    int getTrainCount() const;

    // Helper lookups
    int findTrainIndex(int trainNo) const;
    int findPassengerIndex(int pnr) const;
    int generatePNR() const;
    bool exportTicketToFile(const Passenger& p, const Train& t) const;

    // Core DSA Operations
    void displayTrains() const;
    void searchTrainByNumber() const;
    void searchTrainByDestination() const;
    void displayCoachLayout() const;
    void bookTicket();
    void cancelTicket();
    void cancelWaitingEntry();
    void undoLastCancellation();
    void checkPNRStatus() const;
    void sortTrainsMenu();
    void addTrain();
    void displayUniqueStations() const;
    void exportMongoScript() const;
    void syncWithAtlas() const;

    // STL Pair demonstration
    std::pair<int, std::string> getTrainSummary(int trainNo) const;
};

#endif // DSA_MANAGER_H
