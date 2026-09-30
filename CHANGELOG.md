# Changelog

All notable changes to this project will be documented in this file.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.1.0] - 2026-10-01
### Changed
- Streamlined `main.cpp` into a focused, pure standard C++11 implementation strictly aligned with the 10 syllabus modules.
- Removed Winsock socket networking, multi-threading, and mutex overhead to simplify viva defense and code review.
- Retained interactive Web UI preview mockup in `web/index.html` for presentation purposes.
- Removed `-lws2_32` link dependency from `build.bat`, `Makefile`, and CI workflows.

---

## [2.0.0] - 2026-09-30
### Added
- Embedded Native Winsock HTTP Server (Port 8080) for zero-dependency web access.
- Modern Responsive Web Dashboard (`web/index.html`) with interactive seat grid and live concession calculator.
- Comprehensive REST API endpoints (`/api/trains`, `/api/seats`, `/api/pnr`, `/api/stats`, `/api/manifest`, `/api/ticket`, `/api/book`, `/api/cancel`).
- Age-based fare concessions (Child 50% discount, Senior Citizen 40% discount).
- Electronic ticket text file export (`ticket_<PNR>.txt`) via `std::ofstream`.
- Manual seat selection option with visual coach seat map alongside auto-assign.
- Direct cancellation of waiting list entries from FIFO queue and SQLite database.
- Role-based separation into Passenger Portal and PIN-protected Administrator Portal.
- Executive Analytics Dashboard (Revenue, Bookings, Waitlist, Occupancy).
- Atomic SQLite transactions (`BEGIN IMMEDIATE TRANSACTION;` / `COMMIT;` / `ROLLBACK;`) for ACID consistency.

---

## [1.2.0] - 2026-09-30
### Added
- Unified all application logic into a single clean file (`main.cpp`) for simplified deployment and readability.
- Added comprehensive multi-line instructional comments across all functions and structures.
- Added CLI command-line argument flags (`--version`, `--help`).
- Added SQLite PRAGMA performance and integrity tuning (`foreign_keys = ON`, `synchronous = NORMAL`).
- Added `CONTRIBUTING.md` and `CODE_OF_CONDUCT.md`.
- Added GitHub issue templates and PR template.

### Changed
- Streamlined repository structure and excluded build artifacts from git history.

---

## [1.1.0] - 2026-09-30
### Added
- Integrated Module VIII Stack (`std::stack<Passenger> recentCancellations`) for LIFO cancellation tracking and undo support.
- Added interactive Undo Cancellation feature in main menu (Option 10).
- Cross-platform Makefile for Linux and macOS compilation.

---

## [1.0.0] - 2026-09-30
### Initial Release
- Core railway reservation system with complete seat management.
- Binary Search for train lookup by train number ($O(\log N)$).
- Linear Search for destination with case-insensitive partial substring matching ($O(N)$).
- Manual Bubble Sort on a copy vector for display by fare or name ($O(N^2)$).
- 2D seat map array `seatMap[20][60]` for $O(1)$ random-access seat reservations.
- FIFO waiting queue `map<int, queue<WaitingEntry>>` for full-train handling.
- Automatic FIFO queue promotion upon ticket cancellation.
- SQLite 3 database persistence (`railway.db`) using prepared statements.
