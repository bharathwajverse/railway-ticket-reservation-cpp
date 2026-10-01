# Architecture

The program has three small, independent parts:

```text
app/main.cpp
    │
    ├── DSA/                       # The 10 syllabus modules
    │   ├── include/dsa_manager.h
    │   └── src/dsa_manager.cpp
    │
    └── database/                  # Complete document-database layer
        ├── include/database.h
        ├── src/database.cpp
        ├── data/*.json
        ├── config/mongodb.conf
        └── scripts/
```

`app/main.cpp` only starts the program and dispatches menu choices. `DSAManager` contains the reservation rules and each of the 10 DSA examples. The database layer reads and writes the three JSON document collections and can create a `mongosh` import script.

## Data flow

1. The application opens `database/data/`.
2. `DSAManager` loads trains, passengers, and the waiting list.
3. A booking or cancellation updates the in-memory DSA structures.
4. The database layer saves the relevant JSON collection.
5. Atlas synchronization exports `database/scripts/mongo_seed.js` and runs it with the URI from `database/config/mongodb.conf`.

## Collections

| File | Collection | Purpose |
|---|---|---|
| `database/data/trains.json` | `trains` | Train schedule, fare, and seat counts |
| `database/data/passengers.json` | `passengers` | Confirmed and cancelled tickets |
| `database/data/waiting_list.json` | `waiting_list` | FIFO waiting entries |

## Build boundary

The project uses only the C++11 standard library. Build scripts compile:

```text
app/main.cpp
DSA/src/dsa_manager.cpp
database/src/database.cpp
```

with the include folders `DSA/include` and `database/include`.
