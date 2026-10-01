# Railway Ticket Reservation System

A C++11 railway reservation project organized around the 10 DSA syllabus modules and a separate MongoDB document-data layer.

## Project layout

```text
.
├── app/
│   └── main.cpp                   # Console application entry point
├── DSA/
│   ├── include/dsa_manager.h       # Modules I–X declarations
│   ├── src/dsa_manager.cpp         # Modules I–X implementation
│   └── study_guide/                 # Private syllabus and viva notes
├── database/
│   ├── config/
│   │   └── mongodb.conf.example    # Copy to mongodb.conf for Atlas sync
│   ├── data/                       # MongoDB-style JSON collections
│   │   ├── trains.json
│   │   ├── passengers.json
│   │   └── waiting_list.json
│   ├── include/database.h          # Persistence interface
│   ├── scripts/
│   │   ├── mongo_seed.js           # Generated mongosh import script
│   │   └── sync_to_atlas.bat       # Atlas synchronization helper
│   └── src/database.cpp            # JSON document persistence
├── docs/                           # Architecture and API notes
├── web/                            # Optional browser prototype
├── build.bat                       # Windows build
├── run.bat                         # Windows build and run
└── Makefile                        # Linux/macOS build
```

All database material—including collections, configuration, scripts, headers, and implementation—is in `database/`. All DSA syllabus material is in `DSA/`.

## The 10 syllabus modules

| Module | Topic | Used in the project |
|---|---|---|
| I | C++ basics and I/O | Console input and output |
| II | Control statements | Menu loop and validation |
| III | 1D arrays | Calendar data and helper functions |
| IV | 2D arrays | Seat matrix |
| V | Strings | Route and text processing |
| VI | Structures | Train, passenger, date, and waiting records |
| VII | Algorithms | Linear search, binary search, and bubble sort |
| VIII | Stack | Cancellation undo |
| IX | Queue | FIFO waiting list |
| X | STL | `vector`, `set`, `map`, `deque`, and `pair` |

## Build and run

### Windows

```cmd
run.bat
```

To only build the executable:

```cmd
build.bat
```

### Linux/macOS

```bash
make
./railway
```

## Database and MongoDB Atlas

The program uses JSON files in `database/data/` as its local document collections. It does not require a MongoDB driver to compile or run.

For Atlas synchronization, copy `database/config/mongodb.conf.example` to `database/config/mongodb.conf`, set `MONGODB_URI`, then run:

```cmd
database\scripts\sync_to_atlas.bat
```

The local configuration file is ignored by Git. The application’s menu option 14 generates `database/scripts/mongo_seed.js`; option 13 sends it to Atlas.

## Optional web prototype

Open `web/index.html` in a browser to view the standalone interface mock-up. It is not connected to the C++ console program.
