#include "waiting_repo.h"
#include "db_connection.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>

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

// Serialize a WaitingEntry to a JSON object string
static string waitingToJson(const WaitingEntry& w) {
    stringstream ss;
    ss << "{\"waitId\":" << w.waitId 
       << ",\"name\":\"" << w.name << "\""
       << ",\"age\":" << w.age
       << ",\"gender\":\"" << string(1, w.gender) << "\""
       << ",\"trainNo\":" << w.trainNo
       << ",\"travelDate\":{\"day\":" << w.travelDate.day 
       << ",\"month\":" << w.travelDate.month
       << ",\"year\":" << w.travelDate.year << "}}";
    return ss.str();
}

// Parse a JSON object string to a WaitingEntry struct
static WaitingEntry jsonToWaiting(const string& json) {
    WaitingEntry w;
    w.waitId = getJsonIntValue(json, "waitId");
    w.name = getJsonStringValue(json, "name");
    w.age = getJsonIntValue(json, "age");
    string genderStr = getJsonStringValue(json, "gender");
    w.gender = genderStr.empty() ? ' ' : genderStr[0];
    w.trainNo = getJsonIntValue(json, "trainNo");
    
    string searchDate = "\"travelDate\":";
    size_t pos = json.find(searchDate);
    if (pos != string::npos) {
        string subJson = json.substr(pos);
        w.travelDate.day = getJsonIntValue(subJson, "day");
        w.travelDate.month = getJsonIntValue(subJson, "month");
        w.travelDate.year = getJsonIntValue(subJson, "year");
    } else {
        w.travelDate = {0, 0, 0};
    }
    return w;
}

// Insert a waiting entry into the database
bool dbInsertWaiting(const WaitingEntry& w) {
    vector<WaitingEntry> entries;
    ifstream in(dbGetDataDir() + "/waiting_list.json");
    if (in.is_open()) {
        stringstream buffer;
        buffer << in.rdbuf();
        vector<string> objects = splitJsonArray(buffer.str());
        for (const string& obj : objects) {
            entries.push_back(jsonToWaiting(obj));
        }
        in.close();
    }
    
    entries.push_back(w);
    
    ofstream out(dbGetDataDir() + "/waiting_list.json");
    if (!out.is_open()) return false;
    out << "[";
    for (size_t i = 0; i < entries.size(); ++i) {
        out << waitingToJson(entries[i]);
        if (i < entries.size() - 1) out << ",";
    }
    out << "]";
    return true;
}

// Delete a waiting entry by ID
bool dbDeleteWaiting(int waitId) {
    vector<WaitingEntry> entries;
    ifstream in(dbGetDataDir() + "/waiting_list.json");
    if (in.is_open()) {
        stringstream buffer;
        buffer << in.rdbuf();
        vector<string> objects = splitJsonArray(buffer.str());
        for (const string& obj : objects) {
            entries.push_back(jsonToWaiting(obj));
        }
        in.close();
    }
    
    bool found = false;
    vector<WaitingEntry> newEntries;
    for (const WaitingEntry& w : entries) {
        if (w.waitId == waitId) {
            found = true;
        } else {
            newEntries.push_back(w);
        }
    }
    if (!found) return false;
    
    ofstream out(dbGetDataDir() + "/waiting_list.json");
    if (!out.is_open()) return false;
    out << "[";
    for (size_t i = 0; i < newEntries.size(); ++i) {
        out << waitingToJson(newEntries[i]);
        if (i < newEntries.size() - 1) out << ",";
    }
    out << "]";
    return true;
}

// Load all waiting entries into train-specific queues
bool dbLoadWaiting(map<int, queue<WaitingEntry> >& waitingLists) {
    waitingLists.clear();
    vector<WaitingEntry> entries;
    ifstream in(dbGetDataDir() + "/waiting_list.json");
    if (!in.is_open()) return false;
    
    stringstream buffer;
    buffer << in.rdbuf();
    vector<string> objects = splitJsonArray(buffer.str());
    for (const string& obj : objects) {
        entries.push_back(jsonToWaiting(obj));
    }
    in.close();
    
    sort(entries.begin(), entries.end(), [](const WaitingEntry& a, const WaitingEntry& b) {
        return a.waitId < b.waitId;
    });
    
    for (const WaitingEntry& w : entries) {
        waitingLists[w.trainNo].push(w);
    }
    return true;
}
