# 🚆 Railway Ticket Reservation System v2
### Enterprise C++ Backend with MongoDB NoSQL Architecture, REST API & Web Dashboard

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg?style=flat&logo=c%2B%2B)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![MongoDB](https://img.shields.io/badge/Database-MongoDB%20NoSQL-green.svg?style=flat&logo=mongodb)](https://www.mongodb.com/)
[![REST API](https://img.shields.io/badge/Architecture-REST%20API-orange.svg?style=flat)](http://localhost:8080)
[![Zero Dependency](https://img.shields.io/badge/Build-Zero%20External%20Install-brightgreen.svg?style=flat)](#quick-start)

> **College Project: Group 4 – B.Tech CSE (AI & ML)**  
> Built strictly following the academic C++ & Data Structures syllabus while offering full enterprise-grade NoSQL database integration, REST API server, and responsive web frontend.

---

## 🏛️ System Architecture

```
                  +----------------------------------------------+
                  |         Client Layer (User Interfaces)       |
                  |                                              |
                  |  [ Console CLI Menu ]     [ Web Dashboard ]  |
                  |   (src/main.cpp)         (frontend/index.html)|
                  +-----------+----------------------+-----------+
                              |                      | (HTTP JSON)
                              |                      v
                              |          +-----------------------+
                              |          | REST API Web Server   |
                              |          | (backend/api/server)  |
                              |          +-----------+-----------+
                              |                      |
                              v                      v
                  +----------------------------------------------+
                  |        Core DSA Engine (Pure Logic Zone)     |
                  |    * 100% Syllabus-Compliant Pure C++ *      |
                  |    * No cin/cout, Returns Clean Results *    |
                  |                                              |
                  |  - train_ops.cpp     : Binary & Linear Search|
                  |  - booking_ops.cpp   : Booking & Promotion   |
                  |  - seat_map.cpp      : 2D Array Coach Matrix |
                  |  - waiting_queue.cpp : FIFO Queue Management |
                  |  - validation.cpp    : String & Date Helpers |
                  +----------------------+-----------------------+
                                         |
                                         v
                  +----------------------------------------------+
                  |     Persistence Layer (backend/database)     |
                  |                                              |
                  |  - train_repo.cpp     -> trains.json         |
                  |  - passenger_repo.cpp -> passengers.json     |
                  |  - waiting_repo.cpp   -> waiting_list.json   |
                  |  - mongosh Script     -> MongoDB Atlas Cloud |
                  +----------------------------------------------+
```

---

## 📂 Project Directory Structure

```
project_cpp/
├── backend/
│   ├── api/                    -> REST API server & routes (Extra feature)
│   │   ├── server.cpp          -> Web server main(), static file mount, port 8080
│   │   ├── routes.h            -> Route prototypes
│   │   └── routes.cpp          -> Handlers for 8 endpoints connecting to DSA engine
│   ├── database/               -> Persistence layer (NoSQL documents)
│   │   ├── db_connection.h/.cpp-> Directory connection and file verification
│   │   ├── train_repo.h/.cpp   -> Insert, load, and update trains
│   │   ├── passenger_repo.h/.cpp -> Insert, load, and update passenger status
│   │   └── waiting_repo.h/.cpp -> Insert, load, and delete waiting list entries
│   ├── dsa/                    -> Pure C++ DSA Logic (NO I/O, NO library code)
│   │   ├── train_ops.h/.cpp    -> Binary search, linear search, bubble sort
│   │   ├── booking_ops.h/.cpp  -> PNR generator, booking, cancel, auto-promotion
│   │   ├── seat_map.h/.cpp     -> 2D array coach matrix helpers
│   │   ├── waiting_queue.h/.cpp-> Queue ADT helpers (FIFO enqueue/dequeue)
│   │   └── validation.h/.cpp   -> Leap-year date, string traversal, tokenization
│   ├── include/                -> Shared header definitions
│   │   ├── structures.h        -> Train, Passenger, WaitingEntry, Date, RailwaySystem
│   │   ├── config.h            -> MAX_TRAINS (20), MAX_SEATS (60), MAX_WAITING (10)
│   │   └── third_party/        -> httplib.h, json.hpp (Single-header libraries)
│   ├── src/                    -> Console Application (User Interface)
│   │   ├── main.cpp            -> Startup, DB load, sample seeding, menu loop
│   │   ├── menu.h              -> Console I/O prototypes
│   │   └── menu.cpp            -> printMenu(), handleChoice(), formatted printouts
│   └── CMakeLists.txt          -> Builds railway_cli and railway_api
├── frontend/                   -> Responsive Web Interface (HTML5/CSS3/Vanilla JS)
│   ├── index.html              -> Single Page Application structure with 6 tabs
│   ├── css/style.css           -> Light/Dark themes, 2D coach seat layout, e-ticket styles
│   └── js/
│       ├── api.js              -> REST API client (clean fetch wrapper)
│       └── app.js              -> Reactive UI logic, seat rendering, modal dialogs
├── scripts/                    -> Automation scripts
│   ├── build.ps1               -> One-click compilation on Windows (g++)
│   ├── run_cli.ps1             -> Starts console CLI application
│   ├── run_api.ps1             -> Starts REST server + Web dashboard
│   └── seed_sample_data.js     -> mongosh script for MongoDB Atlas seeding
├── .dockerignore
├── .env.example                -> Sample environment variables
├── .gitignore                  -> Ignores binaries, build artifacts, and .env
├── Dockerfile                  -> Multi-stage production container build
└── README.md                   -> Project documentation & viva guide
```

---

## 📚 Mapping to College DSA Syllabus (Modules I to X)

Every module required by the university curriculum is implemented in clean, structured C++:

| Module | Syllabus Topic | File & Implementation in Project | Time Complexity |
|---|---|---|---|
| **Module I** | Tokens, Constants, Types | `include/config.h`, `include/structures.h`: `MAX_TRAINS`, `MAX_SEATS`, `float fare`, formatted stream manipulators | $O(1)$ |
| **Module II** | Control Statements & Functions | `dsa/validation.cpp`: `isLeapYear()`, `calculateConcession()` (child 50%, senior 40%), pass-by-reference | $O(1)$ |
| **Module III** | Arrays – 1D | `dsa/validation.cpp`: `daysPerMonth[12]` static array for calendar validation; search result index arrays | $O(1)$ |
| **Module IV** | Arrays – 2D Matrix | `dsa/seat_map.cpp`: `int seatMap[MAX_TRAINS][MAX_SEATS]` matrix representing coach seat occupancy (`0`=Free, `1`=Booked) | $O(\text{seats})$ |
| **Module V** | String Arrays & Manipulation | `dsa/validation.cpp`: `toLowerCase()`, `containsIgnoreCase()`, `countCharFrequency()`, `reverseString()`, `tokenizeString()` | $O(N)$ |
| **Module VI** | Structures | `include/structures.h`: `struct Date`, `struct Train`, `struct Passenger`, `struct WaitingEntry`, `struct RailwaySystem` | $O(1)$ |
| **Module VII** | Algorithm Performance Analysis | `dsa/train_ops.cpp`: Binary Search by train number ($O(\log N)$), Linear Search by destination ($O(N)$), Bubble Sort ($O(N^2)$) | $O(\log N)$ / $O(N^2)$ |
| **Module VIII** | Stacks | `include/structures.h`, `dsa/booking_ops.cpp`: LIFO cancellation history tracking and ticket rollback | $O(1)$ |
| **Module IX** | Queues | `dsa/waiting_queue.cpp`: FIFO Waiting List queue ADT. Auto-promotes first waiting passenger upon ticket cancellation | $O(1)$ push/pop |
| **Module X** | STL Containers | `include/structures.h`: `std::vector` (dynamic array), `std::map` (trainNo -> queue), `std::queue` (FIFO), `std::pair` | Dynamic |

---

## ⚡ Quick Start & Execution

### 1. Build Binaries (One-Click)
Open PowerShell in the project root:
```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1
```
This compiles both `backend/railway_cli.exe` and `backend/railway_api.exe`.

### 2. Run the Console CLI (Menu-Driven)
```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run_cli.ps1
```
Select menu options (1 to 9):
- `1` : Add New Train
- `2` : Display All Trains
- `3` : Search Train (Binary Search by Number or Linear Search by Destination)
- `4` : Book Ticket (Allocates seat or queues to FIFO waiting list)
- `5` : Cancel Ticket (Frees seat and **auto-promotes** waiting passenger)
- `6` : Check Coach Seat Layout (**2D Array Matrix Grid**)
- `7` : Display Passenger Details
- `8` : Display Waiting List
- `9` : Sort Trains (Bubble Sort by Fare or Name)

### 3. Run the Web Dashboard & REST API
```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\run_api.ps1
```
Open your browser at: **[http://localhost:8080](http://localhost:8080)**

---

## 🌐 REST API Endpoints Specification

All API responses follow the standard contract:
```json
{
  "success": true,
  "data": { ... },
  "error": ""
}
```

| Method | Endpoint | Description | Query / Body Params |
|---|---|---|---|
| `GET` | `/api/trains` | List all trains | `?sort=fare` or `?sort=name` |
| `GET` | `/api/trains/search` | Search trains | `?number=10101` (Binary) or `?destination=Mumbai` (Linear) |
| `POST` | `/api/trains` | Add new train | `{ "trainNo": 10505, "name": "...", "totalSeats": 4, "fare": 1200 }` |
| `GET` | `/api/trains/:no/seats` | 2D seat occupancy matrix | Returns seat array `[1, 0, 1, ...]` and occupancy statistics |
| `POST` | `/api/bookings` | Book ticket or join queue | `{ "trainNo": 10101, "name": "...", "age": 25, "gender": "M", "travelDate": {...} }` |
| `DELETE` | `/api/bookings/:pnr` | Cancel confirmed ticket | Returns cancellation status and auto-promoted passenger details |
| `GET` | `/api/passengers` | Query passenger records | Optional `?pnr=1001` or `?trainNo=10101` |
| `GET` | `/api/waiting` | Live FIFO waiting queues | Returns per-train waiting queues in FIFO order |

---

## 🍃 MongoDB Cloud Integration

### Database Design: `datadb`
| Collection | Primary Key / Index | Fields |
|---|---|---|
| `trains` | `trainNo` (Unique) | `trainNo`, `name`, `source`, `destination`, `departure`, `totalSeats`, `availableSeats`, `fare` |
| `passengers` | `pnr` (Unique) | `pnr`, `name`, `age`, `gender`, `trainNo`, `seatNo`, `travelDate`, `status` |
| `waiting_list` | `waitId` (Unique) | `waitId`, `name`, `age`, `gender`, `trainNo`, `travelDate` |

To seed sample data directly to MongoDB Atlas:
```powershell
mongosh "mongodb+srv://system:system@cluster0.xhjfpv2.mongodb.net/datadb?appName=Cluster0" scripts/seed_sample_data.js
```

---

## 🐳 Docker Container Execution

To build and run the multi-stage Docker container:
```bash
docker build -t railway-app .
docker run -p 8080:8080 railway-app
```
Navigate to `http://localhost:8080` in your browser.

---

## 🎓 Viva Questions & Answers (Exam Preparation)

### Part A: C++ & DSA Questions
1. **Q: Why did you separate `dsa/` from `src/` and `api/`?**  
   *A:* Separation of Concerns. The core DSA logic functions in `dsa/` are pure algorithms that take parameters and return data structures. They do not contain `cin` or `cout`. This enables the exact same algorithms to be shared between the console CLI and the REST web server without duplicate code.

2. **Q: How does the waiting list queue work?**  
   *A:* We use a First-In-First-Out (FIFO) Queue ADT. When all coach seats are occupied, incoming passengers are enqueued. When any confirmed passenger cancels their ticket, the passenger at the `front()` of the queue is dequeued (`pop()`) and automatically promoted into the freed seat with a new PNR.

3. **Q: Why is Binary Search used for train number search, and what is its complexity?**  
   *A:* Trains in the system are kept strictly sorted by `trainNo`. Binary search divides the search space in half at each iteration, achieving $O(\log N)$ time complexity compared to $O(N)$ linear search.

4. **Q: How is the 2D array used in this project?**  
   *A:* `int seatMap[MAX_TRAINS][MAX_SEATS]` is a 2D numeric matrix where each row represents a train index and each column represents a physical seat number. A value of `0` denotes an available seat and `1` denotes an occupied seat. Finding a free seat takes $O(\text{seats})$ and checking occupancy takes $O(1)$.

5. **Q: Why is Bubble Sort implemented on a copy of the trains vector?**  
   *A:* The primary `trains` vector must remain sorted by `trainNo` at all times to preserve the $O(\log N)$ binary search precondition. When sorting by fare or name, we sort a copy, leaving the original array intact.

### Part B: Database & Architecture Questions
6. **Q: Why MongoDB NoSQL instead of a relational database?**  
   *A:* MongoDB stores data as flexible BSON/JSON documents. Each passenger record naturally embeds a nested `travelDate` document (`{day, month, year}`), eliminating complex table joins.

7. **Q: What is a REST API?**  
   *A:* Representational State Transfer. It uses standard HTTP methods (`GET`, `POST`, `DELETE`) with uniform JSON requests and responses over stateless network connections.

8. **Q: How does the frontend communicate with your C++ backend?**  
   *A:* The vanilla JavaScript frontend sends asynchronous `fetch()` HTTP requests to endpoints like `POST /api/bookings`. The C++ web server (`server.cpp`) processes the request, invokes the DSA function, and sends back a JSON response.

---

## 👥 Project Team – Group 4
- **Academic Focus:** C++ Programming & Data Structures
- **Course:** B.Tech Computer Science & Engineering (AI / ML)
