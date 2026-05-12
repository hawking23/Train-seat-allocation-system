#include "booking.h"

#include <algorithm>
#include <cctype>
#include <deque>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <utility>

#include "data_loader.h"
#include "fare.h"
#include "passenger.h"
#include "route.h"

using namespace std;

namespace {

const int RAC_LIMIT_PER_COACH = 2;
const int WAITING_LIMIT_PER_COACH = 2;
const int REFUND_DEDUCTION_PERCENT = 10;

struct QueueState {
    int trainId{};
    string coachId;
    deque<int> racPnrs;
    deque<int> waitingPnrs;
};

vector<TrainCoachState> g_trainCoachStates;
vector<QueueState> g_queueStates;
map<int, BookingRecord> g_bookingsByPnr;
int g_nextPnr = 100001;

string readLine(const string& prompt) {
    cout << prompt;
    string line;
    getline(cin, line);
    return line;
}

int readIntLine(const string& prompt) {
    cout << prompt;
    string line;
    getline(cin, line);
    int value = 0;
    stringstream text(line);
    text >> value;
    return value;
}

char normalizeGender(char gender) {
    return static_cast<char>(toupper(static_cast<unsigned char>(gender)));
}

string toLowerText(const string& text) {
    string lowered = text;
    for (char& ch : lowered) {
        ch = static_cast<char>(tolower(static_cast<unsigned char>(ch)));
    }
    return lowered;
}

string compactLowerName(const string& name) {
    string compact;
    for (char ch : name) {
        if (!isspace(static_cast<unsigned char>(ch))) {
            compact.push_back(static_cast<char>(tolower(static_cast<unsigned char>(ch))));
        }
    }
    return compact;
}

string sleeperPatternLabel(int zeroBasedSeatIndex) {
    static const string pattern[] = {"LB", "MB", "UB", "LB", "MB", "UB", "SL", "SU"};
    return pattern[zeroBasedSeatIndex % 8];
}

string twoTierPatternLabel(int zeroBasedSeatIndex) {
    static const string pattern[] = {"LB", "UB", "LB", "UB", "SL", "SU"};
    return pattern[zeroBasedSeatIndex % 6];
}

string berthTypeForSeat(const string& coachType, int zeroBasedSeatIndex) {
    if (coachType == "2A") {
        return twoTierPatternLabel(zeroBasedSeatIndex);
    }
    return sleeperPatternLabel(zeroBasedSeatIndex);
}

TrainCoachState* findCoachState(int trainId, const string& coachId) {
    for (TrainCoachState& state : g_trainCoachStates) {
        if (state.trainId == trainId && state.coachId == coachId) {
            return &state;
        }
    }
    return nullptr;
}

const TrainCoachState* findCoachState(int trainId, const string& coachId, bool) {
    for (const TrainCoachState& state : g_trainCoachStates) {
        if (state.trainId == trainId && state.coachId == coachId) {
            return &state;
        }
    }
    return nullptr;
}

QueueState* findQueueState(int trainId, const string& coachId) {
    for (QueueState& state : g_queueStates) {
        if (state.trainId == trainId && state.coachId == coachId) {
            return &state;
        }
    }
    return nullptr;
}

BookingRecord* findBookingByPnr(int pnr) {
    map<int, BookingRecord>::iterator it = g_bookingsByPnr.find(pnr);
    if (it == g_bookingsByPnr.end()) {
        return nullptr;
    }
    return &it->second;
}

const BookingRecord* findBookingByPnr(int pnr, bool) {
    map<int, BookingRecord>::const_iterator it = g_bookingsByPnr.find(pnr);
    if (it == g_bookingsByPnr.end()) {
        return nullptr;
    }
    return &it->second;
}

Seat* findSeatByPnr(TrainCoachState& state, int pnr) {
    for (Seat& seat : state.seats) {
        if (seat.bookingPnr == pnr) {
            return &seat;
        }
    }
    return nullptr;
}

int countAvailableSeatsInCoachState(const TrainCoachState& state) {
    int count = 0;
    for (const Seat& seat : state.seats) {
        if (!seat.booked) {
            ++count;
        }
    }
    return count;
}

int countAvailableCoachesForTrain(int trainId) {
    int count = 0;
    for (const TrainCoachState& state : g_trainCoachStates) {
        if (state.trainId == trainId && countAvailableSeatsInCoachState(state) > 0) {
            ++count;
        }
    }
    return count;
}

int countAvailableSeatsForTrain(int trainId) {
    int count = 0;
    for (const TrainCoachState& state : g_trainCoachStates) {
        if (state.trainId == trainId) {
            count += countAvailableSeatsInCoachState(state);
        }
    }
    return count;
}

int countRacForTrain(int trainId) {
    int count = 0;
    for (const QueueState& state : g_queueStates) {
        if (state.trainId == trainId) {
            count += static_cast<int>(state.racPnrs.size());
        }
    }
    return count;
}

int countWaitingForTrain(int trainId) {
    int count = 0;
    for (const QueueState& state : g_queueStates) {
        if (state.trainId == trainId) {
            count += static_cast<int>(state.waitingPnrs.size());
        }
    }
    return count;
}

char bookedPassengerGender(int pnr) {
    const BookingRecord* booking = findBookingByPnr(pnr, true);
    if (booking == nullptr) {
        return 'U';
    }
    return normalizeGender(booking->gender);
}

vector<const Train*> findMatchingTrains(const string& boarding, const string& destination) {
    vector<const Train*> matches;
    for (const Train& train : getTrains()) {
        if (trainCoversRoute(train, boarding, destination)) {
            matches.push_back(&train);
        }
    }
    return matches;
}

void printTrainRow(int serialNo, const Train& train, const string& boarding, const string& destination) {
    cout << serialNo << ". ";
    cout << train.trainNo << " - " << train.trainName;
    cout << " | " << canonicalStationLabel(boarding) << " -> " << canonicalStationLabel(destination);
    cout << " | Dep " << train.departureTime;
    cout << " | Arr " << train.arrivalTime;
    cout << " | Coaches " << countAvailableCoachesForTrain(train.id);
    cout << " | Seats " << countAvailableSeatsForTrain(train.id);
    cout << " | RAC " << countRacForTrain(train.id);
    cout << " | WL " << countWaitingForTrain(train.id) << '\n';
}

void printMatchingTrains(const vector<const Train*>& matches, const string& boarding, const string& destination,
                         const string& journeyDate) {
    cout << "\n--- Matching Trains ---\n";
    cout << "Date: " << journeyDate << '\n';
    for (size_t i = 0; i < matches.size(); ++i) {
        printTrainRow(static_cast<int>(i + 1), *matches[i], boarding, destination);
    }
    cout << '\n';
}

void printTrainList() {
    cout << "\n--- All Trains ---\n";
    const vector<Train>& trains = getTrains();
    for (size_t i = 0; i < trains.size(); ++i) {
        cout << (i + 1) << ". ";
        cout << trains[i].trainNo << " - " << trains[i].trainName;
        cout << " | " << canonicalStationLabel(trains[i].fromCode) << " -> " << canonicalStationLabel(trains[i].toCode);
        cout << " | Dep " << trains[i].departureTime;
        cout << " | Arr " << trains[i].arrivalTime;
        cout << " | Seats " << countAvailableSeatsForTrain(trains[i].id);
        cout << " | RAC " << countRacForTrain(trains[i].id);
        cout << " | WL " << countWaitingForTrain(trains[i].id) << '\n';
    }
    cout << '\n';
}

void printCoachList(int trainId) {
    const Train* train = findTrainById(trainId);
    if (train == nullptr) {
        cout << "\nTrain not found.\n\n";
        return;
    }

    cout << "\n--- Coaches For " << train->trainNo << " - " << train->trainName << " ---\n";
    const vector<Coach> coaches = getCoachesForTrain(trainId);
    for (size_t i = 0; i < coaches.size(); ++i) {
        const TrainCoachState* coachState = findCoachState(trainId, coaches[i].coachId, true);
        QueueState* queueState = findQueueState(trainId, coaches[i].coachId);
        const int availableSeats = coachState == nullptr ? 0 : countAvailableSeatsInCoachState(*coachState);
        const size_t racCount = queueState == nullptr ? 0 : queueState->racPnrs.size();
        const size_t waitingCount = queueState == nullptr ? 0 : queueState->waitingPnrs.size();

        cout << (i + 1) << ". ";
        cout << coaches[i].coachId << " | " << coaches[i].coachType;
        cout << " | Seats " << coaches[i].seatCount;
        cout << " | Available " << availableSeats;
        cout << " | RAC " << racCount << "/" << RAC_LIMIT_PER_COACH;
        cout << " | WL " << waitingCount << "/" << WAITING_LIMIT_PER_COACH << '\n';
    }
    cout << '\n';
}

void printSeatMap(int trainId, const string& coachId) {
    const TrainCoachState* state = findCoachState(trainId, coachId, true);
    if (state == nullptr) {
        cout << "\nSeat map not found for coach " << coachId << ".\n\n";
        return;
    }

    cout << "\n--- Seat Map For Coach " << coachId << " ---\n";
    for (const Seat& seat : state->seats) {
        cout << setw(2) << seat.seatNo << " ";
        cout << setw(2) << seat.berthType << " ";
        if (seat.booked) {
            cout << "Booked - " << seat.passengerName << " (" << bookedPassengerGender(seat.bookingPnr) << ")";
        } else {
            cout << "Available";
        }
        cout << '\n';
    }
    cout << '\n';
}

void readSearchInputs(string& boarding, string& destination, string& journeyDate) {
    boarding = readLine("From station (name or code): ");
    destination = readLine("To station (name or code): ");
    journeyDate = readLine("Journey date (YYYY-MM-DD): ");
}

Passenger readPassengerInputs(const string& boarding, const string& destination, const string& journeyDate) {
    Passenger passenger;
    passenger.boarding = boarding;
    passenger.destination = destination;
    passenger.journeyDate = journeyDate;
    passenger.name = readLine("Passenger name: ");
    passenger.age = readIntLine("Age: ");
    const string genderText = readLine("Gender (M/F/O): ");
    passenger.gender = genderText.empty() ? 'M' : normalizeGender(genderText[0]);
    return passenger;
}

bool isBesideBookedFemalePassenger(const TrainCoachState& coachState, size_t seatIndex) {
    const int offsets[] = {-1, 1};
    for (int offset : offsets) {
        const int neighborIndex = static_cast<int>(seatIndex) + offset;
        if (neighborIndex < 0 || neighborIndex >= static_cast<int>(coachState.seats.size())) {
            continue;
        }

        const Seat& neighborSeat = coachState.seats[static_cast<size_t>(neighborIndex)];
        if (neighborSeat.booked && bookedPassengerGender(neighborSeat.bookingPnr) == 'F') {
            return true;
        }
    }
    return false;
}

Seat* findRecommendedSeat(TrainCoachState& coachState, char passengerGender) {
    Seat* firstAvailableSeat = nullptr;
    const bool prioritizeFemaleAdjacency = normalizeGender(passengerGender) == 'F';

    for (size_t i = 0; i < coachState.seats.size(); ++i) {
        Seat& seat = coachState.seats[i];
        if (seat.booked) {
            continue;
        }
        if (firstAvailableSeat == nullptr) {
            firstAvailableSeat = &seat;
        }
        if (prioritizeFemaleAdjacency && isBesideBookedFemalePassenger(coachState, i)) {
            return &seat;
        }
    }
    return firstAvailableSeat;
}

Seat* selectSeatForPassenger(TrainCoachState& coachState, const Passenger& passenger) {
    printSeatMap(coachState.trainId, coachState.coachId);
    Seat* recommendedSeat = findRecommendedSeat(coachState, passenger.gender);
    if (recommendedSeat == nullptr) {
        return nullptr;
    }

    const bool femalePriorityApplied =
        normalizeGender(passenger.gender) == 'F' &&
        isBesideBookedFemalePassenger(coachState, static_cast<size_t>(recommendedSeat->seatNo - 1));
    if (femalePriorityApplied) {
        cout << "Recommended seat for female passenger: " << recommendedSeat->seatNo
             << " (beside a previously booked female passenger)\n";
    } else {
        cout << "Recommended seat: " << recommendedSeat->seatNo << '\n';
    }

    const string seatChoiceText = readLine("Select seat number (press Enter for recommended seat): ");
    if (seatChoiceText.empty()) {
        return recommendedSeat;
    }

    int seatChoice = 0;
    stringstream text(seatChoiceText);
    text >> seatChoice;
    if (seatChoice < 1 || seatChoice > static_cast<int>(coachState.seats.size())) {
        cout << "\nInvalid seat selection.\n\n";
        return nullptr;
    }

    Seat* chosenSeat = &coachState.seats[static_cast<size_t>(seatChoice - 1)];
    if (chosenSeat->booked) {
        cout << "\nThat seat is already booked. Please choose an available seat.\n\n";
        return nullptr;
    }
    return chosenSeat;
}

BookingRecord makeBookingRecord(const Train& train, const Coach& coach, const Passenger& passenger,
                                int distanceKmValue, int totalFareValue) {
    BookingRecord booking;
    booking.pnr = g_nextPnr;
    booking.trainId = train.id;
    booking.journeyDate = passenger.journeyDate;
    booking.trainNo = train.trainNo;
    booking.trainName = train.trainName;
    booking.boarding = canonicalStationLabel(passenger.boarding);
    booking.destination = canonicalStationLabel(passenger.destination);
    booking.departureTime = train.departureTime;
    booking.arrivalTime = train.arrivalTime;
    booking.coachId = coach.coachId;
    booking.coachType = coach.coachType;
    booking.passengerName = passenger.name;
    booking.age = passenger.age;
    booking.gender = passenger.gender;
    booking.distanceKm = distanceKmValue;
    booking.totalFare = totalFareValue;
    return booking;
}

void printTicket(const BookingRecord& booking, const FareBreakdown& fare) {
    cout << "\n==================== TICKET ====================\n";
    cout << "PNR: " << booking.pnr << '\n';
    cout << "Status: " << booking.status << '\n';
    cout << "Train: " << booking.trainNo << " - " << booking.trainName << '\n';
    cout << "Date: " << booking.journeyDate << '\n';
    cout << "Route: " << booking.boarding << " -> " << booking.destination << '\n';
    cout << "Departure: " << booking.departureTime << "   Arrival: " << booking.arrivalTime << '\n';
    cout << "Coach: " << booking.coachId << " (" << booking.coachType << ")\n";
    if (booking.status == "CONFIRMED") {
        cout << "Seat: " << booking.seatNo << " (" << booking.berthType << ")\n";
    } else {
        cout << "Seat: Not assigned yet\n";
    }
    cout << "Passenger: " << booking.passengerName << " (" << booking.gender << ") Age: " << booking.age << '\n';
    cout << "Concession: " << concessionLabel(booking.age) << '\n';
    cout << "Distance: " << booking.distanceKm << " km\n";
    cout << "Subtotal: Rs " << fare.subtotal << '\n';
    cout << "GST (" << fare.gstRatePercent << "%): Rs " << fare.gst << '\n';
    cout << "Total: Rs " << fare.total << '\n';
    cout << "================================================\n\n";
}

int computeRefundAmount(int totalFare) {
    const int deduction = (totalFare * REFUND_DEDUCTION_PERCENT) / 100;
    return totalFare - deduction;
}

bool removePnrFromQueue(deque<int>& queue, int pnr) {
    const deque<int>::iterator it = find(queue.begin(), queue.end(), pnr);
    if (it == queue.end()) {
        return false;
    }
    queue.erase(it);
    return true;
}

void promoteWaitingToRac(QueueState& queueState) {
    if (queueState.racPnrs.size() >= static_cast<size_t>(RAC_LIMIT_PER_COACH) || queueState.waitingPnrs.empty()) {
        return;
    }

    const int promotedPnr = queueState.waitingPnrs.front();
    queueState.waitingPnrs.pop_front();
    queueState.racPnrs.push_back(promotedPnr);

    BookingRecord* booking = findBookingByPnr(promotedPnr);
    if (booking != nullptr) {
        booking->status = "RAC";
        cout << "PNR " << booking->pnr << " moved from WL to RAC.\n";
    }
}

void promoteRacToConfirmed(QueueState& queueState, Seat& freedSeat) {
    if (queueState.racPnrs.empty()) {
        return;
    }

    const int promotedPnr = queueState.racPnrs.front();
    queueState.racPnrs.pop_front();
    BookingRecord* booking = findBookingByPnr(promotedPnr);
    if (booking == nullptr) {
        promoteWaitingToRac(queueState);
        return;
    }

    freedSeat.booked = true;
    freedSeat.passengerName = booking->passengerName;
    freedSeat.bookingPnr = booking->pnr;
    booking->status = "CONFIRMED";
    booking->seatNo = freedSeat.seatNo;
    booking->berthType = freedSeat.berthType;
    cout << "PNR " << booking->pnr << " promoted from RAC to confirmed seat "
         << booking->coachId << "-" << booking->seatNo << ".\n";
    promoteWaitingToRac(queueState);
}

const Train* selectTrainFromMatches(const vector<const Train*>& matches) {
    const int trainChoice = readIntLine("Select train number from the list: ");
    if (trainChoice < 1 || trainChoice > static_cast<int>(matches.size())) {
        cout << "\nInvalid train selection.\n\n";
        return nullptr;
    }
    return matches[static_cast<size_t>(trainChoice - 1)];
}

const Coach* selectCoachFromTrain(int trainId, const vector<Coach>& coaches) {
    printCoachList(trainId);
    const int coachChoice = readIntLine("Select coach number from the list: ");
    if (coachChoice < 1 || coachChoice > static_cast<int>(coaches.size())) {
        cout << "\nInvalid coach selection.\n\n";
        return nullptr;
    }
    return &coaches[static_cast<size_t>(coachChoice - 1)];
}

string seatSwapPasswordForBooking(const BookingRecord& booking) {
    string compactName = compactLowerName(booking.passengerName);
    if (compactName.size() > 4) {
        compactName.resize(4);
    }
    return compactName + "@" + to_string(booking.pnr);
}

bool bookingsCanSwapSeats(const BookingRecord& firstBooking, const BookingRecord& secondBooking) {
    return firstBooking.status == "CONFIRMED" &&
           secondBooking.status == "CONFIRMED" &&
           firstBooking.trainId == secondBooking.trainId &&
           firstBooking.journeyDate == secondBooking.journeyDate &&
           firstBooking.boarding == secondBooking.boarding &&
           firstBooking.destination == secondBooking.destination;
}

bool swapConfirmedSeats(BookingRecord& firstBooking, BookingRecord& secondBooking) {
    TrainCoachState* firstCoachState = findCoachState(firstBooking.trainId, firstBooking.coachId);
    TrainCoachState* secondCoachState = findCoachState(secondBooking.trainId, secondBooking.coachId);
    if (firstCoachState == nullptr || secondCoachState == nullptr) {
        return false;
    }

    Seat* firstSeat = findSeatByPnr(*firstCoachState, firstBooking.pnr);
    Seat* secondSeat = findSeatByPnr(*secondCoachState, secondBooking.pnr);
    if (firstSeat == nullptr || secondSeat == nullptr) {
        return false;
    }

    const string firstPassengerName = firstSeat->passengerName;
    const int firstPassengerPnr = firstSeat->bookingPnr;
    firstSeat->passengerName = secondSeat->passengerName;
    firstSeat->bookingPnr = secondSeat->bookingPnr;
    secondSeat->passengerName = firstPassengerName;
    secondSeat->bookingPnr = firstPassengerPnr;

    swap(firstBooking.coachId, secondBooking.coachId);
    swap(firstBooking.coachType, secondBooking.coachType);
    swap(firstBooking.seatNo, secondBooking.seatNo);
    swap(firstBooking.berthType, secondBooking.berthType);
    return true;
}

}  // namespace

void initBookingSystem() {
    g_trainCoachStates.clear();
    g_queueStates.clear();
    g_bookingsByPnr.clear();
    g_nextPnr = 100001;

    for (const Coach& coach : getCoaches()) {
        TrainCoachState coachState;
        coachState.trainId = coach.trainId;
        coachState.coachId = coach.coachId;
        coachState.coachType = coach.coachType;

        for (int seatNo = 1; seatNo <= coach.seatCount; ++seatNo) {
            Seat seat;
            seat.seatNo = seatNo;
            seat.berthType = berthTypeForSeat(coach.coachType, seatNo - 1);
            seat.booked = false;
            seat.passengerName = "";
            seat.bookingPnr = 0;
            coachState.seats.push_back(seat);
        }
        g_trainCoachStates.push_back(coachState);

        QueueState queueState;
        queueState.trainId = coach.trainId;
        queueState.coachId = coach.coachId;
        g_queueStates.push_back(queueState);
    }
}

void searchTrainsFlow() {
    string boarding;
    string destination;
    string journeyDate;
    readSearchInputs(boarding, destination, journeyDate);

    const vector<const Train*> matches = findMatchingTrains(boarding, destination);
    if (matches.empty()) {
        cout << "\nNo trains found for " << canonicalStationLabel(boarding) << " -> "
             << canonicalStationLabel(destination) << " on " << journeyDate << ".\n\n";
        return;
    }
    printMatchingTrains(matches, boarding, destination, journeyDate);
}

void bookTicketFlow() {
    string boarding;
    string destination;
    string journeyDate;
    readSearchInputs(boarding, destination, journeyDate);

    const vector<const Train*> matches = findMatchingTrains(boarding, destination);
    if (matches.empty()) {
        cout << "\nNo trains found for " << canonicalStationLabel(boarding) << " -> "
             << canonicalStationLabel(destination) << " on " << journeyDate << ".\n\n";
        return;
    }

    printMatchingTrains(matches, boarding, destination, journeyDate);
    const Train* selectedTrain = selectTrainFromMatches(matches);
    if (selectedTrain == nullptr) {
        return;
    }

    const vector<Coach> coaches = getCoachesForTrain(selectedTrain->id);
    const Coach* selectedCoach = selectCoachFromTrain(selectedTrain->id, coaches);
    if (selectedCoach == nullptr) {
        return;
    }

    TrainCoachState* coachState = findCoachState(selectedTrain->id, selectedCoach->coachId);
    QueueState* queueState = findQueueState(selectedTrain->id, selectedCoach->coachId);
    if (coachState == nullptr || queueState == nullptr) {
        cout << "\nBooking state for the selected coach is unavailable.\n\n";
        return;
    }

    const bool coachHasSeat = countAvailableSeatsInCoachState(*coachState) > 0;
    const Passenger passenger = readPassengerInputs(boarding, destination, journeyDate);

    Seat* chosenSeat = nullptr;
    if (coachHasSeat) {
        chosenSeat = selectSeatForPassenger(*coachState, passenger);
        if (chosenSeat == nullptr) {
            return;
        }
    } else {
        cout << "\nCoach " << selectedCoach->coachId
             << " is full. Booking will continue through RAC or waiting list if space exists.\n\n";
    }

    const int routeDistance = distanceKm(boarding, destination);
    const FareBreakdown fare = computeFare(routeDistance, selectedCoach->coachType, passenger.age);
    BookingRecord booking = makeBookingRecord(*selectedTrain, *selectedCoach, passenger, routeDistance, fare.total);

    if (chosenSeat != nullptr) {
        chosenSeat->booked = true;
        chosenSeat->passengerName = passenger.name;
        chosenSeat->bookingPnr = booking.pnr;
        booking.status = "CONFIRMED";
        booking.seatNo = chosenSeat->seatNo;
        booking.berthType = chosenSeat->berthType;
    } else if (queueState->racPnrs.size() < static_cast<size_t>(RAC_LIMIT_PER_COACH)) {
        queueState->racPnrs.push_back(booking.pnr);
        booking.status = "RAC";
        booking.seatNo = 0;
        booking.berthType = "Not assigned";
    } else if (queueState->waitingPnrs.size() < static_cast<size_t>(WAITING_LIMIT_PER_COACH)) {
        queueState->waitingPnrs.push_back(booking.pnr);
        booking.status = "WL";
        booking.seatNo = 0;
        booking.berthType = "Not assigned";
    } else {
        cout << "\nNo confirmed, RAC, or WL space is available in this coach.\n\n";
        return;
    }

    g_bookingsByPnr[booking.pnr] = booking;
    ++g_nextPnr;
    printTicket(booking, fare);
}

void cancelTicketFlow() {
    const int pnr = readIntLine("Enter PNR to cancel: ");
    BookingRecord* booking = findBookingByPnr(pnr);
    if (booking == nullptr) {
        cout << "\nPNR not found.\n\n";
        return;
    }

    if (booking->status == "CANCELLED") {
        cout << "\nThis booking is already cancelled.\n\n";
        return;
    }

    TrainCoachState* coachState = findCoachState(booking->trainId, booking->coachId);
    QueueState* queueState = findQueueState(booking->trainId, booking->coachId);
    if (queueState == nullptr) {
        cout << "\nQueue state for this booking is unavailable.\n\n";
        return;
    }

    const string previousStatus = booking->status;
    booking->refundAmount = computeRefundAmount(booking->totalFare);
    booking->status = "CANCELLED";

    if (previousStatus == "CONFIRMED") {
        if (coachState == nullptr) {
            cout << "\nSeat state for this booking is unavailable.\n\n";
            return;
        }
        Seat* seat = findSeatByPnr(*coachState, booking->pnr);
        if (seat != nullptr) {
            seat->booked = false;
            seat->passengerName = "";
            seat->bookingPnr = 0;
            promoteRacToConfirmed(*queueState, *seat);
        }
    } else if (previousStatus == "RAC") {
        removePnrFromQueue(queueState->racPnrs, booking->pnr);
        promoteWaitingToRac(*queueState);
    } else if (previousStatus == "WL") {
        removePnrFromQueue(queueState->waitingPnrs, booking->pnr);
    }

    cout << "\nTicket cancelled successfully.\n";
    cout << "PNR: " << booking->pnr << '\n';
    cout << "Passenger: " << booking->passengerName << '\n';
    cout << "Previous status: " << previousStatus << '\n';
    cout << "Paid fare: Rs " << booking->totalFare << '\n';
    cout << "Refund: Rs " << booking->refundAmount << '\n';
    cout << "Deduction: " << REFUND_DEDUCTION_PERCENT << "%\n\n";
}

void swapSeatsFlow() {
    const int firstPnr = readIntLine("Enter first passenger PNR: ");
    const int secondPnr = readIntLine("Enter second passenger PNR: ");

    if (firstPnr == secondPnr) {
        cout << "\nPlease enter two different PNRs.\n\n";
        return;
    }

    BookingRecord* firstBooking = findBookingByPnr(firstPnr);
    BookingRecord* secondBooking = findBookingByPnr(secondPnr);
    if (firstBooking == nullptr || secondBooking == nullptr) {
        cout << "\nOne or both PNRs were not found.\n\n";
        return;
    }

    if (!bookingsCanSwapSeats(*firstBooking, *secondBooking)) {
        cout << "\nSeat swap is allowed only for confirmed passengers with the same train, source, destination, and date.\n\n";
        return;
    }

    const string expectedPassword = seatSwapPasswordForBooking(*secondBooking);
    const string enteredPassword = toLowerText(readLine("Enter second passenger confirmation password: "));
    if (enteredPassword != expectedPassword) {
        cout << "\nConfirmation failed. Seat swap cancelled.\n\n";
        return;
    }

    const string firstOldCoachId = firstBooking->coachId;
    const int firstOldSeatNo = firstBooking->seatNo;
    const string secondOldCoachId = secondBooking->coachId;
    const int secondOldSeatNo = secondBooking->seatNo;

    if (!swapConfirmedSeats(*firstBooking, *secondBooking)) {
        cout << "\nSeat swap could not be completed because one of the seat records is missing.\n\n";
        return;
    }

    cout << "\nSeat swap completed successfully.\n";
    cout << "PNR " << firstBooking->pnr << " moved from " << firstOldCoachId << "-" << firstOldSeatNo
         << " to " << firstBooking->coachId << "-" << firstBooking->seatNo << ".\n";
    cout << "PNR " << secondBooking->pnr << " moved from " << secondOldCoachId << "-" << secondOldSeatNo
         << " to " << secondBooking->coachId << "-" << secondBooking->seatNo << ".\n\n";
}

void viewAllTrains() {
    printTrainList();
}

void viewTrainCoachesFlow() {
    printTrainList();
    const int trainChoice = readIntLine("Select train number from the list: ");
    if (trainChoice < 1 || trainChoice > static_cast<int>(getTrains().size())) {
        cout << "\nInvalid train selection.\n\n";
        return;
    }

    const Train& train = getTrains()[static_cast<size_t>(trainChoice - 1)];
    printCoachList(train.id);
}

void viewCoachSeatsFlow() {
    printTrainList();
    const int trainChoice = readIntLine("Select train number from the list: ");
    if (trainChoice < 1 || trainChoice > static_cast<int>(getTrains().size())) {
        cout << "\nInvalid train selection.\n\n";
        return;
    }

    const Train& train = getTrains()[static_cast<size_t>(trainChoice - 1)];
    const vector<Coach> coaches = getCoachesForTrain(train.id);
    const Coach* coach = selectCoachFromTrain(train.id, coaches);
    if (coach == nullptr) {
        return;
    }
    printSeatMap(train.id, coach->coachId);
}

void viewBookingsSummary() {
    cout << "\n--- Booking Summary ---\n";
    if (g_bookingsByPnr.empty()) {
        cout << "No bookings yet.\n\n";
        return;
    }

    for (const pair<const int, BookingRecord>& entry : g_bookingsByPnr) {
        const BookingRecord& booking = entry.second;
        cout << "PNR " << booking.pnr;
        cout << " | " << booking.status;
        cout << " | " << booking.trainNo;
        cout << " | " << booking.passengerName;
        cout << " | " << booking.boarding << " -> " << booking.destination;
        cout << " | " << booking.journeyDate;
        cout << " | " << booking.coachId;
        if (booking.seatNo > 0) {
            cout << "-" << booking.seatNo;
        } else {
            cout << "-NA";
        }
        cout << " | Rs " << booking.totalFare;
        if (booking.status == "CANCELLED") {
            cout << " | Refund Rs " << booking.refundAmount;
        }
        cout << '\n';
    }
    cout << '\n';
}
