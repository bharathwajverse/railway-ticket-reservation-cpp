#include "../include/structures.h"
#include "../include/config.h"
#include "validation.h"
#include <string>
#include <vector>
#include <sstream>
#include <cctype>

using namespace std;

// Time Complexity: O(1)
// Purpose: Check if a year is a leap year
bool isLeapYear(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

// Time Complexity: O(1)
// Purpose: Validate a date against rules (range 2024-2035)
bool isValidDate(int d, int m, int y) {
    if (y < 2024 || y > 2035) return false;
    if (m < 1 || m > 12) return false;
    int daysPerMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m == 2 && isLeapYear(y)) {
        daysPerMonth[1] = 29;
    }
    if (d < 1 || d > daysPerMonth[m - 1]) return false;
    return true;
}

// Time Complexity: O(N) where N is length of name
// Purpose: Check if name is valid (non-empty, only letters and spaces)
bool isValidName(const string& name) {
    if (name.empty()) return false;
    for (size_t i = 0; i < name.length(); i++) {
        if (!isalpha(name[i]) && name[i] != ' ') {
            return false;
        }
    }
    return true;
}

// Time Complexity: O(1)
// Purpose: Check if age is between 1 and 120
bool isValidAge(int age) {
    return (age >= 1 && age <= 120);
}

// Time Complexity: O(1)
// Purpose: Check if gender is M, F, or O
bool isValidGender(char g) {
    char lowerG = tolower(g);
    return (lowerG == 'm' || lowerG == 'f' || lowerG == 'o');
}

// Time Complexity: O(1)
// Purpose: Calculate concession based on age
void calculateConcession(int age, float baseFare, string& concession, float& finalFare) {
    if (age < 12) {
        concession = "CHILD";
        finalFare = baseFare * 0.5f;
    } else if (age >= 60) {
        concession = "SENIOR";
        finalFare = baseFare * 0.6f;
    } else {
        concession = "GENERAL";
        finalFare = baseFare;
    }
}

// Time Complexity: O(N) where N is length of s
// Purpose: Convert string to lowercase
string toLowerCase(const string& s) {
    string result = s;
    for (size_t i = 0; i < result.length(); i++) {
        result[i] = tolower(result[i]);
    }
    return result;
}

// Time Complexity: O(N * M) where N is text length, M is pattern length
// Purpose: Check if text contains pattern (case-insensitive)
bool containsIgnoreCase(const string& text, const string& pattern) {
    string lowerText = toLowerCase(text);
    string lowerPattern = toLowerCase(pattern);
    return lowerText.find(lowerPattern) != string::npos;
}

// Time Complexity: O(N) where N is string length
// Purpose: Count occurrences of a character in a string
int countCharFrequency(const string& str, char ch) {
    int count = 0;
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == ch) {
            count++;
        }
    }
    return count;
}

// Time Complexity: O(N) where N is string length
// Purpose: Reverse a string
string reverseString(const string& str) {
    string result = "";
    for (int i = (int)str.length() - 1; i >= 0; i--) {
        result += str[i];
    }
    return result;
}

// Time Complexity: O(N) where N is string length
// Purpose: Split string by delimiter
vector<string> tokenizeString(const string& str, char delimiter) {
    vector<string> tokens;
    stringstream ss(str);
    string token;
    while (getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}
