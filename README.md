# Railway Ticket Reservation System (Group 4)

A modular, menu-driven C++11 console application for managing train schedules, reservations, ticket cancellations, seat layouts, and waiting lists using core Data Structures and Algorithms (DSA) connected to an SQLite database (`railway.db`).

---

## 1. Project Overview
- **Team:** Group 4
- **Language:** C++11 (Procedural with `struct` and functions)
- **Database:** SQLite 3 (Amalgamation C API)
- **Design Philosophy:** In-memory DSA models for fast operations synchronized with relational storage for permanent persistence across restarts.

---

## 2. DSA Concept Mapping

| DSA Concept | Implementation Location | Purpose & Practical Use |
|---|---|---|
| **Structures (`struct`)** | `structures.h` (`Train`, `Passenger`, `WaitingEntry`, `Date`, `RailwaySystem`) | Groups heterogeneous attributes for each entity without OOP overhead. |
| **Nested Structures** | `structures.h` (`Date` inside `Passenger` & `WaitingEntry`) | Encapsulates calendar dates (day, month, year) cleanly. |
| **1D Arrays** | `utils.cpp` (`daysInMonth`), `railway.cpp` (`sampleList`) | Fast lookups for month day counts and initial seeding. |
| **2D Arrays** | `RailwaySystem::seatMap[MAX_TRAINS][MAX_SEATS]` | Provides $O(1)$ random-access seat availability tracking (`0` = free, `1` = booked). |
| **Strings** | `utils.cpp`, `railway.cpp` | Case-insensitive searching, substring pattern matching, and input sanitization. |
| **Binary Search** | `railway.cpp::binarySearchTrain()` | Achieves $O(\log N)$ fast lookup for trains by train number on sorted vector. |
| **Linear Search** | `railway.cpp::searchTrainByDestination()`, `findPassengerByPNR()` | $O(N)$ scanning for partial destination matches and PNR lookups. |
| **Bubble Sort** | `railway.cpp::sortTrains()` | $O(N^2)$ manual sorting by ticket fare or train name on a copy vector for display. |
| **FIFO Queue (`std::queue`)** | `RailwaySystem::waitingLists` | Maintains waiting lists in strict First-Come, First-Served order. |
| **Associative Map (`std::map`)** | `RailwaySystem::waitingLists` | Maps `trainNo` $\to$ `queue<WaitingEntry>` for per-train queue isolation. |
| **Dynamic Vector (`std::vector`)** | `RailwaySystem::trains`, `passengers` | Dynamic list management with index-based access. |

---

## 3. Database Schema

The database uses a single file: `railway.db`.

```sql
-- 1. Trains Table
CREATE TABLE IF NOT EXISTS trains (
  train_no INTEGER PRIMARY KEY,
  name TEXT NOT NULL,
  source TEXT NOT NULL,
  destination TEXT NOT NULL,
  departure TEXT NOT NULL,
  total_seats INTEGER NOT NULL,
  available_seats INTEGER NOT NULL,
  fare REAL NOT NULL
);

-- 2. Passengers Table
CREATE TABLE IF NOT EXISTS passengers (
  pnr INTEGER PRIMARY KEY,
  name TEXT NOT NULL,
  age INTEGER NOT NULL,
  gender TEXT NOT NULL,
  train_no INTEGER NOT NULL,
  seat_no INTEGER NOT NULL,
  day INTEGER NOT NULL,
  month INTEGER NOT NULL,
  year INTEGER NOT NULL,
  status TEXT NOT NULL
);

-- 3. Waiting List Table
CREATE TABLE IF NOT EXISTS waiting_list (
  wait_id INTEGER PRIMARY KEY AUTOINCREMENT,
  name TEXT NOT NULL,
  age INTEGER NOT NULL,
  gender TEXT NOT NULL,
  train_no INTEGER NOT NULL,
  day INTEGER NOT NULL,
  month INTEGER NOT NULL,
  year INTEGER NOT NULL
);
```

---

## 4. Folder Structure

```
RailwayReservation/
├── main.cpp          # Main menu loop and entry point
├── structures.h      # Core structs and capacity constants
├── utils.h           # Input validation and string helper declarations
├── utils.cpp         # Input validation and string helper implementations
├── database.h        # SQLite database interface declarations
├── database.cpp      # SQLite database implementations (prepared statements)
├── railway.h         # Railway business logic prototypes
├── railway.cpp       # Train, booking, seat map, and queue operations
├── sqlite3.c         # SQLite 3 official amalgamation C file
├── sqlite3.h         # SQLite 3 official header
├── sqlite3.o         # Compiled object file for rapid linking
├── build.bat         # Single-command build script for Windows
├── railway.exe       # Generated executable
├── railway.db        # SQLite database file
├── COMPLEXITY.md     # In-depth asymptotic complexity analysis
├── VIVA_QA.md        # 40+ viva questions, answers, and 5-minute demo script
└── README.md         # Project documentation and guide
```

---

## 5. How to Build and Run

### On Windows (Included w64devkit MinGW)
Simply run the included batch file:
```cmd
build.bat
railway.exe
```

Or run manual compilation:
```cmd
gcc -O2 -c sqlite3.c -o sqlite3.o
g++ -std=c++11 -Wall -Wextra main.cpp utils.cpp database.cpp railway.cpp sqlite3.o -o railway.exe
railway.exe
```

### On Linux / macOS
```bash
gcc -O2 -c sqlite3.c -o sqlite3.o
g++ -std=c++11 -Wall -Wextra main.cpp utils.cpp database.cpp railway.cpp sqlite3.o -o railway -lpthread -ldl
./railway
```

---

## 6. Demonstrated Edge Cases

1. **Full Train Booking $\to$ Automatic Waiting List:**
   - Attempting to book a ticket when `availableSeats == 0` automatically enqueues the passenger at `WL-1` and stores the record in `waiting_list`.
2. **Ticket Cancellation $\to$ Automatic FIFO Promotion:**
   - Cancelling a confirmed ticket on a train with waiting passengers instantly dequeues (`pop()`) the first waiting passenger, assigns them the freed seat, allocates a new PNR, and marks them `CONFIRMED`.
3. **Duplicate Prevention:**
   - Rejects duplicate train numbers upon creation.
   - Rejects duplicate passenger bookings with identical name and travel date on the same train.
4. **Input Sanitization:**
   - Robustly handles non-numeric inputs for numbers/dates without crashing or looping (`cin.clear()` + `cin.ignore()`).

---

## 7. Team Contribution Table (Placeholders)

| Member Name | Roll Number | Module / Responsibilities |
|---|---|---|
| Member 1 | ________________ | Project Scaffolding, `structures.h`, Input Validation (`utils.cpp`) |
| Member 2 | ________________ | SQLite Database Integration & Prepared Statements (`database.cpp`) |
| Member 3 | ________________ | Train Operations, Binary Search & Bubble Sort (`railway.cpp`) |
| Member 4 | ________________ | Booking, 2D Seat Map, Cancellation & Waiting Queue Promotion |
