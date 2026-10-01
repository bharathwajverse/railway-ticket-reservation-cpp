# API Specifications & Module Interfaces

This document outlines the programmatic C++ interface (`include/dsa_manager.h`, `include/database.h`) and data contracts for the **Railway Ticket Reservation System (MongoDB Edition)**.

---

## 1. C++ Engine API (`include/dsa_manager.h`)

The core business logic and algorithms are encapsulated within `DSAManager`:

```cpp
class DSAManager {
public:
    DSAManager();

    // Module I - IV: Display & Search Operations
    void displayTrains() const;
    void searchTrainByNumber(int trainNo) const;          // O(log N) Binary Search
    void searchTrainByDestination(const string& dest) const;// O(N) Linear Search
    void checkSeatAvailability(int trainNo) const;       // O(1) 2D Array inspection

    // Module V - VI: Booking & Cancellation
    void bookTicket(const string& name, int age, char gender, int trainNo, const Date& travelDate);
    void cancelConfirmedTicket(int pnr);                  // Auto-promotes queue
    void cancelWaitingTicket(int waitId);
    void undoLastCancellation();                          // O(1) Stack LIFO undo
    void viewPNRStatus(int pnr) const;

    // Module VII - X: Sorting & Utilities
    void sortTrainsByFare();                              // O(N^2) Bubble Sort
    void addNewTrain(const Train& train);
    void viewUniqueStations() const;                      // STL std::set
    void syncToMongoAtlas() const;                        // Atlas datadb push
    void exportMongoScript() const;                       // mongo_seed.js export
};
```

---

## 2. Document Persistence API (`include/database.h`)

The document layer isolates local JSON persistence and Atlas synchronization:

```cpp
// Lifecycle
bool dbOpen(const string& dataDir);
void dbClose();
void dbCreateCollections();

// Train Collection
bool dbInsertTrain(const Train& train);
bool dbLoadTrains(vector<Train>& trains);
bool dbUpdateTrainSeats(int trainNo, int availableSeats);

// Passenger Collection
bool dbInsertPassenger(const Passenger& p);
bool dbLoadPassengers(vector<Passenger>& passengers);
bool dbUpdatePassengerStatus(int pnr, const string& status);

// Waiting List Collection
bool dbInsertWaiting(const WaitingEntry& w);
bool dbLoadWaitingList(map<int, ArrayQueue>& waitingLists);
bool dbDeleteWaiting(int waitId);

// MongoDB Atlas Export & Sync
bool dbExportMongoSeed(const string& outputFile);
bool dbSyncToAtlas(const string& confFile);
```

---

## 3. Web UI JSON Contracts (`web/index.html`)

For the browser client preview, models conform to standard JSON contracts:

### `Train` Document
```json
{
  "_id": "6701a1b2c3d4e5f600000001",
  "train_no": 10101,
  "name": "Rajdhani Express",
  "source": "Delhi",
  "destination": "Mumbai",
  "departure": "06:00 AM",
  "total_seats": 4,
  "available_seats": 4,
  "fare": 1500.00
}
```

### `Passenger` Document
```json
{
  "_id": "6701a1b2c3d4e5f600000101",
  "pnr": 1001,
  "name": "John Doe",
  "age": 25,
  "gender": "M",
  "train_no": 10101,
  "seat_no": 1,
  "date": "15/10/2026",
  "status": "CONFIRMED",
  "concession": "NONE",
  "fare_paid": 1500.00
}
```

### `WaitingEntry` Document
```json
{
  "_id": "6701a1b2c3d4e5f600000201",
  "wait_id": 1,
  "passenger_name": "Alice Smith",
  "age": 30,
  "gender": "F",
  "train_no": 10101,
  "date": "15/10/2026"
}
```
