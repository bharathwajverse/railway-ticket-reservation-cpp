// =============================================================================
// validation.h - Input validation and string utility functions
// MODULE II: Control Statements (if-else, loops, functions)
// MODULE V: String manipulation (toLowerCase, frequency, reversal, tokenize)
// =============================================================================
#ifndef VALIDATION_H
#define VALIDATION_H

#include <string>
#include <vector>
#include "structures.h"

// --- Date validation (Module II: decision making) ---
bool isLeapYear(int y);
bool isValidDate(int d, int m, int y);

// --- Input validation helpers (Module II: functions with return values) ---
bool isValidName(const std::string& name);
bool isValidAge(int age);
bool isValidGender(char g);

// --- Concession calculator (Module II: if-else ladder) ---
void calculateConcession(int age, float baseFare,
                         std::string& concession, float& finalFare);

// --- String utilities (Module V: string methods and manipulation) ---
std::string toLowerCase(const std::string& s);
bool containsIgnoreCase(const std::string& text, const std::string& pattern);
int countCharFrequency(const std::string& str, char ch);
std::string reverseString(const std::string& str);
std::vector<std::string> tokenizeString(const std::string& str, char delimiter);

#endif // VALIDATION_H
