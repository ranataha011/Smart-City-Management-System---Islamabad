#pragma once
#include "Data_Structures.h"
#include <string>

// ==========================================
// MODULE IDENTIFIERS
// ==========================================
enum ModuleType {
    MODULE_TRANSPORT = 1,
    MODULE_EDUCATION = 2,
    MODULE_MEDICAL = 3,
    MODULE_COMMERCIAL = 4,
    MODULE_PUBLIC_FACILITY = 5,
    MODULE_HOUSING = 6
};

// ==========================================
// COORDINATE STRUCTURE
// ==========================================
struct Coordinates {
    double latitude;
    double longitude;

    Coordinates(double lat = 0.0, double lon = 0.0)
        : latitude(lat), longitude(lon) {
    }

    // Calculate Euclidean distance between two coordinates
    double distanceTo(const Coordinates& other) const {
        double latDiff = latitude - other.latitude;
        double lonDiff = longitude - other.longitude;
        // Simplified distance calculation
        return myAbs(latDiff * latDiff + lonDiff * lonDiff);
    }
};

// ==========================================
// BASE CITY NODE (For Main City Graph)
// ==========================================
struct CityNode {
    string nodeID;          // Unique identifier (e.g., "T_Stop1", "E_School01", "M_Hospital01")
    ModuleType moduleType;  // Which module this node belongs to
    string name;            // Display name
    Coordinates coords;     // Location
    void* entityData;       // Pointer to specific entity data

    CityNode() : entityData(nullptr), moduleType(MODULE_TRANSPORT) {}

    CityNode(string id, ModuleType type, string nodeName, double lat, double lon)
        : nodeID(id), moduleType(type), name(nodeName),
        coords(lat, lon), entityData(nullptr) {
    }
};
