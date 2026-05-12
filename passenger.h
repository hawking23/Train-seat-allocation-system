// passenger.h - declares the passenger data collected during booking.

#ifndef PASSENGER_H  // Prevent duplicate inclusion of this header.
#define PASSENGER_H  // Mark this header as included.

#include <string>  // Provides std::string for passenger text fields.

// Stores one passenger entered through the booking flow.
struct Passenger {
    std::string name;            // Stores the passenger name for the ticket.
    int age{};                   // Stores the passenger age for concession checks.
    char gender{};               // Stores the passenger gender character.
    std::string journeyDate;     // Stores the selected journey date.
    std::string boarding;        // Stores the entered boarding station.
    std::string destination;     // Stores the entered destination station.
};

#endif  // PASSENGER_H
