# Contributing to Railway Ticket Reservation System

Thank you for your interest in contributing! This project is an open-source, menu-driven C++11 educational railway reservation system backed by MongoDB Document storage.

---

## Code of Conduct
Please read and follow our [Code of Conduct](CODE_OF_CONDUCT.md) in all project interactions.

---

## How to Contribute

### 1. Reporting Bugs
- Check the [Issues tab](https://github.com/bharathwajverse/railway-ticket-reservation-cpp/issues) to ensure the bug hasn't already been reported.
- Open a new issue using our **Bug Report** template with clear steps to reproduce the issue.

### 2. Suggesting Features
- We welcome suggestions for new DSA features, performance improvements, or terminal UI enhancements.
- Open an issue using our **Feature Request** template describing your proposal and use case.

### 3. Pull Requests
1. **Fork** the repository and create a new branch from `main`:
   ```bash
   git checkout -b feature/your-feature-name
   ```
2. **Adhere to the Project Constraints:**
   - Standard: C++11.
   - Paradigm: Procedural with `struct` and functions (no complex class hierarchies).
   - Only standard containers: `vector`, `queue`, `map`, `stack`, `string`, `set`.
   - All persistence logic must remain encapsulated within the MongoDB database layer (`database.h` / `database.cpp`).
   - Code must compile with `-Wall -Wextra` without warnings.
3. **Commit your changes:**
   ```bash
   git commit -m "feat: description of your feature"
   ```
4. **Push to your fork and submit a Pull Request** to the `main` branch.

---

## Local Build & Verification

### Windows
```cmd
build.bat
railway.exe
```

### Linux / macOS
```bash
make
./railway
```
