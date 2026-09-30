#ifndef UTILS_H
#define UTILS_H

#include <string>
#include "structures.h"

using namespace std;

// Input reading and validation prototypes
int readInt(const string& prompt, int minVal, int maxVal);
float readFloat(const string& prompt, float minVal, float maxVal);
string readName(const string& prompt);
char readGender(const string& prompt);
Date readDate(const string& prompt);

// String helper prototypes
string toLowerCase(const string& str);
bool containsIgnoreCase(const string& text, const string& pattern);
bool isLeapYear(int year);
bool isValidDate(int day, int month, int year);

#endif // UTILS_H
