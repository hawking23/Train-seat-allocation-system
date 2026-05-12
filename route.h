// route.h - declares helpers for route matching and station distance lookup.

#ifndef ROUTE_H  // Prevent duplicate inclusion of this header.
#define ROUTE_H  // Mark this header as included.

#include <string>  // Provides std::string for station names and codes.

#include "train.h"  // Exposes the Train structure used by route matching helpers.

std::string canonicalStationCode(const std::string& input);                               // Normalizes station input to a code when possible.
std::string canonicalStationLabel(const std::string& input);                              // Normalizes station input to a display label when possible.
int routePositionOnTrain(const Train& train, const std::string& stationInput);            // Finds a station position inside a train route.
bool trainCoversRoute(const Train& train, const std::string& boarding, const std::string& destination);  // Checks that a train goes from boarding to destination in order.
int distanceKm(const std::string& boarding, const std::string& destination);              // Sums the distance between two stations on the master line.

#endif  // ROUTE_H
