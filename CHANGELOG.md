# Changelog

All notable changes to this project will be documented in this file.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [1.2.0] - 2026-09-30
### Added
- Unified all application logic into a single clean file (`main.cpp`) for transparent code defense.
- Added comprehensive multi-line instructional comments across all functions and structures.
- Added CLI command-line argument flags (`--version`, `--help`).
- Added SQLite PRAGMA performance and integrity tuning (`foreign_keys = ON`, `synchronous = NORMAL`).
- Added `CONTRIBUTING.md` and `CODE_OF_CONDUCT.md`.
- Added GitHub issue templates and PR template.

### Changed
- Streamlined repository structure, isolating private preparation guides from public git history.

---

## [1.1.0] - 2026-09-30
### Added
- Integrated Module VIII Stack (`std::stack<Passenger> recentCancellations`) for LIFO cancellation tracking and undo support.
- Added interactive Undo Cancellation feature in main menu (Option 10).
- Cross-platform Makefile for Linux and macOS compilation.

---

## [1.0.0] - 2026-09-30
### Initial Release
- Core railway reservation system matching all Group 4 college requirements.
- Binary Search for train lookup by train number ($O(\log N)$).
- Linear Search for destination with case-insensitive partial substring matching ($O(N)$).
- Manual Bubble Sort on a copy vector for display by fare or name ($O(N^2)$).
- 2D seat map array `seatMap[20][60]` for $O(1)$ random-access seat reservations.
- FIFO waiting queue `map<int, queue<WaitingEntry>>` for full-train handling.
- Automatic FIFO queue promotion upon ticket cancellation.
- SQLite 3 database persistence (`railway.db`) using prepared statements.
