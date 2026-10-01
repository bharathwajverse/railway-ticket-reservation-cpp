#include "database.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <iomanip>

#ifdef _WIN32
#include <direct.h>
#define MAKE_DIR(d) _mkdir(d)
#else
#include <sys/stat.h>
#define MAKE_DIR(d) mkdir(d, 0755)
#endif

using namespace std;

static string g_dataDir = "mongodb_data";

// Helper JSON parser utilities for MongoDB Documents
static string getJsonValue(const string& doc, const string& key) {
    string target = "\"" + key + "\"";
    size_t pos = doc.find(target);
    if (pos == string::npos) return "";
    pos += target.length();
    while (pos < doc.length() && (doc[pos] == ' ' || doc[pos] == ':' || doc[pos] == '\t')) pos++;
    if (pos >= doc.length()) return "";
    if (doc[pos] == '\"') {
        size_t endQuote = doc.find('\"', pos + 1);
        if (endQuote == string::npos) return "";
        return doc.substr(pos + 1, endQuote - pos - 1);
    } else {
        size_t endVal = pos;
        while (endVal < doc.length() && doc[endVal] != ',' && doc[endVal] != '}' && doc[endVal] != '\n' && doc[endVal] != '\r') {
            endVal++;
        }
        string val = doc.substr(pos, endVal - pos);
        size_t s = val.find_first_not_of(" \t\r\n");
        size_t e = val.find_last_not_of(" \t\r\n");
        if (s != string::npos && e != string::npos) return val.substr(s, e - s + 1);
        return "";
    }
}

static string getJsonSubObject(const string& doc, const string& key) {
    string target = "\"" + key + "\"";
    size_t pos = doc.find(target);
    if (pos == string::npos) return "";
    size_t braceStart = doc.find('{', pos);
    if (braceStart == string::npos) return "";
    int depth = 0;
    bool inStr = false;
    for (size_t i = braceStart; i < doc.length(); i++) {
        char c = doc[i];
        if (c == '\"' && (i == 0 || doc[i - 1] != '\\')) {
            inStr = !inStr;
        } else if (!inStr) {
            if (c == '{') depth++;
            else if (c == '}') {
                depth--;
                if (depth == 0) return doc.substr(braceStart, i - braceStart + 1);
            }
        }
    }
    return "";
}

static vector<string> extractDocBlocks(const string& jsonContent) {
    vector<string> blocks;
    int depth = 0;
    size_t start = 0;
    bool inStr = false;
    for (size_t i = 0; i < jsonContent.length(); i++) {
        char c = jsonContent[i];
        if (c == '\"' && (i == 0 || jsonContent[i - 1] != '\\')) {
            inStr = !inStr;
        } else if (!inStr) {
            if (c == '{') {
                if (depth == 0) start = i;
                depth++;
            } else if (c == '}') {
                depth--;
                if (depth == 0 && start <= i) {
                    blocks.push_back(jsonContent.substr(start, i - start + 1));
                }
            }
        }
    }
    return blocks;
}

static string readFileContent(const string& filePath) {
    ifstream fin(filePath.c_str());
    if (!fin.is_open()) return "";
    stringstream ss;
    ss << fin.rdbuf();
    return ss.str();
}

static bool writeFileContent(const string& filePath, const string& content) {
    ofstream fout(filePath.c_str());
    if (!fout.is_open()) return false;
    fout << content;
    fout.close();
    return true;
}

bool dbOpen(const string& dataDirectory) {
    g_dataDir = dataDirectory;
    MAKE_DIR(g_dataDir.c_str());
    return true;
}

void dbClose() {
    // Generate fresh MongoDB Shell Seed script upon closing
    dbExportMongoScript();
}

bool dbCreateCollections() {
    MAKE_DIR(g_dataDir.c_str());
    string trainsPath = g_dataDir + "/trains.json";
    string passengersPath = g_dataDir + "/passengers.json";
    string waitingPath = g_dataDir + "/waiting_list.json";

    ifstream f1(trainsPath.c_str());
    if (!f1.is_open()) writeFileContent(trainsPath, "[\n]\n");
    f1.close();

    ifstream f2(passengersPath.c_str());
    if (!f2.is_open()) writeFileContent(passengersPath, "[\n]\n");
    f2.close();

    ifstream f3(waitingPath.c_str());
    if (!f3.is_open()) writeFileContent(waitingPath, "[\n]\n");
    f3.close();

    return true;
}

// ============================================================================
// TRAINS COLLECTION (trains.json)
// ============================================================================

bool dbLoadTrains(vector<Train>& trains) {
    trains.clear();
    string path = g_dataDir + "/trains.json";
    string content = readFileContent(path);
    if (content.empty()) return true;

    vector<string> docs = extractDocBlocks(content);
    for (size_t i = 0; i < docs.size(); i++) {
        const string& d = docs[i];
        string tNoStr = getJsonValue(d, "train_no");
        if (tNoStr.empty()) continue;

        Train t;
        t.trainNo = atoi(tNoStr.c_str());
        t.name = getJsonValue(d, "name");
        t.source = getJsonValue(d, "source");
        t.destination = getJsonValue(d, "destination");
        t.departure = getJsonValue(d, "departure");
        t.totalSeats = atoi(getJsonValue(d, "total_seats").c_str());
        t.availableSeats = atoi(getJsonValue(d, "available_seats").c_str());
        t.fare = (float)atof(getJsonValue(d, "fare").c_str());
        trains.push_back(t);
    }
    return true;
}

static bool saveTrainsCollection(const vector<Train>& trains) {
    stringstream ss;
    ss << "[\n";
    for (size_t i = 0; i < trains.size(); i++) {
        const Train& t = trains[i];
        ss << "  {\n"
           << "    \"_id\": \"6701a1b2c3d4e5f6000000" << setfill('0') << setw(2) << (i + 1) << "\",\n"
           << "    \"train_no\": " << t.trainNo << ",\n"
           << "    \"name\": \"" << t.name << "\",\n"
           << "    \"source\": \"" << t.source << "\",\n"
           << "    \"destination\": \"" << t.destination << "\",\n"
           << "    \"departure\": \"" << t.departure << "\",\n"
           << "    \"total_seats\": " << t.totalSeats << ",\n"
           << "    \"available_seats\": " << t.availableSeats << ",\n"
           << "    \"fare\": " << fixed << setprecision(2) << t.fare << "\n"
           << "  }" << (i + 1 < trains.size() ? "," : "") << "\n";
    }
    ss << "]\n";
    return writeFileContent(g_dataDir + "/trains.json", ss.str());
}

bool dbInsertTrain(const Train& t) {
    vector<Train> trains;
    dbLoadTrains(trains);
    for (size_t i = 0; i < trains.size(); i++) {
        if (trains[i].trainNo == t.trainNo) return false;
    }
    trains.push_back(t);
    return saveTrainsCollection(trains);
}

bool dbUpdateTrainSeats(int trainNo, int availableSeats) {
    vector<Train> trains;
    dbLoadTrains(trains);
    bool updated = false;
    for (size_t i = 0; i < trains.size(); i++) {
        if (trains[i].trainNo == trainNo) {
            trains[i].availableSeats = availableSeats;
            updated = true;
            break;
        }
    }
    if (updated) return saveTrainsCollection(trains);
    return false;
}

// ============================================================================
// PASSENGERS COLLECTION (passengers.json)
// ============================================================================

bool dbLoadPassengers(vector<Passenger>& passengers) {
    passengers.clear();
    string path = g_dataDir + "/passengers.json";
    string content = readFileContent(path);
    if (content.empty()) return true;

    vector<string> docs = extractDocBlocks(content);
    for (size_t i = 0; i < docs.size(); i++) {
        const string& d = docs[i];
        string pnrStr = getJsonValue(d, "pnr");
        if (pnrStr.empty()) continue;

        Passenger p;
        p.pnr = atoi(pnrStr.c_str());
        p.name = getJsonValue(d, "name");
        p.age = atoi(getJsonValue(d, "age").c_str());
        string gStr = getJsonValue(d, "gender");
        p.gender = gStr.empty() ? 'M' : gStr[0];
        p.trainNo = atoi(getJsonValue(d, "train_no").c_str());
        p.seatNo = atoi(getJsonValue(d, "seat_no").c_str());

        string dateSub = getJsonSubObject(d, "travel_date");
        p.travelDate.day = atoi(getJsonValue(dateSub, "day").c_str());
        p.travelDate.month = atoi(getJsonValue(dateSub, "month").c_str());
        p.travelDate.year = atoi(getJsonValue(dateSub, "year").c_str());

        p.status = getJsonValue(d, "status");
        p.concession = getJsonValue(d, "concession");
        if (p.concession.empty()) p.concession = "GENERAL";
        p.farePaid = (float)atof(getJsonValue(d, "fare_paid").c_str());

        passengers.push_back(p);
    }
    return true;
}

static bool savePassengersCollection(const vector<Passenger>& passengers) {
    stringstream ss;
    ss << "[\n";
    for (size_t i = 0; i < passengers.size(); i++) {
        const Passenger& p = passengers[i];
        ss << "  {\n"
           << "    \"_id\": \"6701a1b2c3d4e5f6000001" << setfill('0') << setw(2) << (i + 1) << "\",\n"
           << "    \"pnr\": " << p.pnr << ",\n"
           << "    \"name\": \"" << p.name << "\",\n"
           << "    \"age\": " << p.age << ",\n"
           << "    \"gender\": \"" << p.gender << "\",\n"
           << "    \"train_no\": " << p.trainNo << ",\n"
           << "    \"seat_no\": " << p.seatNo << ",\n"
           << "    \"travel_date\": {\n"
           << "      \"day\": " << p.travelDate.day << ",\n"
           << "      \"month\": " << p.travelDate.month << ",\n"
           << "      \"year\": " << p.travelDate.year << "\n"
           << "    },\n"
           << "    \"status\": \"" << p.status << "\",\n"
           << "    \"concession\": \"" << p.concession << "\",\n"
           << "    \"fare_paid\": " << fixed << setprecision(2) << p.farePaid << "\n"
           << "  }" << (i + 1 < passengers.size() ? "," : "") << "\n";
    }
    ss << "]\n";
    return writeFileContent(g_dataDir + "/passengers.json", ss.str());
}

bool dbInsertPassenger(const Passenger& p) {
    vector<Passenger> passengers;
    dbLoadPassengers(passengers);
    passengers.push_back(p);
    return savePassengersCollection(passengers);
}

bool dbUpdatePassengerStatus(int pnr, const string& status) {
    vector<Passenger> passengers;
    dbLoadPassengers(passengers);
    bool updated = false;
    for (size_t i = 0; i < passengers.size(); i++) {
        if (passengers[i].pnr == pnr) {
            passengers[i].status = status;
            updated = true;
            break;
        }
    }
    if (updated) return savePassengersCollection(passengers);
    return false;
}

// ============================================================================
// WAITING LIST COLLECTION (waiting_list.json)
// ============================================================================

bool dbLoadWaiting(vector<WaitingEntry>& waitingList) {
    waitingList.clear();
    string path = g_dataDir + "/waiting_list.json";
    string content = readFileContent(path);
    if (content.empty()) return true;

    vector<string> docs = extractDocBlocks(content);
    for (size_t i = 0; i < docs.size(); i++) {
        const string& d = docs[i];
        string wIdStr = getJsonValue(d, "wait_id");
        if (wIdStr.empty()) continue;

        WaitingEntry w;
        w.waitId = atoi(wIdStr.c_str());
        w.name = getJsonValue(d, "name");
        w.age = atoi(getJsonValue(d, "age").c_str());
        string gStr = getJsonValue(d, "gender");
        w.gender = gStr.empty() ? 'M' : gStr[0];
        w.trainNo = atoi(getJsonValue(d, "train_no").c_str());

        string dateSub = getJsonSubObject(d, "travel_date");
        w.travelDate.day = atoi(getJsonValue(dateSub, "day").c_str());
        w.travelDate.month = atoi(getJsonValue(dateSub, "month").c_str());
        w.travelDate.year = atoi(getJsonValue(dateSub, "year").c_str());

        waitingList.push_back(w);
    }
    return true;
}

static bool saveWaitingCollection(const vector<WaitingEntry>& waitingList) {
    stringstream ss;
    ss << "[\n";
    for (size_t i = 0; i < waitingList.size(); i++) {
        const WaitingEntry& w = waitingList[i];
        ss << "  {\n"
           << "    \"_id\": \"6701a1b2c3d4e5f6000002" << setfill('0') << setw(2) << (i + 1) << "\",\n"
           << "    \"wait_id\": " << w.waitId << ",\n"
           << "    \"name\": \"" << w.name << "\",\n"
           << "    \"age\": " << w.age << ",\n"
           << "    \"gender\": \"" << w.gender << "\",\n"
           << "    \"train_no\": " << w.trainNo << ",\n"
           << "    \"travel_date\": {\n"
           << "      \"day\": " << w.travelDate.day << ",\n"
           << "      \"month\": " << w.travelDate.month << ",\n"
           << "      \"year\": " << w.travelDate.year << "\n"
           << "    }\n"
           << "  }" << (i + 1 < waitingList.size() ? "," : "") << "\n";
    }
    ss << "]\n";
    return writeFileContent(g_dataDir + "/waiting_list.json", ss.str());
}

bool dbInsertWaiting(WaitingEntry& w) {
    vector<WaitingEntry> list;
    dbLoadWaiting(list);
    int maxId = 0;
    for (size_t i = 0; i < list.size(); i++) {
        if (list[i].waitId > maxId) maxId = list[i].waitId;
    }
    w.waitId = maxId + 1;
    list.push_back(w);
    return saveWaitingCollection(list);
}

bool dbDeleteWaiting(int waitId) {
    vector<WaitingEntry> list;
    dbLoadWaiting(list);
    bool found = false;
    vector<WaitingEntry> remaining;
    for (size_t i = 0; i < list.size(); i++) {
        if (list[i].waitId == waitId) {
            found = true;
        } else {
            remaining.push_back(list[i]);
        }
    }
    if (found) return saveWaitingCollection(remaining);
    return false;
}

// ============================================================================
// MONGOSH COMPATIBLE SCRIPT GENERATION (mongo_seed.js)
// ============================================================================
// MONGODB ATLAS CONFIGURATION & SCRIPT GENERATION
// Target Database: datadb | Cluster: cluster0.xhjfpv2.mongodb.net
// ============================================================================

string dbGetAtlasUri() {
    ifstream fin("mongodb.conf");
    if (fin.is_open()) {
        string line;
        while (getline(fin, line)) {
            if (line.find("MONGODB_URI=") == 0) {
                return line.substr(12);
            }
        }
        fin.close();
    }
    return "mongodb+srv://system:system@cluster0.xhjfpv2.mongodb.net/datadb?appName=Cluster0";
}

string dbGetDatabaseName() {
    ifstream fin("mongodb.conf");
    if (fin.is_open()) {
        string line;
        while (getline(fin, line)) {
            if (line.find("MONGODB_DATABASE=") == 0) {
                return line.substr(17);
            }
        }
        fin.close();
    }
    return "datadb";
}

bool dbExportMongoScript(const string& scriptFileName) {
    vector<Train> trains;
    vector<Passenger> passengers;
    vector<WaitingEntry> waiting;
    dbLoadTrains(trains);
    dbLoadPassengers(passengers);
    dbLoadWaiting(waiting);

    string dbName = dbGetDatabaseName();
    string atlasUri = dbGetAtlasUri();

    stringstream ss;
    ss << "// =============================================================================\n"
       << "// MongoDB Atlas Initialization Script (mongosh compatible)\n"
       << "// Database: " << dbName << "\n"
       << "// Cluster:  cluster0.xhjfpv2.mongodb.net\n"
       << "// Usage:    mongosh \"" << atlasUri << "\" " << scriptFileName << "\n"
       << "// =============================================================================\n\n"
       << "use('" << dbName << "');\n\n"
       << "// 1. Reset Collections\n"
       << "db.trains.drop();\n"
       << "db.passengers.drop();\n"
       << "db.waiting_list.drop();\n\n"
       << "// 2. Create Unique Indexes\n"
       << "db.trains.createIndex({ \"train_no\": 1 }, { unique: true });\n"
       << "db.passengers.createIndex({ \"pnr\": 1 }, { unique: true });\n"
       << "db.waiting_list.createIndex({ \"wait_id\": 1 }, { unique: true });\n\n"
       << "// 3. Insert Trains Collection\n"
       << "db.trains.insertMany([\n";

    for (size_t i = 0; i < trains.size(); i++) {
        const Train& t = trains[i];
        ss << "  { \"train_no\": " << t.trainNo
           << ", \"name\": \"" << t.name << "\""
           << ", \"source\": \"" << t.source << "\""
           << ", \"destination\": \"" << t.destination << "\""
           << ", \"departure\": \"" << t.departure << "\""
           << ", \"total_seats\": " << t.totalSeats
           << ", \"available_seats\": " << t.availableSeats
           << ", \"fare\": " << fixed << setprecision(2) << t.fare << " }"
           << (i + 1 < trains.size() ? ",\n" : "\n");
    }
    ss << "]);\n\n";

    if (!passengers.empty()) {
        ss << "// 4. Insert Passengers Collection\n"
           << "db.passengers.insertMany([\n";
        for (size_t i = 0; i < passengers.size(); i++) {
            const Passenger& p = passengers[i];
            ss << "  { \"pnr\": " << p.pnr
               << ", \"name\": \"" << p.name << "\""
               << ", \"age\": " << p.age
               << ", \"gender\": \"" << p.gender << "\""
               << ", \"train_no\": " << p.trainNo
               << ", \"seat_no\": " << p.seatNo
               << ", \"travel_date\": { \"day\": " << p.travelDate.day
               << ", \"month\": " << p.travelDate.month
               << ", \"year\": " << p.travelDate.year << " }"
               << ", \"status\": \"" << p.status << "\""
               << ", \"concession\": \"" << p.concession << "\""
               << ", \"fare_paid\": " << fixed << setprecision(2) << p.farePaid << " }"
               << (i + 1 < passengers.size() ? ",\n" : "\n");
        }
        ss << "]);\n\n";
    }

    if (!waiting.empty()) {
        ss << "// 5. Insert Waiting List Collection\n"
           << "db.waiting_list.insertMany([\n";
        for (size_t i = 0; i < waiting.size(); i++) {
            const WaitingEntry& w = waiting[i];
            ss << "  { \"wait_id\": " << w.waitId
               << ", \"name\": \"" << w.name << "\""
               << ", \"age\": " << w.age
               << ", \"gender\": \"" << w.gender << "\""
               << ", \"train_no\": " << w.trainNo
               << ", \"travel_date\": { \"day\": " << w.travelDate.day
               << ", \"month\": " << w.travelDate.month
               << ", \"year\": " << w.travelDate.year << " } }"
               << (i + 1 < waiting.size() ? ",\n" : "\n");
        }
        ss << "]);\n\n";
    }

    ss << "print('>>> Successfully synchronized to MongoDB Atlas database: " << dbName << "');\n";
    return writeFileContent(scriptFileName, ss.str());
}

bool dbSyncToAtlas() {
    dbExportMongoScript("mongo_seed.js");

    string mongoshCmd = "mongosh";
    ifstream testMongosh("G:\\mongosh-2.10.0-win32-x64\\mongosh-2.10.0-win32-x64\\bin\\mongosh.exe");
    if (testMongosh.is_open()) {
        mongoshCmd = "\"G:\\mongosh-2.10.0-win32-x64\\mongosh-2.10.0-win32-x64\\bin\\mongosh.exe\"";
        testMongosh.close();
    }

    string uri = dbGetAtlasUri();
    string cmd = mongoshCmd + " \"" + uri + "\" mongo_seed.js";
#ifdef _WIN32
    string fullCmd = "\"" + cmd + "\"";
#else
    string fullCmd = cmd;
#endif
    cout << "\n[MongoDB Atlas Sync] Connecting to cluster: cluster0.xhjfpv2.mongodb.net (database: "
         << dbGetDatabaseName() << ")...\n";
    int ret = system(fullCmd.c_str());
    return (ret == 0);
}

