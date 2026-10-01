#include "train_repo.h"
#include "db_connection.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

// Extract string value from JSON string
static string getJsonStringValue(const string& doc, const string& key) {
    string search = "\"" + key + "\":";
    size_t pos = doc.find(search);
    if (pos == string::npos) return "";
    pos += search.length();
    while (pos < doc.length() && (doc[pos] == ' ' || doc[pos] == '\t')) pos++;
    if (pos < doc.length() && doc[pos] == '\"') {
        size_t endPos = doc.find('\"', pos + 1);
        if (endPos != string::npos) {
            return doc.substr(pos + 1, endPos - pos - 1);
        }
    }
    return "";
}

// Extract int value from JSON string
static int getJsonIntValue(const string& doc, const string& key) {
    string search = "\"" + key + "\":";
    size_t pos = doc.find(search);
    if (pos == string::npos) return 0;
    pos += search.length();
    while (pos < doc.length() && (doc[pos] == ' ' || doc[pos] == '\t')) pos++;
    size_t endPos = pos;
    while (endPos < doc.length() && (isdigit(doc[endPos]) || doc[endPos] == '-')) endPos++;
    if (endPos > pos) {
        return stoi(doc.substr(pos, endPos - pos));
    }
    return 0;
}

// Extract float value from JSON string
static float getJsonFloatValue(const string& doc, const string& key) {
    string search = "\"" + key + "\":";
    size_t pos = doc.find(search);
    if (pos == string::npos) return 0.0f;
    pos += search.length();
    while (pos < doc.length() && (doc[pos] == ' ' || doc[pos] == '\t')) pos++;
    size_t endPos = pos;
    while (endPos < doc.length() && (isdigit(doc[endPos]) || doc[endPos] == '-' || doc[endPos] == '.')) endPos++;
    if (endPos > pos) {
        return stof(doc.substr(pos, endPos - pos));
    }
    return 0.0f;
}

// Split JSON array string into individual object strings
static vector<string> splitJsonArray(const string& content) {
    vector<string> result;
    int braceCount = 0;
    string current = "";
    bool inString = false;
    for (char c : content) {
        if (c == '\"') inString = !inString;
        if (!inString) {
            if (c == '{') braceCount++;
            else if (c == '}') braceCount--;
        }
        if (braceCount > 0 || (braceCount == 0 && c == '}')) {
            current += c;
        }
        if (braceCount == 0 && c == '}' && current.length() > 0) {
            result.push_back(current);
            current = "";
        }
    }
    return result;
}

// Serialize a Train to a JSON object string
static string trainToJson(const Train& t) {
    stringstream ss;
    ss << "{\"trainNo\":" << t.trainNo 
       << ",\"name\":\"" << t.name << "\""
       << ",\"source\":\"" << t.source << "\""
       << ",\"destination\":\"" << t.destination << "\""
       << ",\"departure\":\"" << t.departure << "\""
       << ",\"totalSeats\":" << t.totalSeats
       << ",\"availableSeats\":" << t.availableSeats
       << ",\"fare\":" << t.fare << "}";
    return ss.str();
}

// Parse a JSON object string to a Train struct
static Train jsonToTrain(const string& json) {
    Train t;
    t.trainNo = getJsonIntValue(json, "trainNo");
    t.name = getJsonStringValue(json, "name");
    t.source = getJsonStringValue(json, "source");
    t.destination = getJsonStringValue(json, "destination");
    t.departure = getJsonStringValue(json, "departure");
    t.totalSeats = getJsonIntValue(json, "totalSeats");
    t.availableSeats = getJsonIntValue(json, "availableSeats");
    t.fare = getJsonFloatValue(json, "fare");
    return t;
}

// Insert a train into the database
bool dbInsertTrain(const Train& t) {
    vector<Train> trains;
    dbLoadTrains(trains);
    trains.push_back(t);
    
    ofstream out(dbGetDataDir() + "/trains.json");
    if (!out.is_open()) return false;
    out << "[";
    for (size_t i = 0; i < trains.size(); ++i) {
        out << trainToJson(trains[i]);
        if (i < trains.size() - 1) out << ",";
    }
    out << "]";
    return true;
}

// Update available seats for a train
bool dbUpdateTrainSeats(int trainNo, int availableSeats) {
    vector<Train> trains;
    if (!dbLoadTrains(trains)) return false;
    bool found = false;
    for (Train& t : trains) {
        if (t.trainNo == trainNo) {
            t.availableSeats = availableSeats;
            found = true;
            break;
        }
    }
    if (!found) return false;
    
    ofstream out(dbGetDataDir() + "/trains.json");
    if (!out.is_open()) return false;
    out << "[";
    for (size_t i = 0; i < trains.size(); ++i) {
        out << trainToJson(trains[i]);
        if (i < trains.size() - 1) out << ",";
    }
    out << "]";
    return true;
}

// Load all trains from database
bool dbLoadTrains(vector<Train>& trains) {
    trains.clear();
    ifstream in(dbGetDataDir() + "/trains.json");
    if (!in.is_open()) return false;
    stringstream buffer;
    buffer << in.rdbuf();
    string content = buffer.str();
    
    vector<string> objects = splitJsonArray(content);
    for (const string& obj : objects) {
        trains.push_back(jsonToTrain(obj));
    }
    
    sort(trains.begin(), trains.end(), [](const Train& a, const Train& b) {
        return a.trainNo < b.trainNo;
    });
    
    return true;
}
