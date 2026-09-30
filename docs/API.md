# REST API Specification

The embedded C++ Winsock HTTP server exposes a lightweight REST API on port `8080`. All payloads are encoded in UTF-8 JSON.

---

## Base URL
```text
http://localhost:8080
```

---

## Endpoints Summary

| Method | Endpoint | Description | Query Parameters / Body |
|---|---|---|---|
| `GET` | `/` | Serves the web dashboard (`web/index.html`) | None |
| `GET` | `/api/trains` | List all scheduled trains and seat availability | None |
| `GET` | `/api/seats` | Get seat occupancy array for a train | `?trainNo=<int>` |
| `GET` | `/api/pnr` | Lookup passenger ticket details by PNR | `?pnr=<int>` |
| `GET` | `/api/ticket` | Download or view plain-text electronic ticket slip | `?pnr=<int>` |
| `GET` | `/api/stats` | Executive metrics (trains, bookings, revenue) | None |
| `GET` | `/api/manifest` | Complete passenger booking manifest | None |
| `POST` | `/api/book` | Book a ticket with age concession and seat preference | JSON Body |
| `POST` | `/api/cancel` | Cancel a confirmed ticket by PNR | JSON Body |

---

## Endpoint Details

### 1. `GET /api/trains`
Returns an array of all active trains in the system.

**Response `200 OK`:**
```json
[
  {
    "trainNo": 10101,
    "name": "Rajdhani Express",
    "source": "Delhi",
    "destination": "Mumbai",
    "departure": "06:00 AM",
    "totalSeats": 4,
    "availableSeats": 2,
    "fare": 1500.00
  }
]
```

---

### 2. `GET /api/seats?trainNo=10101`
Returns total capacity, available count, and a 0/1 occupancy array (`0` = free, `1` = booked).

**Response `200 OK`:**
```json
{
  "trainNo": 10101,
  "totalSeats": 4,
  "availableSeats": 2,
  "seats": [1, 1, 0, 0]
}
```

---

### 3. `POST /api/book`
Books a ticket for a passenger. If seats are available, allocates the selected seat (or auto-assigns if `seatNo` is `-1`) and computes the age-based concession. If the train is full, adds the passenger to the FIFO waiting list.

**Request Body:**
```json
{
  "trainNo": 10101,
  "name": "Aarav Sharma",
  "age": 65,
  "gender": "M",
  "dateStr": "2026-11-15",
  "seatNo": 3
}
```

**Response `200 OK` (Confirmed):**
```json
{
  "success": true,
  "status": "CONFIRMED",
  "pnr": 1009,
  "name": "Aarav Sharma",
  "age": 65,
  "seatNo": 3,
  "concession": "SENIOR CITIZEN (40% OFF)",
  "farePaid": 900.00
}
```

**Response `200 OK` (Waitlisted):**
```json
{
  "success": true,
  "status": "WAITING",
  "name": "Aarav Sharma",
  "waitPos": 1
}
```

---

### 4. `GET /api/pnr?pnr=1009`
Retrieves ticket status and travel details by unique PNR number.

**Response `200 OK`:**
```json
{
  "found": true,
  "pnr": 1009,
  "name": "Aarav Sharma",
  "age": 65,
  "gender": "M",
  "trainNo": 10101,
  "seatNo": 3,
  "date": "15/11/2026",
  "status": "CONFIRMED",
  "concession": "SENIOR CITIZEN (40% OFF)",
  "farePaid": 900.00
}
```

---

### 5. `POST /api/cancel`
Cancels an active confirmed ticket, marks it `CANCELLED` in SQLite, pushes to the undo stack, frees the coach seat, and automatically promotes the head of the FIFO waiting queue if one exists.

**Request Body:**
```json
{
  "pnr": 1009
}
```

**Response `200 OK`:**
```json
{
  "success": true,
  "message": "Seat is now free for booking."
}
```

---

### 6. `GET /api/stats`
Returns aggregate system analytics for executive dashboard cards.

**Response `200 OK`:**
```json
{
  "totalTrains": 4,
  "totalBookings": 8,
  "confirmedBookings": 6,
  "cancelledBookings": 2,
  "totalWaitlisted": 0,
  "totalRevenue": 8250.00
}
```
