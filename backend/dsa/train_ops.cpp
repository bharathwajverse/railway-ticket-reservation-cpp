#include "../include/structures.h"
#include "../include/config.h"
#include "train_ops.h"
#include "validation.h"
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

// Time Complexity: O(log N) where N is number of trains
// Purpose: Binary search for a train by trainNo
int binarySearchTrain(const vector<Train>& trains, int trainNo) {
    int low = 0;
    int high = trains.size() - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (trains[mid].trainNo == trainNo) {
            return mid;
        } else if (trains[mid].trainNo < trainNo) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return -1;
}

// Time Complexity: O(N * M) where N is trains, M is string lengths
// Purpose: Linear search for trains by destination
vector<int> linearSearchByDestination(const vector<Train>& trains, const string& destination) {
    vector<int> matches;
    for (size_t i = 0; i < trains.size(); i++) {
        if (containsIgnoreCase(trains[i].destination, destination)) {
            matches.push_back(i);
        }
    }
    return matches;
}

// Time Complexity: O(N) where N is number of trains
// Purpose: Insert a train into sorted position
int insertTrainSorted(vector<Train>& trains, const Train& newTrain) {
    if (binarySearchTrain(trains, newTrain.trainNo) != -1) {
        return -1; // Duplicate
    }
    size_t i = 0;
    for (; i < trains.size(); i++) {
        if (trains[i].trainNo > newTrain.trainNo) {
            break;
        }
    }
    trains.insert(trains.begin() + i, newTrain);
    return i;
}

// Time Complexity: O(N^2) where N is number of trains
// Purpose: Sort trains by fare
vector<Train> sortTrainsByFare(vector<Train> trainsCopy) {
    for (size_t i = 0; i < trainsCopy.size(); i++) {
        for (size_t j = 0; j < trainsCopy.size() - 1 - i; j++) {
            if (trainsCopy[j].fare > trainsCopy[j+1].fare) {
                Train temp = trainsCopy[j];
                trainsCopy[j] = trainsCopy[j+1];
                trainsCopy[j+1] = temp;
            }
        }
    }
    return trainsCopy;
}

// Time Complexity: O(N^2)
// Purpose: Sort trains by name
vector<Train> sortTrainsByName(vector<Train> trainsCopy) {
    for (size_t i = 0; i < trainsCopy.size(); i++) {
        for (size_t j = 0; j < trainsCopy.size() - 1 - i; j++) {
            if (trainsCopy[j].name > trainsCopy[j+1].name) {
                Train temp = trainsCopy[j];
                trainsCopy[j] = trainsCopy[j+1];
                trainsCopy[j+1] = temp;
            }
        }
    }
    return trainsCopy;
}

// Time Complexity: O(1)
// Purpose: Format train details to string
string formatTrainInfo(const Train& train) {
    stringstream ss;
    ss << "TrainNo: " << train.trainNo 
       << ", Name: " << train.name
       << ", " << train.source << " -> " << train.destination
       << ", Dep: " << train.departure
       << ", Seats: " << train.availableSeats
       << ", Fare: " << train.fare;
    return ss.str();
}

// Time Complexity: O(N)
// Purpose: Format list of trains as table
string formatTrainTable(const vector<Train>& trains) {
    stringstream ss;
    ss << left << setw(10) << "TrainNo" 
       << setw(20) << "Name"
       << setw(15) << "Source"
       << setw(15) << "Destination"
       << setw(10) << "Seats"
       << setw(10) << "Fare" << "\n";
    for (size_t i = 0; i < trains.size(); i++) {
        ss << left << setw(10) << trains[i].trainNo 
           << setw(20) << trains[i].name
           << setw(15) << trains[i].source
           << setw(15) << trains[i].destination
           << setw(10) << trains[i].availableSeats
           << setw(10) << trains[i].fare << "\n";
    }
    return ss.str();
}
