#ifndef DB_CONNECTION_H
#define DB_CONNECTION_H
#include <string>

// Connect to data directory (creates it if needed). Returns true on success.
bool dbConnect(const std::string& dataDir);
// Close / cleanup.
void dbClose();
// Get the data directory path.
std::string dbGetDataDir();

#endif
