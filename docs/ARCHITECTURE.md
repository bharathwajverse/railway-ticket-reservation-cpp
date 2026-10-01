# System Architecture & Technical Design

This document details the modular architectural design, data structures, folder organization, and data persistence model of the **Railway Ticket Reservation System (MongoDB Edition)**.

---

## 1. High-Level Architectural Overview

The system follows a clean modular three-tier architecture with zero external compilation dependencies:

```
┌─────────────────────────────────────────────────────────────┐
│                       Client Layer                          │
│   ┌───────────────────────────┐ ┌───────────────────────┐   │
│   │ Terminal Kiosk (Console)  │ │ Web Dashboard (HTML5) │   │
│   │     src/main.cpp          │ │    web/index.html     │   │
│   └─────────────┬─────────────┘ └───────────────────────┘   │
└─────────────────┼───────────────────────────────────────────┘
                  │
┌─────────────────┼───────────────────────────────────────────┐
│                 ▼                                           │
│   ┌─────────────────────────────────────────────────────┐   │
│   │          DSA Engine (10 Syllabus Modules)           │   │
│   │           include/dsa_manager.h                     │   │
│   │           src/dsa_manager.cpp                       │   │
│   │                                                     │   │
│   │  • Module I:   C++ Streams & Type Casting           │   │
│   │  • Module II:  Control Statements (Loop/Decision)   │   │
│   │  • Module III: 1D Numeric Arrays                    │   │
│   │  • Module IV:  2D Coach Seat Matrix                 │   │
│   │  • Module V:   String Traversal & Reversal          │   │
│   │  • Module VI:  Heterogeneous Structures             │   │
│   │  • Module VII: Time/Space Complexity & Big-O        │   │
│   │  • Module VIII: Stack ADT (Array-based LIFO Undo)   │   │
│   │  • Module IX:  Queue ADT (Array-based Circular FIFO)│   │
│   │  • Module X:   STL Containers (vector, set, pair)   │   │
│   └─────────────────────────┬───────────────────────────┘   │
└─────────────────────────────┼───────────────────────────────┘
                              │
┌─────────────────────────────┼───────────────────────────────┐
│                             ▼                               │
│              ┌─────────────────────────────┐                │
│              │ Document Persistence Layer  │                │
│              │ include/database.h          │                │
│              │ src/database.cpp            │                │
│              └──────────────┬──────────────┘                │
│                             │                               │
│              ┌──────────────┴──────────────┐                │
│              ▼                             ▼                │
│   ┌─────────────────────┐      ┌────────────────────────┐   │
│   │ Local Collections   │      │ MongoDB Atlas Cloud    │   │
│   │ (mongodb_data/)     │      │ (cluster0 / datadb)    │   │
│   │ • trains.json       │      │ • mongo_seed.js        │   │
│   │ • passengers.json   │      │ • sync_to_atlas.bat    │   │
│   │ • waiting_list.json │      │                        │   │
│   └─────────────────────┘      └────────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

---

## 2. Directory Structure

```text
railway-ticket-reservation-cpp/
├── include/                       # Public Header Declarations
│   ├── database.h                 # MongoDB document persistence interfaces
│   └── dsa_manager.h              # 10 Syllabus Modules & DSAManager class
├── src/                           # Source Implementations
│   ├── database.cpp               # MongoDB JSON document file I/O & Atlas export
│   ├── dsa_manager.cpp            # All 10 DSA Syllabus modules implementation
│   └── main.cpp                   # Clean CLI entry-point driver
├── mongodb_data/                  # Local MongoDB JSON Document Collections
│   ├── trains.json                # Trains document collection
│   ├── passengers.json            # Passengers document collection
│   └── waiting_list.json          # Waiting queue document collection
├── scripts/
│   └── sync_to_atlas.bat          # Batch utility to push mongo_seed.js via mongosh
├── docs/                          # Architecture and API documentation
│   ├── API.md                     # Data structures and function interfaces
│   └── ARCHITECTURE.md            # System architecture and data persistence flow
├── web/
│   └── index.html                 # Responsive Web Dashboard preview
├── build.bat                      # Windows build script (-Iinclude src/*.cpp)
├── run.bat                        # One-click Windows compile & launcher
├── sync_to_atlas.bat              # Quick Atlas sync launcher
├── Makefile                       # Multi-platform Makefile for Linux/macOS/Windows
├── mongo_seed.js                  # Auto-generated mongosh import script
└── README.md                      # Comprehensive project guide
```

---

## 3. Data Structures & Algorithmic Complexity

| Entity / Operation | Data Structure / Module | Algorithmic Design | Time Complexity | Space Complexity |
|---|---|---|---|---|
| **Train Schedules** | `std::vector<Train>` (Module X) | Contiguous memory, cache-friendly indexing | Lookup: $O(\log N)$ by No., $O(N)$ linear | $O(N)$ |
| **Train Number Search** | Binary Search (Module VII) | Divide and conquer over sorted train list | $O(\log N)$ | $O(1)$ |
| **Train Destination Search** | Linear Search (Module VII) | Sequential search comparing destination string | $O(N)$ | $O(1)$ |
| **Seat Map Layout** | 2D Array `seatMap[20][60]` (Module IV) | Row = train index, Col = seat index | $O(1)$ allocation / lookup | $O(T \times S)$ |
| **Cancellation Undo** | Custom `ArrayStack` (Module VIII) | LIFO Stack ADT backed by static 1D array | Push: $O(1)$, Pop: $O(1)$ | $O(\text{Capacity})$ |
| **Waiting List Queue** | Custom `ArrayQueue` (Module IX) | Circular Queue ADT backed by 1D array | Enqueue: $O(1)$, Dequeue: $O(1)$ | $O(\text{Capacity})$ |
| **Unique Route Stations** | `std::set<string>` (Module X) | Self-balancing Red-Black binary search tree | Insertion: $O(\log K)$, Traversal: $O(K)$ | $O(K)$ |
| **Train Sorting** | Bubble Sort (Module VII) | Comparison-based adjacent swap algorithm | Best: $O(N)$, Worst: $O(N^2)$ | $O(1)$ |

---

## 4. MongoDB Persistence & Cloud Synchronization

### Document Architecture
1. **Zero External Dependencies:** Built using standard C++11 `<fstream>`, `<sstream>`, `<iomanip>` to generate MongoDB-compliant JSON documents with standard 24-character hexadecimal `_id` ObjectIds.
2. **Local Document Collections (`mongodb_data/`):**
   - `trains.json`: Array of Train documents.
   - `passengers.json`: Array of confirmed Passenger documents.
   - `waiting_list.json`: Array of waiting list ticket records.
3. **Automated `mongo_seed.js` Generation:**
   - Option 14 (or automated sync) generates a self-contained JavaScript script containing `use datadb;`, collection drops, and `insertMany([...])` commands.
4. **Cloud Atlas Sync (`sync_to_atlas.bat`):**
   - Single-click sync utility connects via `mongosh` to MongoDB Atlas cluster `cluster0.xhjfpv2.mongodb.net` targeting database `datadb`.
   - Credentials configured in `mongodb.conf` (gitignored for safety).
