# 📖 Word-for-Word, Line-by-Line Code Breakdown: `main.cpp`

This private guide walks through every single block and line of `main.cpp` in plain, simple English. Use this to prepare for your viva voce presentation so you understand every character in your code.

---

## 1. HEADER INCLUDES & NAMESPACE

```cpp
#include <iostream>  // Input/Output stream: provides std::cin, std::cout, std::endl
#include <string>    // String class: provides std::string, length(), substr()
#include <vector>    // Dynamic array container: std::vector<Train>
#include <map>       // Associative key-value map: std::map<int, queue<WaitingEntry>>
#include <queue>     // FIFO Queue container: std::queue<WaitingEntry>
#include <stack>     // LIFO Stack container: std::stack<Passenger>
#include <iomanip>   // I/O Manipulators: std::setw(), std::setprecision(), std::fixed
#include <cctype>    // Character classification: tolower(), isalpha(), isspace(), toupper()
#include <cstdlib>   // General utilities
#include "sqlite3.h" // SQLite 3 C-library API: handles database connection and queries

using namespace std; // Allows using cout, cin, string without writing std:: prefix
```

---

## 2. SECTION 1: SYSTEM CONSTANTS & DATA STRUCTURES

### Constants
```cpp
const int MAX_TRAINS = 20;   // Maximum trains our system can manage
const int MAX_SEATS = 60;    // Maximum seats per train (columns in 2D seatMap)
const int MAX_WAITING = 10;  // Maximum passengers allowed in waiting queue per train
```
- **Why `const`?** Prevents accidental modification anywhere in the program.

### `struct Date`
```cpp
struct Date {
    int day;   // 1 to 31
    int month; // 1 to 12
    int year;  // 2024 to 2035
};
```
- **Word-for-Word Explanation:** Groups the three integer components of a calendar date. Nested inside `Passenger` and `WaitingEntry` to avoid duplicating date fields.

### `struct Train`
```cpp
struct Train {
    int trainNo;        // Unique 5-digit train identifier (e.g., 10101)
    string name;        // Train name (e.g., "Rajdhani Express")
    string source;      // Departure station (e.g., "Delhi")
    string destination; // Arrival station (e.g., "Mumbai")
    string departure;   // Time string (e.g., "06:00 AM")
    int totalSeats;     // Total seat capacity (e.g., 4)
    int availableSeats; // Unbooked seat count (decrements on booking, increments on cancel)
    float fare;         // Ticket price in Rupees (e.g., 1500.00)
};
```

### `struct Passenger`
```cpp
struct Passenger {
    int pnr;         // Unique Passenger Name Record number (starts at 1001)
    string name;     // Passenger full name
    int age;         // Age in years (1 to 120)
    char gender;     // 'M', 'F', or 'O'
    int trainNo;     // Associated train number
    int seatNo;      // Assigned seat number (1-based index)
    Date travelDate; // Nested Date structure
    string status;   // "CONFIRMED" or "CANCELLED"
};
```

### `struct WaitingEntry`
```cpp
struct WaitingEntry {
    int waitId;      // Database autoincrement primary key
    string name;     // Waiting passenger's name
    int age;         // Waiting passenger's age
    char gender;     // 'M', 'F', or 'O'
    int trainNo;     // Target train number
    Date travelDate; // Desired travel date
};
```
- **Why separate from `Passenger`?** Waiting passengers do NOT have an assigned `seatNo` or confirmed `pnr`. Separating them avoids storing dummy seat numbers.

### `struct RailwaySystem`
```cpp
struct RailwaySystem {
    vector<Train> trains;                         // Sorted list of all active trains
    vector<Passenger> passengers;                 // History of all booked & cancelled tickets
    map<int, queue<WaitingEntry> > waitingLists;  // Key = trainNo -> FIFO waiting queue
    int seatMap[MAX_TRAINS][MAX_SEATS];           // 2D grid: 0 = free, 1 = booked
    stack<Passenger> recentCancellations;         // LIFO stack for undoing cancellations
};
```

---

## 3. SECTION 2: INPUT VALIDATION & STRING UTILITIES

### `isLeapYear(int year)`
- Checks if `(year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)`.
- If true, February has 29 days; otherwise 28 days.

### `isValidDate(int day, int month, int year)`
- Verifies `year` is within realistic bounds (2024 to 2035).
- Verifies `month` is 1 to 12.
- Uses a 1D array `daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}`.
- If leap year and month == 2, adjusts `daysInMonth[1] = 29`.
- Returns `false` if `day < 1 || day > daysInMonth[month - 1]`.

### `toLowerCase(const string& str)`
- Iterates from `i = 0` to `str.length() - 1`.
- Calls `tolower(str[i])` and appends to a new string `result`.
- Guarantees comparisons are case-insensitive ("Delhi" equals "delhi").

### `containsIgnoreCase(const string& text, const string& pattern)`
- Converts both `text` and `pattern` to lowercase.
- Uses nested loops to check if `lowerPattern` is a substring of `lowerText`.
- Allows partial searching: typing "mumb" will match "Mumbai".

### `readInt(prompt, minVal, maxVal)`
- Loops `while (true)`:
  - Prints prompt.
  - Executes `if (cin >> value)`:
    - If between `minVal` and `maxVal`, calls `cin.ignore(10000, '\n')` to flush leftover newline and returns `value`.
    - Else prints range error.
  - If `cin >> value` fails (e.g., user typed "abc"):
    - Prints error message.
    - Calls `cin.clear()` to reset the internal error flag.
    - Calls `cin.ignore(10000, '\n')` to discard invalid characters from the stream.

### `readName(prompt)`
- Uses `getline(cin, name)`.
- Manually trims leading spaces: increments `start` while `isspace(name[start])`.
- Manually trims trailing spaces: decrements `end` while `isspace(name[end - 1])`.
- Verifies every character is either `isalpha()` or `isspace()`.
- Rejects blank entries.

---

## 4. SECTION 3: SQLITE DATABASE LAYER

### `openDatabase(sqlite3*& db, const char* fileName)`
- Calls `sqlite3_open(fileName, &db)`.
- If successful, returns `true`; if failed, prints `sqlite3_errmsg(db)` and returns `false`.

### `createTables(sqlite3* db)`
- Executes `sqlite3_exec()` with DDL string creating `trains`, `passengers`, and `waiting_list` tables with `CREATE TABLE IF NOT EXISTS`.

### Prepared Statements Pattern (Used in `insertTrain`, `insertPassenger`, `insertWaiting`):
1. **Prepare:** `sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);`
   - Compiles SQL query with `?` placeholders into bytecode.
2. **Bind:**
   - `sqlite3_bind_int(stmt, 1, value);` -> Fills 1st placeholder with integer.
   - `sqlite3_bind_text(stmt, 2, str.c_str(), -1, SQLITE_TRANSIENT);` -> Fills 2nd placeholder with string. `SQLITE_TRANSIENT` tells SQLite to make its own copy of the string.
   - `sqlite3_bind_double(stmt, 8, (double)fare);` -> Fills placeholder with float/double.
3. **Step:** `int rc = sqlite3_step(stmt);`
   - Executes the query. Expects `SQLITE_DONE` for inserts/updates.
4. **Finalize:** `sqlite3_finalize(stmt);`
   - Frees memory allocated for the statement.

### `loadWaiting(sqlite3* db, map<int, queue<WaitingEntry>>& lists)`
- Query: `SELECT ... FROM waiting_list ORDER BY wait_id ASC;`
- **Why `ORDER BY wait_id ASC`?** `wait_id` is autoincrementing. Lower `wait_id` means the passenger queued earlier. Loading in ascending order and calling `lists[trainNo].push(w)` recreates the exact FIFO queue in memory!

---

## 5. SECTION 4: RAILWAY LOGIC & CORE ALGORITHMS

### `binarySearchTrain(vector<Train>& trains, int trainNo)`
- `low = 0`, `high = trains.size() - 1`.
- `mid = low + (high - low) / 2` (prevents integer overflow compared to `(low + high) / 2`).
- If `trains[mid].trainNo == trainNo` -> returns index `mid`.
- If `trains[mid].trainNo < trainNo` -> `low = mid + 1`.
- Else -> `high = mid - 1`.
- If `low > high`, returns `-1` (Not found).
- **Time Complexity:** $O(\log N)$.

### `addTrain(RailwaySystem& sys, sqlite3* db)`
- Checks `sys.trains.size() < MAX_TRAINS`.
- Calls `findTrainIndex` to reject duplicate train numbers.
- Finds sorted position: `while (pos < size && trains[pos].trainNo < t.trainNo) pos++;`.
- Shifts `seatMap` rows down from `size` down to `pos + 1` so each train's row index matches its vector index.
- Zeroes out `seatMap[pos]` (all seats initially free).
- Inserts train at `trains.begin() + pos`.
- Persists record to SQLite via `insertTrain()`.

### `bookTicket(RailwaySystem& sys, sqlite3* db)`
- Asks for train number, passenger name, age, gender, and date.
- Duplicate check: Scans `sys.passengers` for an already `CONFIRMED` ticket with the identical name on the same train and date.
- Checks `findFreeSeat(sys, trainIdx)`:
  - **If seat available (`availableSeats > 0`):**
    - Finds first column in `seatMap[trainIdx]` containing `0`.
    - Marks `seatMap[trainIdx][seatIdx] = 1` (booked).
    - Decrements `trains[trainIdx].availableSeats`.
    - Generates unique PNR (`max(PNR) + 1`).
    - Appends to `sys.passengers` and calls `insertPassenger(db, p)`.
    - Synchronizes updated seat count in SQLite via `updateTrainSeats()`.
  - **If train is full (`availableSeats == 0`):**
    - Checks waiting queue size `< MAX_WAITING`.
    - Creates `WaitingEntry`.
    - Pushes into `sys.waitingLists[trainNo].push(w)` (FIFO Queue).
    - Inserts into SQLite `waiting_list` table.
    - Notifies user of their position: `WL-X`.

### `cancelTicket(RailwaySystem& sys, sqlite3* db)`
- Asks for PNR, searches via `findPassengerByPNR()` ($O(P)$).
- Confirms ticket is not already `CANCELLED`.
- Changes status to `CANCELLED` in memory and calls `updatePassengerStatus()`.
- Pushes ticket to `sys.recentCancellations.push(p)` (Stack LIFO).
- Frees seat: `seatMap[trainIdx][seatNo - 1] = 0`.
- Calls `promoteFromWaitingList()`.

### `promoteFromWaitingList(sys, db, trainIndex, seatNo)`
- Checks if `sys.waitingLists[trainNo]` is non-empty.
- **If waiting passenger exists:**
  - `topWait = wQueue.front();` (inspects first in line).
  - `wQueue.pop();` (removes from queue).
  - Deletes row from SQLite `waiting_list`.
  - Allocates freed `seatNo` to this passenger with a brand-new PNR.
  - Marks `seatMap[trainIndex][seatNo - 1] = 1`.
  - Inserts new confirmed ticket into `sys.passengers` and SQLite.
- **If waiting queue was empty:**
  - Increments `trains[trainIndex].availableSeats++`.
  - Updates SQLite available seats.

### `undoLastCancellation(sys, db)`
- Inspects `sys.recentCancellations.top()`.
- Checks if `seatMap[trainIdx][seatNo - 1] == 0`:
  - If still free, pops the stack: `sys.recentCancellations.pop()`.
  - Re-books seat: `seatMap = 1`, decrements `availableSeats`.
  - Reverts passenger status to `CONFIRMED` in memory and SQLite.
  - Restores booking cleanly.
- If seat was already taken by auto-promotion, prints error: cannot undo!

### `displayAvailableSeats(sys)`
- Visualizes `seatMap[trainIdx]` in rows of 6:
  - If `seatMap[trainIdx][s] == 1` -> prints `[ XX ]`.
  - If `seatMap[trainIdx][s] == 0` -> prints `[  N ]` where $N = s + 1$.

### `sortTrains(sys)`
- Clones `sys.trains` into a temporary `copyList`.
- Runs nested Bubble Sort loops on `copyList` by fare or name.
- Displays `copyList` without modifying `sys.trains`, keeping the main system sorted for Binary Search.

---

## 6. SECTION 5: MAIN MENU & ENTRY POINT

### `main()` Execution Flow:
1. Calls `openDatabase(db, "railway.db")`.
2. Calls `createTables(db)`.
3. Calls `loadSystem(sys, db)` to reconstruct all in-memory arrays, queues, and 2D seat maps from SQLite.
4. Enters `do-while` menu loop.
5. Dispatches user choice (1 to 10) to modular functions.
6. When `0` is chosen, loop terminates and calls `closeDatabase(db)`.
