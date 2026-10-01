#include "db_connection.h"
#include <iostream>
#include <fstream>
#include <sstream>
#ifdef _WIN32
#include <direct.h>
#define MAKE_DIR(d) _mkdir(d)
#else
#include <sys/stat.h>
#define MAKE_DIR(d) mkdir(d, 0755)
#endif

using namespace std;

static string g_dataDir = "";

// Connect to data directory (creates it if needed). Returns true on success.
bool dbConnect(const string& dataDir) {
    g_dataDir = dataDir;
    MAKE_DIR(dataDir.c_str());
    
    string files[] = {"/trains.json", "/passengers.json", "/waiting_list.json"};
    for (const string& f : files) {
        string path = dataDir + f;
        ifstream infile(path);
        if (!infile.good()) {
            ofstream outfile(path);
            outfile << "[]";
            outfile.close();
        }
        infile.close();
    }
    return true;
}

// Close / cleanup.
void dbClose() {
    // No-op for now
}

// Get the data directory path.
string dbGetDataDir() {
    return g_dataDir;
}
