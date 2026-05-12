// booking.h - declares runtime booking state and booking-related menu flows.

#ifndef BOOKING_H  // Prevent this header from being included more than once.
#define BOOKING_H  // Mark this header as included for the current compilation unit.

#include <string>  // Provides std::string for text fields.
#include <vector>  // Provides std::vector for dynamic seat collections.

#include "seat.h"  // Reuse the Seat structure for runtime coach seat maps.

// Stores the live seat state for one coach of one train after CSV data is loaded.
struct TrainCoachState {
    int trainId{};               // Identifies which train owns this runtime coach state.
    std::string coachId;         // Stores the coach label such as S1 or A1.
    std::string coachType;       // Stores the coach category such as SL, 3A, or 2A.
    std::vector<Seat> seats;     // Stores every generated seat object for this coach.
};

// Stores the final details of one confirmed booking so they can be shown later.
struct BookingRecord {
    int pnr{};                   // Stores the generated passenger name record number.
    int trainId{};               // Stores the internal train id for seat and queue updates.
    std::string journeyDate;     // Stores the journey date entered by the user.
    std::string trainNo;         // Stores the selected public train number.
    std::string trainName;       // Stores the selected train name.
    std::string boarding;        // Stores the normalized boarding station label.
    std::string destination;     // Stores the normalized destination station label.
    std::string departureTime;   // Stores the train departure time shown on the ticket.
    std::string arrivalTime;     // Stores the train arrival time shown on the ticket.
    std::string coachId;         // Stores the selected coach id.
    std::string coachType;       // Stores the selected coach type.
    int seatNo{};                // Stores the selected seat number.
    std::string berthType;       // Stores the berth label such as LB or SU.
    std::string passengerName;   // Stores the passenger name printed on the ticket.
    int age{};                   // Stores the passenger age used for concessions.
    char gender{};               // Stores the passenger gender character.
    int distanceKm{};            // Stores the route distance used for fare calculation.
    int totalFare{};             // Stores the final payable amount.
    std::string status;          // Stores the live booking status such as CONFIRMED, RAC, WL, or CANCELLED.
    int refundAmount{};          // Stores the refund amount after a cancellation.
};

void initBookingSystem();        // Builds live seat maps and resets booking memory.
void searchTrainsFlow();         // Runs the search-only menu flow.
void bookTicketFlow();           // Runs the full booking flow from search to ticket print.
void cancelTicketFlow();         // Cancels one booking by PNR and applies queue promotion.
void swapSeatsFlow();            // Swaps seats between two eligible confirmed passengers.
void viewAllTrains();            // Prints every loaded train.
void viewTrainCoachesFlow();     // Lets the user inspect coaches for one selected train.
void viewCoachSeatsFlow();       // Lets the user inspect seats for one selected coach.
void viewBookingsSummary();      // Prints all bookings created in the current run.

#endif  // BOOKING_H
