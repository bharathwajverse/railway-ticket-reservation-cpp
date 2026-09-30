#include "utils.h"
#include <iostream>
#include <cctype>

using namespace std;

// Checks whether a given year is a leap year
// Time Complexity: O(1)
bool isLeapYear(int year) {
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        return true;
    }
    return false;
}

// Validates whether day, month, and year form a legitimate calendar date
// Time Complexity: O(1)
bool isValidDate(int day, int month, int year) {
    if (year < 2024 || year > 2035) {
        return false;
    }
    if (month < 1 || month > 12) {
        return false;
    }
    int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month == 2 && isLeapYear(year)) {
        daysInMonth[1] = 29;
    }
    if (day < 1 || day > daysInMonth[month - 1]) {
        return false;
    }
    return true;
}

// Converts a string to lowercase character-by-character
// Time Complexity: O(n) where n is length of string
string toLowerCase(const string& str) {
    string result = "";
    for (size_t i = 0; i < str.length(); i++) {
        result += (char)tolower(str[i]);
    }
    return result;
}

// Performs case-insensitive substring search (manual pattern matching)
// Time Complexity: O(n * m) where n is text length and m is pattern length
bool containsIgnoreCase(const string& text, const string& pattern) {
    string lowerText = toLowerCase(text);
    string lowerPattern = toLowerCase(pattern);

    if (lowerPattern.length() == 0) {
        return true;
    }
    if (lowerPattern.length() > lowerText.length()) {
        return false;
    }

    for (size_t i = 0; i <= lowerText.length() - lowerPattern.length(); i++) {
        bool match = true;
        for (size_t j = 0; j < lowerPattern.length(); j++) {
            if (lowerText[i + j] != lowerPattern[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            return true;
        }
    }
    return false;
}

// Reads an integer within [minVal, maxVal] and handles non-numeric input gracefully
// Time Complexity: O(1) per valid attempt
int readInt(const string& prompt, int minVal, int maxVal) {
    int value = 0;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            if (value >= minVal && value <= maxVal) {
                // Clear any remaining characters up to newline
                cin.ignore(10000, '\n');
                return value;
            } else {
                cout << "[Error] Input must be between " << minVal << " and " << maxVal << ". Try again.\n";
            }
        } else {
            cout << "[Error] Invalid input. Please enter a valid whole number.\n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }
}

// Reads a float within [minVal, maxVal] and handles non-numeric input gracefully
// Time Complexity: O(1) per valid attempt
float readFloat(const string& prompt, float minVal, float maxVal) {
    float value = 0.0f;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            if (value >= minVal && value <= maxVal) {
                cin.ignore(10000, '\n');
                return value;
            } else {
                cout << "[Error] Value must be between " << minVal << " and " << maxVal << ". Try again.\n";
            }
        } else {
            cout << "[Error] Invalid input. Please enter a valid decimal number.\n";
            cin.clear();
            cin.ignore(10000, '\n');
        }
    }
}

// Reads a person's name ensuring only letters and spaces, non-empty
// Time Complexity: O(n) where n is length of name
string readName(const string& prompt) {
    string name = "";
    while (true) {
        cout << prompt;
        getline(cin, name);

        // Trim leading and trailing spaces manually
        size_t start = 0;
        while (start < name.length() && isspace(name[start])) {
            start++;
        }
        size_t end = name.length();
        while (end > start && isspace(name[end - 1])) {
            end--;
        }

        if (start >= end) {
            cout << "[Error] Name cannot be blank. Try again.\n";
            continue;
        }

        string trimmedName = name.substr(start, end - start);
        bool allLetters = true;
        for (size_t i = 0; i < trimmedName.length(); i++) {
            if (!isalpha(trimmedName[i]) && !isspace(trimmedName[i])) {
                allLetters = false;
                break;
            }
        }

        if (!allLetters) {
            cout << "[Error] Name must contain only letters and spaces. Try again.\n";
            continue;
        }

        return trimmedName;
    }
}

// Reads gender (M, F, or O)
// Time Complexity: O(1) per attempt
char readGender(const string& prompt) {
    string input = "";
    while (true) {
        cout << prompt;
        getline(cin, input);
        if (input.length() == 1) {
            char g = (char)toupper(input[0]);
            if (g == 'M' || g == 'F' || g == 'O') {
                return g;
            }
        }
        cout << "[Error] Please enter 'M' for Male, 'F' for Female, or 'O' for Other.\n";
    }
}

// Reads a valid calendar date
// Time Complexity: O(1)
Date readDate(const string& prompt) {
    Date d;
    cout << prompt << "\n";
    while (true) {
        d.day = readInt("  Enter Day (1-31): ", 1, 31);
        d.month = readInt("  Enter Month (1-12): ", 1, 12);
        d.year = readInt("  Enter Year (2024-2035): ", 2024, 2035);

        if (isValidDate(d.day, d.month, d.year)) {
            return d;
        }
        cout << "[Error] Invalid calendar date (" << d.day << "/" << d.month << "/" << d.year 
             << ")! Check month days and leap years. Please re-enter.\n";
    }
}
