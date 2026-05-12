// coach.h - declares one coach definition loaded from the CSV files.

#ifndef COACH_H  // Prevent duplicate inclusion of this header.
#define COACH_H  // Mark this header as already included.

#include <string>  // Provides std::string for coach identifiers and types.

// Stores one coach layout belonging to one train.
struct Coach {
    int trainId{};               // Links this coach back to its owning train id.
    std::string coachId;         // Stores the coach label such as S1 or A1.
    std::string coachType;       // Stores the coach category such as SL, 3A, or 2A.
    int seatCount{};             // Stores how many seats should be generated for the coach.
};

#endif  // COACH_H
