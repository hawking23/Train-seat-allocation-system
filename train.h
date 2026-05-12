// train.h - declares one train with identity, timings, and covered route.

#ifndef TRAIN_H  // Prevent duplicate inclusion of this header.
#define TRAIN_H  // Mark this header as included.

#include <string>  // Provides std::string for train text fields.
#include <vector>  // Provides std::vector for ordered route station codes.

// Stores one train loaded from the CSV files.
struct Train {
    int id{};                           // Stores the internal numeric train id.
    std::string trainNo;                // Stores the public train number.
    std::string trainName;              // Stores the train name shown to the user.
    std::string fromCode;               // Stores the first station code for the train.
    std::string toCode;                 // Stores the last station code for the train.
    std::string departureTime;          // Stores the departure time string.
    std::string arrivalTime;            // Stores the arrival time string.
    std::vector<std::string> routeCodes;  // Stores the ordered station codes covered by the train.
};

#endif  // TRAIN_H
