// =============================================================================
// menu.h - Console menu display and input handling
// This file IS in the syllabus zone but handles I/O (cin/cout) for the user
// =============================================================================
#ifndef MENU_H
#define MENU_H

#include <string>
#include "structures.h"

// --- Console I/O helpers (these DO use cin/cout) ---
int readInt(const std::string& prompt, int minVal, int maxVal);
float readFloat(const std::string& prompt, float minVal, float maxVal);
std::string readNonEmptyString(const std::string& prompt);
char readGender(const std::string& prompt);
Date readDate(const std::string& prompt);

// --- Menu display and dispatch ---
void printMenu();
void handleChoice(RailwaySystem& sys, int choice);

#endif // MENU_H
