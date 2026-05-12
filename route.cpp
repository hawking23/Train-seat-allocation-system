// route.cpp - normalizes station input, checks train coverage, and computes route distances.

#include "route.h"  // Pull in route declarations and the Train structure.

#include <cctype>   // Provides std::tolower for case-insensitive comparisons.

#include "data_loader.h"  // Provides access to the shared station and train data.

using namespace std;      // Keeps the helper code short and readable.

namespace {  // Hide internal string and station lookup helpers inside this file.

// Removes spaces, tabs, and carriage returns from the start and end of a string.
string trim(const string& text) {
    size_t left = 0;       // Start scanning from the left edge of the string.
    while (left < text.size() && (text[left] == ' ' || text[left] == '\t' || text[left] == '\r')) {  // Skip leading whitespace characters.
        ++left;            // Move the left boundary forward.
    }
    size_t right = text.size();  // Start scanning from the right edge of the string.
    while (right > left && (text[right - 1] == ' ' || text[right - 1] == '\t' || text[right - 1] == '\r')) {  // Skip trailing whitespace characters.
        --right;           // Move the right boundary backward.
    }
    return text.substr(left, right - left);  // Return the trimmed substring.
}

// Compares two strings without caring about letter case.
bool iequals(const string& left, const string& right) {
    if (left.size() != right.size()) {  // Different lengths can never be equal.
        return false;                   // Report a mismatch immediately.
    }
    for (size_t i = 0; i < left.size(); ++i) {  // Compare each character position.
        unsigned char a = static_cast<unsigned char>(left[i]);  // Safely widen the left character for tolower.
        unsigned char b = static_cast<unsigned char>(right[i]); // Safely widen the right character for tolower.
        if (tolower(a) != tolower(b)) {  // Compare the lowercase form of both characters.
            return false;                // Stop at the first mismatch.
        }
    }
    return true;                         // Report equality when all characters match.
}

// Finds a station in the master station list using either code or full name.
const Station* findStationByInput(const string& input) {
    const string query = trim(input);    // Normalize the typed station text.
    for (const Station& station : getStations()) {  // Check every loaded station.
        if (iequals(query, station.code) || iequals(query, station.name)) {  // Match either the code or the full name.
            return &station;             // Return the matching station.
        }
    }
    return nullptr;                      // Return null if no station matches.
}

// Finds the index of a station inside the master station line.
int masterRouteIndex(const string& input) {
    const string query = trim(input);    // Normalize the typed station text.
    const vector<Station>& stations = getStations();  // Read the shared station list once.
    for (size_t i = 0; i < stations.size(); ++i) {  // Walk through the station list by index.
        if (iequals(query, stations[i].code) || iequals(query, stations[i].name)) {  // Match either the code or the full name.
            return static_cast<int>(i);  // Return the matching station index.
        }
    }
    return -1;                           // Return -1 when the station is not found.
}

}  // namespace

// Converts typed station input into its canonical station code when known.
string canonicalStationCode(const string& input) {
    const Station* station = findStationByInput(input);  // Look up the typed station.
    if (station != nullptr) {                            // Use the stored station data when a match exists.
        return station->code;                            // Return the canonical station code.
    }
    return trim(input);                                  // Otherwise return a trimmed version of the original input.
}

// Converts typed station input into its canonical display label when known.
string canonicalStationLabel(const string& input) {
    const Station* station = findStationByInput(input);  // Look up the typed station.
    if (station != nullptr) {                            // Use the stored station data when a match exists.
        return station->name;                            // Return the canonical station name.
    }
    return trim(input);                                  // Otherwise return a trimmed version of the original input.
}

// Finds the position of a station inside a train's ordered route.
int routePositionOnTrain(const Train& train, const string& stationInput) {
    const string stationCode = canonicalStationCode(stationInput);  // Normalize the typed station to a canonical code.
    for (size_t i = 0; i < train.routeCodes.size(); ++i) {          // Walk through the train route by index.
        if (train.routeCodes[i] == stationCode) {                   // Check whether this route stop matches the requested station.
            return static_cast<int>(i);                             // Return the matching route position.
        }
    }
    return -1;                                                      // Return -1 when the train does not stop there.
}

// Checks whether a train covers a boarding station before a destination station.
bool trainCoversRoute(const Train& train, const string& boarding, const string& destination) {
    const int boardingPosition = routePositionOnTrain(train, boarding);        // Find where the boarding station appears in the route.
    const int destinationPosition = routePositionOnTrain(train, destination);  // Find where the destination station appears in the route.
    return boardingPosition >= 0 && destinationPosition >= 0 && boardingPosition < destinationPosition;  // Accept only routes where both stops exist in the correct order.
}

// Computes the distance between two stations on the master station line.
int distanceKm(const string& boarding, const string& destination) {
    const int start = masterRouteIndex(boarding);  // Find the master-line index of the boarding station.
    const int end = masterRouteIndex(destination); // Find the master-line index of the destination station.
    int total = 0;                                 // Start the distance total at zero.
    for (int i = start; i < end; ++i) {            // Walk from the boarding station up to the destination station.
        total += getStations()[static_cast<size_t>(i)].kmToNext;  // Add the segment distance to the running total.
    }
    return total;                                  // Return the final route distance.
}
