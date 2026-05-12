// station.h - declares one station on the shared master route line.

#ifndef STATION_H  // Prevent duplicate inclusion of this header.
#define STATION_H  // Mark this header as included.

#include <string>  // Provides std::string for station names and codes.

// Stores one station and the distance to the next station in the master line.
struct Station {
    std::string name;            // Stores the full station name shown to the user.
    std::string code;            // Stores the station code used in CSV routes.
    int kmToNext{};              // Stores the distance from this station to the next one.
};

#endif  // STATION_H
