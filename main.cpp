// main.cpp - starts the program, loads data, and runs the main menu loop.

#include <iostream>  // Provides console input and output streams.
#include <string>    // Provides std::string for menu input.

#include "booking.h"      // Exposes booking-related menu flow functions.
#include "data_loader.h"  // Exposes CSV loading functions.

using namespace std;      // Keeps the menu code short and readable.

// Entry point of the train booking system.
int main() {
    loadDataFiles();       // Load all CSV-backed station, train, route, and coach data.
    initBookingSystem();   // Build runtime seat maps and reset booking memory.

    while (true) {         // Keep showing the menu until the user chooses to exit.
        cout << "\n=== Train Booking System ===\n";           // Print the menu title.
        cout << "1. Search trains\n";                         // Print menu option 1.
        cout << "2. Book ticket\n";                           // Print menu option 2.
        cout << "3. View all trains\n";                       // Print menu option 3.
        cout << "4. View coaches of a train\n";               // Print menu option 4.
        cout << "5. View seats of a coach\n";                 // Print menu option 5.
        cout << "6. View bookings summary\n";                 // Print menu option 6.
        cout << "7. Cancel ticket by PNR\n";                  // Print menu option 7.
        cout << "8. Swap passenger seats\n";                  // Print menu option 8.
        cout << "9. Exit\n";                                  // Print menu option 9.
        cout << "Choice: ";                                   // Ask the user for a menu choice.

        string line;             // Hold the raw menu choice typed by the user.
        getline(cin, line);      // Read the entire line from standard input.

        if (line == "1") {       // Dispatch option 1.
            searchTrainsFlow();  // Run the search flow.
        } else if (line == "2") {  // Dispatch option 2.
            bookTicketFlow();    // Run the booking flow.
        } else if (line == "3") {  // Dispatch option 3.
            viewAllTrains();     // Show all trains.
        } else if (line == "4") {  // Dispatch option 4.
            viewTrainCoachesFlow();  // Show coaches for a selected train.
        } else if (line == "5") {  // Dispatch option 5.
            viewCoachSeatsFlow(); // Show seats for a selected coach.
        } else if (line == "6") {  // Dispatch option 6.
            viewBookingsSummary();  // Show all bookings made in this run.
        } else if (line == "7") {  // Dispatch option 7.
            cancelTicketFlow();    // Run the cancel-by-PNR flow.
        } else if (line == "8") {  // Dispatch option 8.
            swapSeatsFlow();       // Run the seat-swap flow.
        } else if (line == "9") {  // Dispatch option 9.
            break;               // Exit the main menu loop.
        }
    }

    return 0;                    // End the program successfully.
}
