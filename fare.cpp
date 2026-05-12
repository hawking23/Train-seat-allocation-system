// fare.cpp - implements basic fare rules based on distance, coach type, age, and GST.

#include "fare.h"  // Pull in fare declarations and the FareBreakdown structure.

#include <cmath>   // Provides std::lround for rounded monetary values.

using namespace std;  // Keeps the code compact for this small project.

namespace {  // Hide internal fare constants and helpers inside this file.

// Returns the base fare rate per kilometer for the given coach type.
double ratePerKm(const string& coachType) {
    if (coachType == "2A") {    // Use the highest rate for 2A.
        return 3.0;             // Charge Rs 3.0 per kilometer for 2A.
    }
    if (coachType == "3A") {    // Use the middle rate for 3A.
        return 2.0;             // Charge Rs 2.0 per kilometer for 3A.
    }
    return 1.2;                 // Use the default sleeper-style rate for other coach types.
}

// Returns the concession multiplier based on passenger age.
double concessionFactor(int age) {
    if (age >= 60 || age < 12) {  // Give the same discount to seniors and children.
        return 0.5;               // Charge only half of the base fare.
    }
    return 1.0;                   // Charge the full base fare for all other passengers.
}

// Returns the GST percentage for the selected coach type.
int gstRatePercent(const string& coachType) {
    if (coachType == "2A") {       // Keep the highest GST on the premium AC coach.
        return 8;
    }
    if (coachType == "3A") {       // Apply a mid-tier GST for 3A coaches.
        return 5;
    }
    return 2;                      // Keep a lower GST for sleeper and other non-AC coaches.
}

}  // namespace

// Computes the complete fare breakdown for one booking.
FareBreakdown computeFare(int routeDistanceKm, const string& coachType, int age) {
    FareBreakdown fare;           // Create an output structure to fill.
    fare.distanceKm = routeDistanceKm;  // Store the route distance in the result.
    const double base = static_cast<double>(routeDistanceKm) * ratePerKm(coachType) * concessionFactor(age);  // Compute the unrounded pre-tax base fare.
    fare.subtotal = static_cast<int>(lround(base));  // Round the base fare to the nearest integer.
    fare.gstRatePercent = gstRatePercent(coachType);  // Capture the coach-wise GST percentage for display and totals.
    const double gstRate = static_cast<double>(fare.gstRatePercent) / 100.0;
    fare.gst = static_cast<int>(lround(static_cast<double>(fare.subtotal) * gstRate));  // Round the GST amount from the subtotal.
    fare.total = fare.subtotal + fare.gst;  // Add subtotal and GST to get the final total.
    return fare;                 // Return the completed fare breakdown.
}

// Returns the display label for the age-based concession category.
string concessionLabel(int age) {
    if (age >= 60) {             // Seniors are passengers aged 60 or above.
        return "Senior";         // Return the senior concession label.
    }
    if (age < 12) {              // Children are passengers younger than 12.
        return "Child";          // Return the child concession label.
    }
    return "None";               // Return no concession for everyone else.
}
