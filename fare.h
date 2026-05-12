// fare.h - declares fare calculation helpers for tickets.

#ifndef FARE_H  // Prevent duplicate inclusion of this header.
#define FARE_H  // Mark this header as included.

#include <string>  // Provides std::string for coach type parameters and return values.

// Stores the detailed price breakup for one booking.
struct FareBreakdown {
    int distanceKm{};            // Stores the route distance used to derive the fare.
    int subtotal{};              // Stores the rounded fare before GST is added.
    int gstRatePercent{};        // Stores the GST percentage applied for the selected coach type.
    int gst{};                   // Stores the rounded GST amount.
    int total{};                 // Stores the final amount payable by the passenger.
};

FareBreakdown computeFare(int routeDistanceKm, const std::string& coachType, int age);  // Computes fare for one passenger.
std::string concessionLabel(int age);                                                    // Returns a human-readable concession category.

#endif  // FARE_H
