#include "passenger_repo.h"
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

// Serialize a Passenger to a JSON object string
static string passengerToJson(const Passenger& p) {
    stringstream ss;
    ss << "{\"pnr\":" << p.pnr 
       << ",\"name\":\"" << p.name << "\""
       << ",\"age\":" << p.age
       << ",\"gender\":\"" << string(1, p.gender) << "\""
       << ",\"trainNo\":" << p.trainNo
       << ",\"seatNo\":" << p.seatNo
       << ",\"travelDate\":{\"day\":" << p.travelDate.day 
       << ",\"month\":" << p.travelDate.month
       << ",\"year\":" << p.travelDate.year << "}"
       << ",\"status\":\"" << p.status << "\"}";
    return ss.str();
}

// Parse a JSON object string to a Passenger struct
static Passenger jsonToPassenger(const string& json) {
    Passenger p;
    p.pnr = getJsonIntValue(json, "pnr");
    p.name = getJsonStringValue(json, "name");
    p.age = getJsonIntValue(json, "age");
    string genderStr = getJsonStringValue(json, "gender");
    p.gender = genderStr.empty() ? ' ' : genderStr[0];
    p.trainNo = getJsonIntValue(json, "trainNo");
    p.seatNo = getJsonIntValue(json, "seatNo");
    
    string searchDate = "\"travelDate\":";
    size_t pos = json.find(searchDate);
    if (pos != string::npos) {
        string subJson = json.substr(pos);
        p.travelDate.day = getJsonIntValue(subJson, "day");
        p.travelDate.month = getJsonIntValue(subJson, "month");
        p.travelDate.year = getJsonIntValue(subJson, "year");
    } else {
        p.travelDate = {0, 0, 0};
    }
    
    p.status = getJsonStringValue(json, "status");
    return p;
}

// Insert a passenger into the database
bool dbInsertPassenger(const Passenger& p) {
    vector<Passenger> passengers;
    dbLoadPassengers(passengers);
    passengers.push_back(p);
    
    ofstream out(dbGetDataDir() + "/passengers.json");
    if (!out.is_open()) return false;
    out << "[";
    for (size_t i = 0; i < passengers.size(); ++i) {
        out << passengerToJson(passengers[i]);
        if (i < passengers.size() - 1) out << ",";
    }
    out << "]";
    return true;
}

// Update a passenger's status
bool dbUpdatePassengerStatus(int pnr, const string& status) {
    vector<Passenger> passengers;
    if (!dbLoadPassengers(passengers)) return false;
    bool found = false;
    for (Passenger& p : passengers) {
        if (p.pnr == pnr) {
            p.status = status;
            found = true;
            break;
        }
    }
    if (!found) return false;
    
    ofstream out(dbGetDataDir() + "/passengers.json");
    if (!out.is_open()) return false;
    out << "[";
    for (size_t i = 0; i < passengers.size(); ++i) {
        out << passengerToJson(passengers[i]);
        if (i < passengers.size() - 1) out << ",";
    }
    out << "]";
    return true;
}

// Load all passengers from database
bool dbLoadPassengers(vector<Passenger>& passengers) {
    passengers.clear();
    ifstream in(dbGetDataDir() + "/passengers.json");
    if (!in.is_open()) return false;
    stringstream buffer;
    buffer << in.rdbuf();
    string content = buffer.str();
    
    vector<string> objects = splitJsonArray(content);
    for (const string& obj : objects) {
        passengers.push_back(jsonToPassenger(obj));
    }
    return true;
}
