// data_loader.cpp - loads CSV data into simple in-memory vectors.

#include "data_loader.h"  // Pull in loader declarations and shared data structures.

#include <fstream>        // Provides std::ifstream for file reading.
#include <sstream>        // Provides std::stringstream for CSV splitting and int parsing.

using namespace std;      // Keeps the sample code compact and easy to follow.

namespace {  // Hide the stored data and helper functions inside this source file.

vector<Station> g_stations;  // Stores the shared station line used for labels and distances.
vector<Train> g_trains;      // Stores every loaded train definition.
vector<Coach> g_coaches;     // Stores every loaded coach definition.

// Splits one comma-separated CSV line into plain text cells.
vector<string> splitCsvLine(const string& line) {
    vector<string> cells;      // Store the parsed cells from the current line.
    string cell;               // Hold one cell at a time while parsing.
    stringstream row(line);    // Wrap the text line in a stream for comma-based extraction.
    while (getline(row, cell, ',')) {  // Extract cells until the line is exhausted.
        cells.push_back(cell); // Append the current cell to the result vector.
    }
    return cells;              // Return the parsed cell list.
}

// Converts a text cell to an integer using a simple stream parse.
int toInt(const string& text) {
    int value = 0;             // Default the parsed value to zero.
    stringstream cell(text);   // Wrap the text in a stream for numeric extraction.
    cell >> value;             // Parse the integer value from the stream.
    return value;              // Return the parsed integer.
}

// Loads the station master line from its CSV file.
void loadStations(const string& path) {
    g_stations.clear();        // Remove any station data from earlier loads.
    ifstream in(path);         // Open the requested station CSV file.
    string line;               // Hold one CSV row at a time.
    while (getline(in, line)) {  // Read the file row by row.
        if (line.empty()) {    // Skip blank lines to avoid malformed records.
            continue;          // Move on to the next row.
        }
        vector<string> cells = splitCsvLine(line);  // Split the row into CSV cells.
        Station station;       // Create one station object for this row.
        station.name = cells[0];      // Read the station name.
        station.code = cells[1];      // Read the station code.
        station.kmToNext = toInt(cells[2]);  // Read the distance to the next station.
        g_stations.push_back(station);  // Append the station to the shared station list.
    }
}

// Loads train identities and timings from their CSV file.
void loadTrains(const string& path) {
    g_trains.clear();          // Remove any train data from earlier loads.
    ifstream in(path);         // Open the requested train CSV file.
    string line;               // Hold one CSV row at a time.
    while (getline(in, line)) {  // Read the file row by row.
        if (line.empty()) {    // Skip blank lines to avoid malformed records.
            continue;          // Move on to the next row.
        }
        vector<string> cells = splitCsvLine(line);  // Split the row into CSV cells.
        Train train;           // Create one train object for this row.
        train.id = toInt(cells[0]);      // Read the internal train id.
        train.trainNo = cells[1];        // Read the public train number.
        train.trainName = cells[2];      // Read the train name.
        train.fromCode = cells[3];       // Read the starting station code.
        train.toCode = cells[4];         // Read the ending station code.
        train.departureTime = cells[5];  // Read the departure time.
        train.arrivalTime = cells[6];    // Read the arrival time.
        g_trains.push_back(train);       // Append the train to the train list.
    }
}

// Loads route stations and appends them to the matching train.
void loadTrainRoutes(const string& path) {
    ifstream in(path);         // Open the requested route CSV file.
    string line;               // Hold one CSV row at a time.
    while (getline(in, line)) {  // Read the file row by row.
        if (line.empty()) {    // Skip blank lines to avoid malformed records.
            continue;          // Move on to the next row.
        }
        vector<string> cells = splitCsvLine(line);  // Split the row into CSV cells.
        int trainId = toInt(cells[0]);  // Read the train id that owns this route stop.
        string stationCode = cells[1];  // Read the station code for this route stop.
        for (Train& train : g_trains) {  // Search the loaded train list for the matching train.
            if (train.id == trainId) {   // Keep only the train that owns this route stop.
                train.routeCodes.push_back(stationCode);  // Append the station code in route order.
            }
        }
    }
}

// Loads coach definitions for all trains from their CSV file.
void loadCoaches(const string& path) {
    g_coaches.clear();         // Remove any coach data from earlier loads.
    ifstream in(path);         // Open the requested coach CSV file.
    string line;               // Hold one CSV row at a time.
    while (getline(in, line)) {  // Read the file row by row.
        if (line.empty()) {    // Skip blank lines to avoid malformed records.
            continue;          // Move on to the next row.
        }
        vector<string> cells = splitCsvLine(line);  // Split the row into CSV cells.
        Coach coach;           // Create one coach object for this row.
        coach.trainId = toInt(cells[0]);  // Read the owning train id.
        coach.coachId = cells[1];         // Read the coach label.
        coach.coachType = cells[2];       // Read the coach type.
        coach.seatCount = toInt(cells[3]);  // Read the seat count for the coach.
        g_coaches.push_back(coach);       // Append the coach to the coach list.
    }
}

}  // namespace

// Loads every CSV file required by the train booking system.
void loadDataFiles() {
    loadStations("data/stations.csv");        // Load the station master line first.
    loadTrains("data/trains.csv");            // Load train identities before route attachment.
    loadTrainRoutes("data/train_routes.csv"); // Append route stops to the already loaded trains.
    loadCoaches("data/coaches.csv");          // Load coach layouts after train definitions exist.
}

// Returns the shared station list.
const vector<Station>& getStations() {
    return g_stations;                        // Expose the stored station vector by const reference.
}

// Returns the shared train list.
const vector<Train>& getTrains() {
    return g_trains;                          // Expose the stored train vector by const reference.
}

// Returns the shared coach list.
const vector<Coach>& getCoaches() {
    return g_coaches;                         // Expose the stored coach vector by const reference.
}

// Finds one train by its internal id.
const Train* findTrainById(int trainId) {
    for (const Train& train : g_trains) {     // Walk through every loaded train.
        if (train.id == trainId) {            // Check for an id match.
            return &train;                    // Return the address of the matching train.
        }
    }
    return nullptr;                           // Return null if no train matches.
}

// Finds one coach by its owning train id and coach id.
const Coach* findCoachById(int trainId, const string& coachId) {
    for (const Coach& coach : g_coaches) {    // Walk through every loaded coach.
        if (coach.trainId == trainId && coach.coachId == coachId) {  // Match both train id and coach id.
            return &coach;                    // Return the address of the matching coach.
        }
    }
    return nullptr;                           // Return null if no coach matches.
}

// Collects every coach that belongs to one train.
vector<Coach> getCoachesForTrain(int trainId) {
    vector<Coach> matches;                    // Store matching coaches for the selected train.
    for (const Coach& coach : g_coaches) {    // Walk through every loaded coach.
        if (coach.trainId == trainId) {       // Keep only coaches that belong to the selected train.
            matches.push_back(coach);         // Append the matching coach to the result list.
        }
    }
    return matches;                           // Return the collected coach list.
}
