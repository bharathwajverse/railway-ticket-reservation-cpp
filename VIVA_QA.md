# Viva Voce Preparation Guide: Railway Ticket Reservation System
**Course:** C++ Data Structures and Algorithms (DSA)  
**Target Audience:** 1st Year B.Tech CSE (AI/ML)  

---

## 1. Structures (Module VI)

#### Q1: What is a `struct` in C++, and why did you use it instead of primitive variables?
**Answer:** A `struct` groups multiple related variables of different data types under one meaningful name. For example, a train has a number, name, source, destination, and fare. Instead of managing separate parallel arrays, `struct Train` bundles them into a single record.

#### Q2: What is the difference between a `struct` and a `class` in C++?
**Answer:** In C++, the only technical difference is default access specifiers: members of a `struct` are `public` by default, whereas members of a `class` are `private`. Since OOP and encapsulation are beyond our current syllabus, we chose `struct` for transparent data grouping.

#### Q3: Why did you use nested structures in `struct Date`?
**Answer:** `struct Date` (containing day, month, and year) is embedded inside `struct Passenger` and `struct WaitingEntry`. This promotes reusability, modular validation, and clear organization.

#### Q4: Why are structures passed by reference (`const Train& t` or `RailwaySystem& sys`)?
**Answer:** Passing by value creates a full copy of the entire structure in memory, wasting CPU cycles and stack space. Passing by reference passes only an alias (memory address), which is fast ($O(1)$). Adding `const` guarantees that read-only functions cannot inadvertently alter the original data.

#### Q5: Can a `struct` have member functions? Why didn't you add member functions to `Train`?
**Answer:** Yes, in C++ structs can have member functions. However, to keep our code strictly procedural as taught in our course modules, we kept all logic in independent, modular functions that accept structures as arguments.

---

## 2. Arrays and 2D Arrays (Modules III & IV)

#### Q6: Where is a 2D array used in this project and why?
**Answer:** In `int seatMap[MAX_TRAINS][MAX_SEATS]`. Row index represents the train, and column index represents the seat number. A value of `0` means free and `1` means booked.

#### Q7: Why is a 2D array superior to a 1D vector for seat visualization?
**Answer:** A 2D array provides immediate $O(1)$ random access to check or toggle any seat status without having to search through a list of passenger tickets.

#### Q8: What are the dimensions of your 2D array, and how much memory does it consume?
**Answer:** $20 \times 60$ integers (`MAX_TRAINS = 20`, `MAX_SEATS = 60`). At 4 bytes per integer, $20 \times 60 \times 4 = 4,800$ bytes ($\approx 4.7\text{ KB}$), which is tiny and easily fits in memory.

#### Q9: What happens if a train has fewer seats than `MAX_SEATS`?
**Answer:** Each `Train` record stores `totalSeats`. Functions only iterate from `0` to `totalSeats - 1`, completely ignoring the unused columns for that row.

#### Q10: How do you map a 1-based seat number to a 0-based array index?
**Answer:** By subtracting 1: Seat #1 maps to `seatMap[trainIdx][0]`, Seat #2 to `seatMap[trainIdx][1]`, and so on.

---

## 3. Strings and Input Validation (Modules I, II & V)

#### Q11: Why did you write `containsIgnoreCase` manually instead of using regex?
**Answer:** Writing our own nested loop traversal directly demonstrates string pattern matching and case-folding algorithms covered in Module V, avoiding overhead and un-taught libraries.

#### Q12: Why do you call `cin.clear()` and `cin.ignore()` when reading integers?
**Answer:** If a user enters letters when a number is expected, `cin` enters an error state (`cin.fail()`) and leaves characters in the input stream. `cin.clear()` resets the error flag, and `cin.ignore(10000, '\n')` discards the bad characters to prevent infinite loops.

#### Q13: Why did you use `std::getline` for train and passenger names?
**Answer:** Standard `cin >> str` stops reading at the first space, meaning names like "New Delhi" or "Rajdhani Express" would be cut in half. `getline` reads the entire line including spaces.

#### Q14: How does your date validation handle leap years?
**Answer:** A year is a leap year if `(year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)`. If true, February is given 29 days instead of 28; otherwise dates like 29/02/2025 are rejected.

---

## 4. Vectors, Queues & Maps (Modules VIII, IX & X)

#### Q15: Why is `std::queue` used for the waiting list?
**Answer:** A waiting list requires strict First-In, First-Out (FIFO) fairness. The first passenger who queued must be the first one promoted when a ticket is cancelled. A queue naturally provides $O(1)$ `push()`, `front()`, and `pop()` operations.

#### Q16: Why is `std::map<int, queue<WaitingEntry>>` used?
**Answer:** Each train has its own separate waiting list. The `map` uses the `trainNo` as the key and maps it to that specific train's FIFO queue, allowing $O(\log T)$ access to any train's waiting queue.

#### Q17: How do you display a queue without destroying its contents?
**Answer:** Since a standard queue only allows access to `front()`, we create a local copy of the queue (`queue<WaitingEntry> copyQueue = originalQueue`). We then iterate through `copyQueue.front()` and `copyQueue.pop()` while the original queue remains untouched.

#### Q18: Why did you choose `std::vector<Train>` over a static array?
**Answer:** `std::vector` manages dynamic heap allocation, tracks its own size (`.size()`), and supports dynamic insertion (`.insert()`) while maintaining contiguous memory for fast indexing.

#### Q19: Why write explicit iterators like `map<int, queue<WaitingEntry>>::const_iterator it`?
**Answer:** In older or strict C++ standards and under our syllabus guidelines, we avoid modern keywords like `auto` to prove that we understand the exact underlying types and iterator mechanics.

---

## 5. Searching and Sorting (Module VII)

#### Q20: Why can Binary Search be used for searching by train number?
**Answer:** Binary Search requires the collection to be sorted. In our system, whenever a new train is added, it is inserted at its sorted position by `trainNo`. Thus, the `trains` vector is always ordered, allowing $O(\log N)$ binary search.

#### Q21: Why is Linear Search used for searching by destination?
**Answer:** Trains are sorted by train number, not destination. Furthermore, multiple trains can share the same destination, and users can search for partial strings (e.g., "Mum" for "Mumbai"). Linear search checks every train in $O(N)$ time.

#### Q22: Why did you sort a copy of the vector in `sortTrains` instead of sorting `sys.trains` directly?
**Answer:** If we sorted `sys.trains` by ticket fare or name, it would no longer be sorted by `trainNo`. That would break binary search and desynchronize the row indexes of our 2D seat map. Sorting a temporary copy preserves the integrity of our primary data structure.

#### Q23: What is the time complexity of Bubble Sort?
**Answer:** $O(N^2)$ in the worst and average cases. While slower than Merge Sort ($O(N \log N)$), $N$ is small ($\le 20$ trains), making Bubble Sort simple, lightweight, and straightforward to explain.

---

## 6. Database and SQLite Integration

#### Q24: Why SQLite instead of MySQL or MongoDB?
**Answer:** SQLite is serverless, zero-configuration, and fully self-contained in a single file (`railway.db`). It eliminates networking, port issues, and credentials, while providing a genuine SQL engine with ACID guarantees.

#### Q25: What is a prepared statement in SQLite?
**Answer:** A prepared statement is precompiled SQL bytecode created by `sqlite3_prepare_v2`. Parameters are passed via placeholders (`?`), and bound safely using `sqlite3_bind_*`. This provides performance optimization and complete immunity against SQL injection attacks.

#### Q26: What is the role of `sqlite3_step()`?
**Answer:** It executes the compiled bytecode. For write queries (`INSERT`, `UPDATE`, `DELETE`), it executes and returns `SQLITE_DONE`. For read queries (`SELECT`), it returns `SQLITE_ROW` for each record until reaching `SQLITE_DONE`.

#### Q27: Why is `sqlite3_finalize()` critical?
**Answer:** It destroys the prepared statement object and releases allocated memory. Forgetting to call it causes memory leaks in the SQLite C runtime.

#### Q28: How does the system handle database persistence across restarts?
**Answer:** On program launch, `loadSystem()` reads all trains, confirmed passengers, and waiting lists from `railway.db`. It then traverses the confirmed passengers to re-mark booked seats in the 2D `seatMap`, restoring the exact memory state.

#### Q29: How does the waiting list maintain FIFO order after being loaded from disk?
**Answer:** The `waiting_list` table has an autoincrementing primary key `wait_id`. When loading, the query executes `SELECT ... ORDER BY wait_id ASC`. Because IDs are assigned in the exact order passengers were queued, pushing them back into `std::queue` restores the exact FIFO sequence.

---

## 7. Edge Cases & Error Handling

#### Q30: What happens if a user tries to book a train that is already full?
**Answer:** The system detects `availableSeats == 0`, displays a notice, and transparently routes the passenger to the waiting list queue with status `WL-1`, saving the record in both memory and SQLite.

#### Q31: What happens if a confirmed passenger cancels their ticket?
**Answer:** The ticket status changes to `CANCELLED`. If the waiting queue is not empty, the head of the queue (`front()`) is popped, allocated the freed seat, assigned a new PNR, marked `CONFIRMED`, and saved to SQLite—all automatically.

#### Q32: What happens if the waiting list itself reaches capacity (`MAX_WAITING = 10`)?
**Answer:** The system rejects the booking with an informative message indicating that both confirmed seats and waiting slots are exhausted.

#### Q33: How do you prevent duplicate bookings?
**Answer:** Before confirming, the system verifies that no passenger with the same name (case-insensitive) is already confirmed on that same train for that same travel date.

#### Q34: What happens if the user inputs invalid text when prompted for an integer?
**Answer:** Our `readInt()` helper catches the error, clears the input stream, ignores leftover characters, and reprompts politely without crashing or entering an infinite loop.

---

## 8. Weak Spots and Honest Answers

#### Q35: Weak Spot: "What happens if two people run this program at the exact same time?"
**Answer:** "Our current application is a single-process console backend. In a multi-user environment, we would need file locking, database transaction isolation (`BEGIN IMMEDIATE`), or a client-server architecture with mutex concurrency."

#### Q36: Weak Spot: "Why is the seat map a fixed-size array (`MAX_TRAINS = 20`) rather than dynamic?"
**Answer:** "A fixed 2D array was chosen to explicitly satisfy the syllabus requirement for multi-dimensional arrays and ensure deterministic, zero-overhead $O(1)$ seat indexing."

---

## 9. Recommended 5-Minute Demo Script

| Step | Action | What to Say / Highlight |
|---|---|---|
| **0:00 - 0:45** | **Introduction** | "Respected Sir, we present Group 4's Railway Ticket Reservation System. We used C++ structures, 2D arrays for seat mapping, Binary Search on sorted train numbers, and a FIFO Queue for waiting passengers, backed by SQLite." |
| **0:45 - 1:30** | **Display & Search** | Run Option 2 (Display Trains). Then Run Option 3 (Search Train #10202 via Binary Search in $O(\log N)$). Search by destination "Mumbai" (Linear Search). |
| **1:30 - 2:30** | **Edge Case 1: Train Full to Waiting Queue** | Display seat map for Train 10303 (3 seats total). Book 3 tickets to fill it. Attempt to book 4th ticket. Show message: *"Train full -> Passenger added to Waiting List at position WL-1 (Queue push)"*. Show Option 8 (Waiting list queue). |
| **2:30 - 3:30** | **Edge Case 2: Cancellation & Auto-Promotion** | Cancel PNR 1001. Highlight terminal alert: *"Auto-promotion event: Waiting passenger promoted to Seat #1 with new PNR"*. Display Passenger Details to prove the waiting passenger is now `CONFIRMED`. Show waiting list is now empty (`queue.pop()`). |
| **3:30 - 4:15** | **Database Verification** | Exit the program (Option 0). Restart the program. Run Option 2 & 7. Show that all trains, seats, and passengers were reloaded from `railway.db`. |
| **4:15 - 5:00** | **Conclusion & Viva Defense** | "Every module follows the syllabus: `struct` with pass-by-reference, 2D arrays, STL queues and vectors, prepared SQL statements, and manual sorting/searching." |
