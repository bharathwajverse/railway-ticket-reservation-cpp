# 🎓 Advanced Features & Full-Stack Web Architecture Viva Defense Guide

> **Confidential Student Study Guide**  
> *This file is located in `study_guide/` and is strictly ignored by `.gitignore` so that professors cannot see your preparation notes.*

---

## 1. How to Explain the Embedded Web Interface (Winsock2)

### Question: "Why did you build a web interface for a C++ DSA project, and does it require external servers like Node.js or Apache?"
**Word-for-Word Answer:**
> *"No, Sir. Our project has **zero external server dependencies**. We embedded a native HTTP/1.1 server directly inside C++ using standard Windows Winsock sockets (`winsock2.h`, linked via `-lws2_32`).  
> When `railway.exe` starts, it launches a background worker thread (`std::thread`) listening on port 8080. When a browser opens `http://localhost:8080`, our C++ socket code parses the incoming HTTP GET/POST headers, retrieves data from our in-memory C++ STL containers and SQLite backend, and responds with JSON or HTML.  
> Both the terminal kiosk and browser interface run simultaneously in real-time, synchronized via `std::mutex` to prevent race conditions."*

---

## 2. How to Explain Age-Based Fare Concessions

### Question: "How does the concession logic work, and where is it calculated?"
**Word-for-Word Answer:**
> *"Sir, we implemented `calculateConcession(age, baseFare, concessionTier, finalFare)` in Section 2.  
> - For passengers under 12 years (children), we apply a **50% discount** (`fare * 0.50f`).  
> - For senior citizens aged 60 and above, we apply a **40% discount** (`fare * 0.60f`).  
> - For all other passengers (12 to 59 years), standard general fare applies.  
> In our web dashboard, as the user types their age, the discount badge and final price calculate dynamically in real-time before submission. Once booked, both the concession tier and the discounted fare paid are recorded in SQLite and printed on the official E-Ticket."*

---

## 3. How to Explain Manual Seat Selection vs. Auto-Assign

### Question: "How did you implement manual seat selection using your 2D array?"
**Word-for-Word Answer:**
> *"Sir, in our 2D array `seatMap[MAX_TRAINS][MAX_SEATS]`, each train index maps to 60 seat positions initialized to `0` (free).  
> During booking, the passenger can either choose:  
> 1. **Auto-Assign:** Our greedy algorithm `findFreeSeat()` scans the row and allocates the first index containing `0`.  
> 2. **Manual Selection:** In the CLI, it prints the 2D coach layout and lets the user choose their exact seat number (1 to total seats). In the web interface, it renders a visual coach grid where available seats are green and clickable.  
> If a passenger selects an already occupied seat (`seatMap[trainIdx][seat - 1] == 1`), the system rejects it and offers a chance to choose another seat or fall back to auto-assign."*

---

## 4. How to Explain E-Ticket File Export (`ticket_<PNR>.txt`)

### Question: "Which C++ concept is used for generating the ticket file?"
**Word-for-Word Answer:**
> *"Sir, this implements **Module V: File I/O Streams** using `std::ofstream` from `<fstream>`.  
> Once a ticket is confirmed, `exportTicketToFile()` serializes an official Electronic Reservation Slip to disk named `ticket_<PNR>.txt`. It formats passenger details, travel date, allocated seat number, berth category (Window, Aisle, Middle calculated via modulo arithmetic `seatNo % 4`), concession tier, base fare, discount savings, and final fare paid.  
> It is also directly downloadable via the web interface using the `/api/ticket?pnr=X` endpoint."*

---

## 5. How to Explain Waiting List Direct Cancellation

### Question: "Standard `std::queue` does not have random access or a search method. How did you cancel a waiting passenger from the middle of the queue?"
**Word-for-Word Answer:**
> *"Sir, that is a classic Queue Data Structure problem! Since `std::queue` only allows access to `front()` and removal via `pop()`, we implemented a **temporary queue filtering algorithm** in `cancelWaitingListEntry()`:  
> 1. We create a temporary empty queue `tempQueue`.  
> 2. We dequeue each entry from the train's waiting queue one by one using `front()` and `pop()`.  
> 3. If an entry matches the target passenger being cancelled, we delete their row from the SQLite `waiting_list` table and **skip pushing** them to `tempQueue`.  
> 4. All other waiting passengers are pushed into `tempQueue`.  
> 5. Finally, we assign `wQueue = tempQueue`, which restores the remaining waiting passengers in their exact original FIFO sequence without disturbing queue fairness!"*

---

## 6. How to Explain Atomic SQLite Transactions (ACID Properties)

### Question: "What are atomic transactions and why are they needed in this project?"
**Word-for-Word Answer:**
> *"Sir, in a railway reservation system, booking a ticket requires multiple database operations: updating `available_seats` in the `trains` table and inserting a new row into the `passengers` table.  
> If the application crashed midway, the database would become corrupt (seats deducted but no ticket issued).  
> To guarantee **ACID Atomicity**, we wrap multi-table updates in `beginTransaction()` (`BEGIN IMMEDIATE TRANSACTION;`) and `commitTransaction()` (`COMMIT;`). If any query fails, we call `rollbackTransaction()` (`ROLLBACK;`) to revert the database to its pristine prior state."*

---

## 7. How to Explain Administrator vs. Passenger Role Separation

### Question: "How is access control handled in the system?"
**Word-for-Word Answer:**
> *"Sir, at startup the application offers a clean role selector:  
> - **Passenger Portal:** Exposes customer-facing operations (search trains, inspect coach seat maps, book tickets with manual seat choice and age concession, cancel tickets, look up PNR, export E-Tickets).  
> - **Administrator Portal:** Protected by a security PIN (`admin123`). Only administrators can add new trains, inspect full passenger manifests, review complete seat occupancy matrices across all trains, inspect waiting queues, and view live revenue and booking analytics (`displaySystemStats()`)."*
