// data_loader.h - declares helpers that load and expose CSV-backed train data.

#ifndef DATA_LOADER_H  // Prevent duplicate inclusion of this header.
#define DATA_LOADER_H  // Mark this header as included.

#include <string>  // Provides std::string for coach lookup parameters.
#include <vector>  // Provides std::vector for returned collections.

#include "coach.h"    // Exposes the Coach structure used by loader functions.
#include "station.h"  // Exposes the Station structure used by loader functions.
#include "train.h"    // Exposes the Train structure used by loader functions.

void loadDataFiles();                                                   // Loads all CSV files into memory.
const std::vector<Station>& getStations();                              // Returns the shared station list.
const std::vector<Train>& getTrains();                                  // Returns the loaded train list.
const std::vector<Coach>& getCoaches();                                 // Returns the loaded coach list.
const Train* findTrainById(int trainId);                                // Finds one train by its internal id.
const Coach* findCoachById(int trainId, const std::string& coachId);    // Finds one coach by train id and coach id.
std::vector<Coach> getCoachesForTrain(int trainId);                     // Returns all coaches for one train.

#endif  // DATA_LOADER_H
