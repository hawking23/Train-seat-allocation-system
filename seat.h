// seat.h - declares one runtime seat inside a train coach.

#ifndef SEAT_H  // Prevent duplicate inclusion of this header.
#define SEAT_H  // Mark this header as included.

#include <string>  // Provides std::string for berth labels and passenger names.

// Stores one seat generated for a coach at runtime.
struct Seat {
    int seatNo{};                // Stores the visible seat number.
    std::string berthType;       // Stores the berth label such as LB, MB, UB, SL, or SU.
    bool booked{};               // Tracks whether the seat has already been booked.
    std::string passengerName;   // Stores the name of the passenger who booked the seat.
    int bookingPnr{};            // Stores the PNR currently assigned to the seat.
};

#endif  // SEAT_H
