# System Architecture & Technical Design

This document details the internal architectural design, data structures, concurrency model, and data flow of the **Railway Ticket Reservation System**.

---

## 1. High-Level Architectural Overview

The system employs a **dual-interface, dual-storage** architecture:

```
┌─────────────────────────────────────────────────────────────┐
│                       Client Layer                          │
│   ┌───────────────────────────┐ ┌───────────────────────┐   │
│   │ Terminal Kiosk (Console)  │ │ Web Dashboard (HTML5) │   │
│   └─────────────┬─────────────┘ └───────────┬───────────┘   │
└─────────────────┼───────────────────────────┼───────────────┘
                  │                           │ HTTP / JSON (:8080)
┌─────────────────┼───────────────────────────┼───────────────┐
│                 ▼                           ▼               │
│   ┌───────────────────────────┐ ┌───────────────────────┐   │
│   │    Console Menu Loops     │ │ Embedded HTTP Server  │   │
│   │   (Passenger / Admin)     │ │   (Winsock / POSIX)   │   │
│   └─────────────┬─────────────┘ └───────────┬───────────┘   │
│                 │                           │               │
│                 └───────────┬───────────────┘               │
│                             ▼                               │
│                 ┌───────────────────────┐                   │
│                 │  std::mutex (Thread   │                   │
│                 │     Synchronization)  │                   │
│                 └───────────┬───────────┘                   │
│                             ▼                               │
│              ┌─────────────────────────────┐                │
│              │ In-Memory Data Structures   │                │
│              │ (vector, 2D map, queue,     │                │
│              │  stack, associative map)    │                │
│              └──────────────┬──────────────┘                │
│                             ▼                               │
│              ┌─────────────────────────────┐                │
│              │ SQLite 3 Relational Layer   │                │
│              │ (railway.db - ACID commits) │                │
│              └─────────────────────────────┘                │
└─────────────────────────────────────────────────────────────┘
```

---

## 2. In-Memory Data Structures (Dual Storage)

| Entity | Primary Data Structure | Algorithmic Justification | Time Complexity |
|---|---|---|---|
| **Trains Registry** | `std::vector<Train>` | Kept sorted by `trainNo` to enable Binary Search | Lookup: $O(\log N)$, Insert: $O(N)$ |
| **Seat Allocation Grid** | `int seatMap[MAX_TRAINS][MAX_SEATS]` | 2D contiguous matrix where row = train index, column = seat number (0 = free, 1 = booked) | Lookup & Booking: $O(1)$ |
| **Waiting Lists** | `std::map<int, std::queue<WaitingEntry>>` | Associative dictionary mapping each `trainNo` to a First-In, First-Out (FIFO) queue | Promotion: $O(1)$, Enqueue: $O(1)$ |
| **Ticket Registry** | `std::vector<Passenger>` | Contiguous collection of all active and cancelled passenger tickets | Linear scan: $O(P)$ |
| **Cancellation History** | `std::stack<Passenger>` | Last-In, First-Out (LIFO) stack of recently cancelled tickets for one-click undo | Push / Pop: $O(1)$ |

---

## 3. Concurrency & Thread Safety

The embedded Winsock HTTP server runs in a detached background thread (`std::thread`), concurrently serving browser HTTP requests while the main thread runs the interactive terminal kiosk.

To prevent race conditions during simultaneous bookings, cancellations, or reads:
- Every mutating and reading operation across both the terminal menus and the HTTP REST handlers acquires a `std::lock_guard<std::mutex>` on the global `g_sysMutex`.
- SQLite multi-step operations use `BEGIN IMMEDIATE TRANSACTION;` and `COMMIT;` blocks to prevent concurrent write contention at the database file level.

---

## 4. REST API Pipeline

The embedded HTTP server operates on port `8080`:

1. **Winsock Initialization:** Initializes `WSAStartup` (Windows) and binds a stream socket (`AF_INET`, `SOCK_STREAM`).
2. **Connection Acceptance:** Loops on `accept()`, receiving client HTTP requests.
3. **HTTP Parsing:** Parses request method (`GET`, `POST`), target URI path, headers, and `Content-Length`.
4. **Body Assembly:** For `POST` payloads, loops on `recv()` until the complete JSON payload is received.
5. **Route Dispatching:** Matches the URI to appropriate handler functions (`/api/trains`, `/api/seats`, `/api/book`, etc.).
6. **JSON Serialization:** Formats the in-memory response data into standard JSON strings without external dependencies.
7. **HTTP Response:** Sends `HTTP/1.1 200 OK` with `Content-Type: application/json` or `text/html` and closes the socket.

---

## 5. Persistence & ACID Transactions

The database (`railway.db`) consists of three tables:
- `trains`: Train specifications and seat capacities.
- `passengers`: Confirmed and cancelled tickets, assigned seats, concession tiers, and paid fares.
- `waiting_list`: FIFO queues for over-capacity passenger bookings.

All booking transactions follow the **ACID** principle:
- **Atomicity:** Decrementing available seats, reserving the seat map slot, and inserting the passenger record succeed together or fail together via `ROLLBACK`.
- **Consistency:** Seat constraints and primary keys are enforced by SQLite constraints and in-memory validation.
- **Isolation:** Managed via `std::mutex` and SQLite transaction locking.
- **Durability:** Changes are flushed to disk before the HTTP response or console confirmation is emitted.
