# Railway Ticket Reservation System - Project Context & Master Guide

## Project Information
- Group 4 – Railway Ticket Reservation System
- Student: First-year B.Tech CSE (AI/ML)
- Course: Data Structures and Algorithms (DSA in C++)
- Focus: Menu-driven C++ console backend connected to SQLite database (`railway.db`)

## Official Requirements
1. Add / display train information
2. Search train by number or destination
3. Book a ticket
4. Cancel a ticket
5. Display available seats
6. Display passenger details
7. Maintain a waiting list using a queue

## Syllabus Constraints (Strictly Enforced)
- Standard: C++11, compiled with g++.
- Paradigm: Procedural with `struct` and functions.
- NOT allowed: `class`, inheritance, polymorphism, templates, lambdas, `auto`, smart pointers, `try/catch`, operator overloading.
- Allowed containers: `iostream`, `string`, `vector`, `map`, `queue`, `stack`, `iomanip`, `cctype`, `cstdlib`, `sqlite3.h`.
- Manual algorithms: Searching (Binary Search by train number, Linear Search by destination and PNR) and Sorting (Bubble Sort) written manually without `std::sort`.
- Memory + Persistence: In-memory working structures (`vector`, 2D array, `queue`) synchronized with SQLite database tables.

## Architecture
```
RailwayReservation/
├── main.cpp          -> Menu loop and system lifecycle
├── structures.h      -> Struct definitions & system constants
├── utils.h / utils.cpp        -> Input validation helpers & string tools
├── database.h / database.cpp  -> SQLite integration & prepared statements
├── railway.h / railway.cpp    -> Core train, booking, cancel, waiting-list logic
├── sqlite3.c / sqlite3.h      -> SQLite 3 amalgamation C library
├── build.bat         -> Single-command compilation script
├── COMPLEXITY.md     -> Time and space complexity analysis
├── VIVA_QA.md        -> 40+ curated viva questions with beginner-friendly answers
└── README.md         -> Project documentation and DSA mapping
```
