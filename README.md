# 🚆 Railway Ticket Reservation System

[![C++ Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg?logo=c%2B%2B)](https://isocpp.org/)
[![Database](https://img.shields.io/badge/Database-SQLite%203-003B57.svg?logo=sqlite)](https://www.sqlite.org/)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)]()
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()

A modular, menu-driven **C++11** backend application for managing railway reservations, seat layouts, cancellations, and waiting lists using core **Data Structures and Algorithms (DSA)** integrated with an **SQLite 3** relational database (`railway.db`).

---

## 📌 Table of Contents
- [Project Overview](#-project-overview)
- [Architecture & Data Flow](#-architecture--data-flow)
- [DSA Concepts Used](#-dsa-concepts-used)
- [Key Features](#-key-features)
- [Database Schema](#-database-schema)
- [Project Structure](#-project-structure)
- [Quick Start Guide](#-quick-start-guide)
- [Edge Case Demonstrations](#-edge-case-demonstrations)
- [Documentation & Viva Voce](#-documentation--viva-voce)
- [License](#-license)

---

## 📖 Project Overview
Designed as a first-year college DSA mini-project for **Group 4**, this application demonstrates the practical application of fundamental data structures (structures, 2D arrays, STL queues, vectors, maps) in a real-world scenario.

### Core Design Philosophy
- **Fast In-Memory Operations:** Active train models, 2D seat maps, and waiting queues operate entirely in RAM for high performance.
- **Relational Persistence:** Every transaction (add train, seat booking, ticket cancellation, auto-promotion) is immediately synchronized to SQLite (`railway.db`).
- **Complete Reconstitution:** On application launch, memory state (including the visual 2D seat layout and queue orders) is automatically rebuilt from the database.

---

## 🏛️ Architecture & Data Flow

```mermaid
flowchart TD
    User([User / Console]) -->|Interactive Menu| Main[main.cpp]
    Main -->|Menu Option Dispatch| Logic[railway.cpp]
    Logic -->|Input Sanitization| Utils[utils.cpp]
    Logic -->|Working Memory| Memory[In-Memory DSA Models\n- vector<Train>\n- 2D seatMap[20][60]\n- map<int, queue<WaitingEntry>>]
    Logic -->|Prepared Statements| DB[database.cpp]
    DB -->|SQL Queries| SQLite[(railway.db\nSQLite 3 File)]
```

---

## 💡 DSA Concepts Used

| DSA Concept | Source File | Real-World Application in System |
|---|---|---|
| **Structures (`struct`)** | `structures.h` | Groups heterogeneous properties for entities (`Train`, `Passenger`, `WaitingEntry`). |
| **Nested Structures** | `structures.h` | `struct Date` (day, month, year) nested inside passenger and queue records. |
| **2D Arrays** | `railway.cpp` | `seatMap[MAX_TRAINS][MAX_SEATS]` provides $O(1)$ random access to check/mark seat availability. |
| **1D Arrays** | `utils.cpp` | Static lookup array for calendar month days and leap year handling. |
| **FIFO Queue (`std::queue`)** | `railway.cpp` | Maintains waiting passengers in strict First-In, First-Out order for fair promotions. |
| **Map (`std::map`)** | `railway.cpp` | Associates each unique `trainNo` with its own isolated waiting queue. |
| **Dynamic Vector (`std::vector`)** | `railway.cpp` | Dynamic storage for trains and passengers with index-based operations. |
| **Binary Search** | `railway.cpp` | Achieves $O(\log N)$ fast lookup for trains by train number on a sorted vector. |
| **Linear Search** | `railway.cpp` | $O(N)$ scanning for partial destination matches and PNR lookups. |
| **Bubble Sort** | `railway.cpp` | Manual $O(N^2)$ sorting by fare or name on a copy vector for display. |

---

## ✨ Key Features

1. **Train Management:** Add new trains (inserted in sorted order), display formatted tables with departure times and fares.
2. **Search Operations:**
   - **Binary Search** by Train Number ($O(\log N)$).
   - **Linear Search** by Destination (case-insensitive substring matching).
3. **Smart Booking:**
   - Visual seat allocation using the 2D seat map grid.
   - Unique PNR generation starting from `1001`.
   - Rejection of duplicate bookings (same name + train + travel date).
4. **Waiting List System (FIFO Queue):**
   - Automatically enqueues passengers to `WL-1`, `WL-2`, etc., when seats are full.
5. **Cancellation & Auto-Promotion:**
   - Cancelling a ticket immediately frees the seat.
   - If passengers are waiting, the head of the queue (`front()`) is auto-promoted into the freed seat with a new confirmed PNR.
6. **Visual 2D Seat Map:**
   - Displays real-time seating grids (6 seats per row: `[ 1] [ 2] [XX] ...`).
7. **Robust Input Validation:**
   - Graceful recovery from non-numeric input (`cin.clear()` and `cin.ignore()`).
   - Leap-year aware date verification.

---

## 🗄️ Database Schema

The database uses a single portable file: `railway.db`.

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

## 📂 Project Structure

```text
railway-ticket-reservation-cpp/
├── .github/
│   └── workflows/
│       └── build.yml       # Automated GitHub Actions CI workflow
├── main.cpp                # Main menu driver and program loop
├── structures.h            # Data structures and system constants
├── utils.h / utils.cpp     # Input validation, sanitization, and string tools
├── database.h / database.cpp # SQLite API encapsulation with prepared statements
├── railway.h / railway.cpp # Train, booking, seat map, and queue logic
├── sqlite3.c / sqlite3.h   # SQLite 3 official amalgamation C source
├── Makefile                # Cross-platform Makefile (Linux, macOS, MinGW)
├── build.bat               # Windows one-click compilation script
├── test_workflow.ps1       # Automated end-to-end test script
├── array_queue_demo.cpp    # Bonus Module IX: Array-based circular queue
├── COMPLEXITY.md           # Asymptotic time/space complexity analysis
├── VIVA_QA.md              # 40+ curated viva voce questions & answers
├── LICENSE                 # MIT Open-Source License
└── README.md               # Project documentation
```

---

## 🚀 Quick Start Guide

### Prerequisites
- Any C++11 compliant compiler (`g++`, `clang++`, or MinGW).
- Standard C runtime library.

### Windows (Using `build.bat`)
```cmd
# 1. Clone the repository
git clone https://github.com/bharathwajverse/railway-ticket-reservation-cpp.git
cd railway-ticket-reservation-cpp

# 2. Build the project
build.bat

# 3. Run the application
railway.exe
```

### Linux / macOS (Using `Makefile`)
```bash
# 1. Clone the repository
git clone https://github.com/bharathwajverse/railway-ticket-reservation-cpp.git
cd railway-ticket-reservation-cpp

# 2. Compile using make
make

# 3. Launch application
./railway
```

---

## 🧪 Edge Case Demonstrations

1. **Full Train $\to$ Waiting Queue:**
   - Train `#10303` is pre-seeded with 3 seats. Booking 3 tickets fills the train. The 4th booking automatically transitions to `WL-1` in the train's queue.
2. **Cancellation $\to$ Auto-Promotion:**
   - Cancelling a confirmed ticket triggers `promoteFromWaitingList()`, promoting the waiting passenger to the newly freed seat with a new PNR in $O(1)$ queue time.
3. **Duplicate Ticket Prevention:**
   - Attempting to book the same passenger name on the same train and date is politely rejected.
4. **Resilience to Bad Input:**
   - Typing letters or symbols into numeric prompts (e.g., entering "abc" for Train Number) is safely intercepted without crashing.

---

## 📚 Documentation & Viva Voce

- **[COMPLEXITY.md](COMPLEXITY.md):** Complete asymptotic time and space complexity breakdown for all functions and algorithms.
- **[VIVA_QA.md](VIVA_QA.md):** 40+ structured viva questions and answers covering C++ basics, structures, 2D arrays, STL queues, prepared statements, and a 5-minute presentation script.

---

## 📄 License
This project is open-source under the [MIT License](LICENSE).
