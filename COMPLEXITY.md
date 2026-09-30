# Railway Ticket Reservation System - Complexity Analysis

This document provides asymptotic time and space complexity for every major function and data structure operation in the project, formatted for first-year B.Tech DSA examination and viva defense.

---

## 1. Summary Complexity Table

| Function / Operation | Location | Time Complexity (Best) | Time Complexity (Average) | Time Complexity (Worst) | Space Complexity | Justification |
|---|---|---|---|---|---|---|
| `binarySearchTrain` | `railway.cpp` | $O(1)$ | $O(\log N)$ | $O(\log N)$ | $O(1)$ | Divides the search range in half each iteration over sorted `trains` vector. |
| `searchTrainByDestination` | `railway.cpp` | $O(N \times M)$ | $O(N \times M)$ | $O(N \times M)$ | $O(1)$ auxiliary | Iterates through all $N$ trains and checks substring matching against pattern length $M$. |
| `sortTrains` (Bubble Sort) | `railway.cpp` | $O(N)$ | $O(N^2)$ | $O(N^2)$ | $O(N)$ | Operates nested loops comparing adjacent train records on a temporary copy vector. |
| `findFreeSeat` | `railway.cpp` | $O(1)$ | $O(S)$ | $O(S)$ | $O(1)$ | Linearly scans row in 2D array `seatMap[trainIdx]` up to total seats $S$. |
| `generatePNR` | `railway.cpp` | $O(P)$ | $O(P)$ | $O(P)$ | $O(1)$ | Linearly inspects existing passengers to compute $\max(\text{PNR}) + 1$. |
| `bookTicket` | `railway.cpp` | $O(\log N)$ | $O(P + \log N)$ | $O(P + \log N)$ | $O(1)$ | Performs binary search for train, duplicate passenger scan ($O(P)$), and seat/queue insertion ($O(1)$). |
| `findPassengerByPNR` | `railway.cpp` | $O(1)$ | $O(P)$ | $O(P)$ | $O(1)$ | Linear scan through the unsorted `passengers` vector. |
| `promoteFromWaitingList` | `railway.cpp` | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ | Queue `front()` and `pop()` operations take constant time; updating 2D array and database row is $O(1)$. |
| `cancelTicket` | `railway.cpp` | $O(1)$ | $O(P + \log N)$ | $O(P + \log N)$ | $O(1)$ | Finds passenger by PNR ($O(P)$), pushes to cancellation stack ($O(1)$), and triggers promotion ($O(1)$). |
| `viewLastCancelledTicket` | `railway.cpp` | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ | Direct $O(1)$ access to the `top()` element of the `std::stack`. |
| `undoLastCancellation` | `railway.cpp` | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ `pop()` from stack and constant-time status and seat restoration. |
| `displayAvailableSeats` | `railway.cpp` | $O(\log N)$ | $O(\log N + S)$ | $O(\log N + S)$ | $O(1)$ | Binary search to locate train row, then prints $S$ seat statuses from 2D array. |
| `displayWaitingList` | `railway.cpp` | $O(1)$ | $O(T \times W)$ | $O(T \times W)$ | $O(W)$ | Iterates map of trains ($T$) and traverses copied queue of size $W$. |
| `loadSystem` | `railway.cpp` | $O(1)$ | $O(T + P + W)$ | $O(T + P + W)$ | $O(T + P + W)$ | Rebuilds in-memory vector, queue map, and 2D seat grid from SQLite rows. |
| `isLeapYear` | `utils.cpp` | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ | Basic arithmetic checks (`year % 4`, `year % 100`, `year % 400`). |
| `isValidDate` | `utils.cpp` | $O(1)$ | $O(1)$ | $O(1)$ | $O(1)$ | Constant lookup into days-per-month array. |
| `containsIgnoreCase` | `utils.cpp` | $O(1)$ | $O(N \times M)$ | $O(N \times M)$ | $O(N + M)$ | Manual string traversal comparing substring matches of length $M$ inside text $N$. |

*Where:*
- $N$ = Number of trains in system ($\le 20$)
- $S$ = Total seats per train ($\le 60$)
- $P$ = Total passenger records
- $W$ = Number of waiting list entries ($\le 10$ per train)
- $M$ = Length of search destination string

---

## 2. Space Complexity Breakdown

1. **2D Seat Map (`int seatMap[MAX_TRAINS][MAX_SEATS]`):**
   - Memory allocated: $20 \times 60 \times 4\text{ bytes} = 4,800\text{ bytes} \approx 4.7\text{ KB}$.
   - Provides instant $O(1)$ access and seat reservation checks.
2. **Train Vector (`vector<Train>`):**
   - Stores up to `MAX_TRAINS` items. Contiguous in heap memory.
3. **Passenger Vector (`vector<Passenger>`):**
   - Dynamic size $O(P)$, amortized $O(1)$ insertion.
4. **Waiting List Queues (`map<int, queue<WaitingEntry>>`):**
   - Map keys provide $O(\log T)$ lookup to train-specific FIFO queues.
   - Enqueue and dequeue operations are strictly $O(1)$.
5. **Recent Cancellations Stack (`stack<Passenger>`):**
   - Dynamically tracks recently cancelled tickets in LIFO order for inspection and undo operations with $O(1)$ push and pop.
