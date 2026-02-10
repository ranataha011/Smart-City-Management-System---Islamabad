#pragma once
#include "CityEntities.h"
#include "MainCityGraph.h"
#include "Data_Structures.h"
#include <iostream>
#include <string>
#include <cctype>
#include <cmath>
using namespace std;

// ==========================================
// PUBLIC FACILITY TYPES
// ==========================================
enum FacilityType {
    FACILITY_MOSQUE = 1,
    FACILITY_PARK = 2,
    FACILITY_WATER_COOLER = 3
};

// ==========================================
// FACILITY DETAILS STRUCTURE
// ==========================================
struct FacilityDetails {
    FacilityType type;
    string facilityName;
    int capacity;           // Capacity in people
    string timings;         // Opening hours
    bool is24Hours;         // For water coolers, some mosques
    string imamName;        // For mosques
    double parkArea;        // For parks (in acres)
    string waterType;       // For coolers: "Filtered", "Mineral", "Normal"
    bool hasWuduArea;       // For mosques
    bool hasPlayground;     // For parks
    int coolersCount;       // Number of coolers at water point

    FacilityDetails() : type(FACILITY_MOSQUE), capacity(0), is24Hours(false),
        parkArea(0.0), hasWuduArea(false), hasPlayground(false),
        coolersCount(1) {
        timings = "06:00-22:00";
        waterType = "Filtered";
    }
};

// ==========================================
// HELPER FUNCTIONS
// ==========================================

// Converts a string to lowercase
string toLowerCase(string str) {
    for (int i = 0; i < (int)str.length(); i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
    return str;
}

// Checks if a string contains a substring (case-insensitive)
bool containsIgnoreCase(string mainStr, string subStr) {
    string mainLower = toLowerCase(mainStr);
    string subLower = toLowerCase(subStr);

    if (mainLower.length() < subLower.length()) {
        return false;
    }

    for (int i = 0; i <= (int)(mainLower.length() - subLower.length()); i++) {
        bool found = true;
        for (int j = 0; j < (int)subLower.length(); j++) {
            if (mainLower[i + j] != subLower[j]) {
                found = false;
                break;
            }
        }
        if (found) {
            return true;
        }
    }
    return false;
}

// ==========================================
// PUBLIC FACILITIES MODULE CLASS (with integrated menu)
// ==========================================
class PublicFacilitiesModule {
private:
    MainCityGraph& cityGraph;           // Reference to main city graph
    HashTable<FacilityDetails> facilitiesHash;  // Store facility details
    LinkedList<string> facilityNodeIDs; // Track all facility nodes

    // Generates a unique facility ID based on type and count
    string generateFacilityID(FacilityType type, int count) {
        string prefix;
        switch (type) {
        case FACILITY_MOSQUE:
            prefix = "MOS";
            break;
        case FACILITY_PARK:
            prefix = "PRK";
            break;
        case FACILITY_WATER_COOLER:
            prefix = "WAT";
            break;
        }
        return prefix + "_" + to_string(count);
    }

    // Returns the human-readable name for a facility type
    string getFacilityTypeName(FacilityType type) {
        switch (type) {
        case FACILITY_MOSQUE:
            return "Mosque";
        case FACILITY_PARK:
            return "Park";
        case FACILITY_WATER_COOLER:
            return "Water Cooler";
        default:
            return "Unknown";
        }
    }

    // Gets integer input from user with validation
    int getIntegerInput(string prompt) {
        int value;
        cout << prompt;
        while (!(cin >> value)) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Invalid input. Please enter a number: ";
        }
        cin.ignore();
        return value;
    }

    // Gets double input from user with validation
    double getDoubleInput(string prompt) {
        double value;
        cout << prompt;
        while (!(cin >> value)) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Invalid input. Please enter a number: ";
        }
        cin.ignore();
        return value;
    }

    // Gets string input from user
    string getStringInput(string prompt) {
        string input;
        cout << prompt;
        getline(cin, input);
        return input;
    }

    // Counts the number of nodes in a linked list
    // Data Structure: LinkedList - O(n) linear traversal
    int countLinkedListNodes(LListNode<string>* head) {
        int count = 0;
        LListNode<string>* current = head;
        while (current != nullptr) {
            count++;
            current = current->next;
        }
        return count;
    }

    // Checks if two nodes are already connected
    // Data Structure: Graph (edge checking)
    bool checkConnectionExists(string nodeID1, string nodeID2) {
        EdgeNode<string>* edges = cityGraph.getCityGraph().getEdges(nodeID1);
        while (edges != nullptr) {
            if (edges->destinationID == nodeID2) {
                return true;
            }
            edges = edges->next;
        }
        return false;
    }

public:
    // Constructor - initializes the Public Facilities module
    PublicFacilitiesModule(MainCityGraph& graph)
        : cityGraph(graph), facilitiesHash(200) {
        cout << "[INFO] Public Facilities Module Initialized" << endl;
    }

    // Connects all facility nodes within the same sector
    // Data Structure: LinkedList (node collection) + Graph (edge creation)
    void connectNodesInSameSector(string sectorName) {
        if (sectorName.empty()) {
            return;
        }
        
        // Find all nodes in this sector
        LinkedList<string> nodesInSector;
        LListNode<string>* node = facilityNodeIDs.getHead();

        while (node != nullptr) {
            CityNode* cityNode = cityGraph.getNode(node->data);
            if (cityNode) {
                string sector = cityGraph.getSectorSystem()
                    .getSectorFromCoordinates(cityNode->coords.latitude,
                        cityNode->coords.longitude);
                if (sector == sectorName) {
                    nodesInSector.insertBack(node->data);
                }
            }
            node = node->next;
        }

        // Connect them in a chain
        int count = countLinkedListNodes(nodesInSector.getHead());
        if (count > 1) {
            cout << "  Connecting " << count << " nodes in sector " << sectorName << "..." << endl;

            LListNode<string>* prev = nullptr;
            LListNode<string>* current = nodesInSector.getHead();

            while (current != nullptr) {
                if (prev != nullptr) {
                    // Check if already connected to avoid duplicates
                    if (!checkConnectionExists(prev->data, current->data)) {
                        CityNode* node1 = cityGraph.getNode(prev->data);
                        CityNode* node2 = cityGraph.getNode(current->data);

                        if (node1 && node2) {
                            double dist = calculateDistance(
                                node1->coords.latitude, node1->coords.longitude,
                                node2->coords.latitude, node2->coords.longitude
                            );

                            if (dist > 0) {
                                cityGraph.connectNodes(prev->data, current->data, (int)dist);
                                cout << "    Connected " << prev->data << " <-> "
                                    << current->data << " (" << (int)dist << " km)" << endl;
                            }
                        }
                    }
                }
                prev = current;
                current = current->next;
            }
        }
    }

    // Connects one node from each of two adjacent sectors
    // Data Structure: LinkedList (node traversal) + Graph (edge creation)
    void connectAdjacentSectorNodes(string sector1, string sector2) {
        if (sector1.empty() || sector2.empty()) {
            return;
        }
        
        // Check if sectors are adjacent
        if (!cityGraph.getSectorSystem().areSectorsAdjacent(sector1, sector2)) {
            return;
        }

        // Find one node from each sector
        string node1 = "";
        string node2 = "";

        LListNode<string>* node = facilityNodeIDs.getHead();
        while (node != nullptr) {
            CityNode* cityNode = cityGraph.getNode(node->data);
            if (cityNode) {
                string sector = cityGraph.getSectorSystem()
                    .getSectorFromCoordinates(cityNode->coords.latitude,
                        cityNode->coords.longitude);

                if (sector == sector1 && node1.empty()) {
                    node1 = node->data;
                }
                else if (sector == sector2 && node2.empty()) {
                    node2 = node->data;
                }
            }

            if (!node1.empty() && !node2.empty()) {
                break;
            }
            node = node->next;
        }

        // Connect them (only if not already connected)
        if (!node1.empty() && !node2.empty()) {
            // Check if already connected to avoid duplicates
            if (!checkConnectionExists(node1, node2)) {
                CityNode* cityNode1 = cityGraph.getNode(node1);
                CityNode* cityNode2 = cityGraph.getNode(node2);

                if (cityNode1 && cityNode2) {
                    double dist = calculateDistance(
                        cityNode1->coords.latitude, cityNode1->coords.longitude,
                        cityNode2->coords.latitude, cityNode2->coords.longitude
                    );

                    if (dist > 0 && dist < 10) {
                        cityGraph.connectNodes(node1, node2, (int)dist);
                        cout << "  Connected adjacent sectors: " << node1 << " <-> "
                            << node2 << " (" << (int)dist << " km)" << endl;
                    }
                }
            }
        }
    }

    // Adds a new mosque to the system
    // Data Structure: HashTable (O(1) facility storage) + LinkedList (node tracking) + Graph (node addition)
    void addMosque(string name, string sector, int capacity,
        string timings, string imamName, bool hasWuduArea) {
        if (name.empty() || sector.empty()) {
            cout << "[ERROR] Mosque name and sector cannot be empty" << endl;
            return;
        }
        
        if (capacity <= 0) {
            cout << "[ERROR] Capacity must be greater than 0" << endl;
            return;
        }
        
        // Generate ID
        int mosqueCount = countFacilitiesByType(FACILITY_MOSQUE);
        string nodeID = generateFacilityID(FACILITY_MOSQUE, mosqueCount + 1);

        // Create facility details
        FacilityDetails details;
        details.type = FACILITY_MOSQUE;
        details.facilityName = name;
        details.capacity = capacity;
        details.timings = timings;
        details.imamName = imamName;
        details.hasWuduArea = hasWuduArea;
        details.is24Hours = (timings == "24/7");

        // Add to city graph with auto-connect
        cityGraph.addNodeInSector(nodeID, MODULE_PUBLIC_FACILITY,
            name, sector, &details);

        // Store in hash table
        facilitiesHash.insert(nodeID, details);
        facilityNodeIDs.insertBack(nodeID);

        cout << "[SUCCESS] Mosque added: " << name << " (" << nodeID << ")" << endl;

        // Connect to existing nodes in same sector
        connectNodesInSameSector(sector);
    }

    // Adds a new park to the system
    // Data Structure: HashTable (O(1) facility storage) + LinkedList (node tracking) + Graph (node addition)
    void addPark(string name, string sector, double area,
        string timings, bool hasPlayground) {
        if (name.empty() || sector.empty()) {
            cout << "[ERROR] Park name and sector cannot be empty" << endl;
            return;
        }
        
        if (area <= 0.0) {
            cout << "[ERROR] Park area must be greater than 0" << endl;
            return;
        }
        
        // Generate ID
        int parkCount = countFacilitiesByType(FACILITY_PARK);
        string nodeID = generateFacilityID(FACILITY_PARK, parkCount + 1);

        // Create facility details
        FacilityDetails details;
        details.type = FACILITY_PARK;
        details.facilityName = name;
        details.parkArea = area;
        details.timings = timings;
        details.hasPlayground = hasPlayground;
        details.capacity = (int)(area * 100); // Estimate: 100 people per acre

        // Add to city graph
        cityGraph.addNodeInSector(nodeID, MODULE_PUBLIC_FACILITY,
            name, sector, &details);

        // Store in hash table
        facilitiesHash.insert(nodeID, details);
        facilityNodeIDs.insertBack(nodeID);

        cout << "[SUCCESS] Park added: " << name << " (" << area << " acres)" << endl;

        // Connect to existing nodes in same sector
        connectNodesInSameSector(sector);
    }

    // Adds a new water cooler to the system
    // Data Structure: HashTable (O(1) facility storage) + LinkedList (node tracking) + Graph (node addition)
    void addWaterCooler(string name, string sector, string waterType,
        int coolersCount, bool is24Hours) {
        if (name.empty() || sector.empty()) {
            cout << "[ERROR] Water cooler name and sector cannot be empty" << endl;
            return;
        }
        
        if (coolersCount <= 0) {
            cout << "[ERROR] Number of coolers must be greater than 0" << endl;
            return;
        }
        
        // Generate ID
        int waterCount = countFacilitiesByType(FACILITY_WATER_COOLER);
        string nodeID = generateFacilityID(FACILITY_WATER_COOLER, waterCount + 1);

        // Create facility details
        FacilityDetails details;
        details.type = FACILITY_WATER_COOLER;
        details.facilityName = name;
        details.waterType = waterType;
        details.coolersCount = coolersCount;
        details.is24Hours = is24Hours;
        details.capacity = coolersCount * 50; // Estimate: 50 people per cooler
        details.timings = is24Hours ? "24/7" : "06:00-22:00";

        // Add to city graph
        cityGraph.addNodeInSector(nodeID, MODULE_PUBLIC_FACILITY,
            name, sector, &details);

        // Store in hash table
        facilitiesHash.insert(nodeID, details);
        facilityNodeIDs.insertBack(nodeID);

        cout << "[SUCCESS] Water Cooler added: " << name << endl;

        // Connect to existing nodes in same sector
        connectNodesInSameSector(sector);
    }

    // Counts facilities of a specific type
    // Data Structure: LinkedList (O(n) traversal) + HashTable (O(1) lookup)
    int countFacilitiesByType(FacilityType type) {
        int count = 0;
        LListNode<string>* current = facilityNodeIDs.getHead();

        while (current != nullptr) {
            FacilityDetails* details = facilitiesHash.search(current->data);
            if (details && details->type == type) {
                count++;
            }
            current = current->next;
        }
        return count;
    }

    // Finds the nearest facility of a specific type from a given node
    // Data Structure: LinkedList (O(n) traversal) + HashTable (O(1) lookup) + Graph (shortest path)
    void findNearestFacility(string currentNodeID, FacilityType facilityType) {
        if (currentNodeID.empty()) {
            cout << "[ERROR] Current node ID cannot be empty" << endl;
            return;
        }
        
        CityNode* currentNode = cityGraph.getNode(currentNodeID);
        if (!currentNode) {
            cout << "[ERROR] Current node not found: " << currentNodeID << endl;
            return;
        }

        cout << endl << "[SEARCH] Finding nearest " << getFacilityTypeName(facilityType)
            << " from " << currentNodeID << "..." << endl;

        string nearestNodeID = "";
        double minDistance = 999999.0;

        // Search through all facilities of matching type
        LListNode<string>* node = facilityNodeIDs.getHead();

        while (node != nullptr) {
            if (node->data == currentNodeID) {
                node = node->next;
                continue;
            }

            FacilityDetails* details = facilitiesHash.search(node->data);
            CityNode* facilityNode = cityGraph.getNode(node->data);

            if (details && facilityNode && details->type == facilityType) {
                double dist = calculateDistance(
                    currentNode->coords.latitude,
                    currentNode->coords.longitude,
                    facilityNode->coords.latitude,
                    facilityNode->coords.longitude
                );

                if (dist < minDistance) {
                    minDistance = dist;
                    nearestNodeID = node->data;
                }
            }

            node = node->next;
        }

        if (nearestNodeID != "") {
            CityNode* nearest = cityGraph.getNode(nearestNodeID);
            FacilityDetails* details = facilitiesHash.search(nearestNodeID);

            if (nearest && details) {
                cout << endl << "[FOUND] Nearest " << getFacilityTypeName(facilityType)
                    << ": " << nearest->name << " (" << nearestNodeID << ")" << endl;
                cout << "Approximate distance: " << (int)minDistance << " km" << endl;

                // Display facility details
                displayFacilityDetails(nearestNodeID);

                // Find route
                cout << endl << "Finding route..." << endl;
                cityGraph.findShortestPath(currentNodeID, nearestNodeID);
            }
        }
        else {
            cout << "[NOT FOUND] No " << getFacilityTypeName(facilityType)
                << " facilities found" << endl;
        }
    }

    // Finds the nearest facility by name (case-insensitive partial match)
    // Data Structure: LinkedList (O(n) traversal) + HashTable (O(1) lookup) + String search
    void findNearestFacilityByName(string currentNodeID, string facilityName) {
        if (currentNodeID.empty()) {
            cout << "[ERROR] Current node ID cannot be empty" << endl;
            return;
        }
        
        if (facilityName.empty()) {
            cout << "[ERROR] Facility name cannot be empty" << endl;
            return;
        }
        
        CityNode* currentNode = cityGraph.getNode(currentNodeID);
        if (!currentNode) {
            cout << "[ERROR] Current node not found: " << currentNodeID << endl;
            return;
        }

        cout << endl << "[SEARCH] Finding nearest facility with name containing: "
            << facilityName << "..." << endl;

        string nearestNodeID = "";
        double minDistance = 999999.0;
        FacilityType foundType = FACILITY_MOSQUE;

        // Search through all facilities
        LListNode<string>* node = facilityNodeIDs.getHead();

        while (node != nullptr) {
            if (node->data == currentNodeID) {
                node = node->next;
                continue;
            }

            FacilityDetails* details = facilitiesHash.search(node->data);
            CityNode* facilityNode = cityGraph.getNode(node->data);

            if (details && facilityNode) {
                // Use custom case-insensitive search
                if (containsIgnoreCase(facilityNode->name, facilityName)) {
                    double dist = calculateDistance(
                        currentNode->coords.latitude,
                        currentNode->coords.longitude,
                        facilityNode->coords.latitude,
                        facilityNode->coords.longitude
                    );

                    if (dist < minDistance) {
                        minDistance = dist;
                        nearestNodeID = node->data;
                        foundType = details->type;
                    }
                }
            }

            node = node->next;
        }

        if (nearestNodeID != "") {
            CityNode* nearest = cityGraph.getNode(nearestNodeID);

            if (nearest) {
                cout << endl << "[FOUND] Nearest matching facility: " << nearest->name
                    << " (" << nearestNodeID << ")" << endl;
                cout << "Type: " << getFacilityTypeName(foundType) << endl;
                cout << "Approximate distance: " << (int)minDistance << " km" << endl;

                // Display facility details
                displayFacilityDetails(nearestNodeID);

                // Find route
                cout << endl << "Finding route..." << endl;
                cityGraph.findShortestPath(currentNodeID, nearestNodeID);
            }
        }
        else {
            cout << "[NOT FOUND] No facilities found with name: " << facilityName << endl;
        }
    }

    // Displays all facilities in the system
    // Data Structure: LinkedList (O(n) traversal) + HashTable (O(1) lookup)
    void displayAllFacilities() {
        cout << endl << "========================================" << endl;
        cout << "       ALL PUBLIC FACILITIES" << endl;
        cout << "========================================" << endl << endl;

        int mosqueCount = 0;
        int parkCount = 0;
        int waterCount = 0;
        LListNode<string>* current = facilityNodeIDs.getHead();

        if (!current) {
            cout << "No facilities found." << endl;
            return;
        }

        while (current != nullptr) {
            displayFacilityDetails(current->data);

            FacilityDetails* details = facilitiesHash.search(current->data);
            if (details) {
                switch (details->type) {
                case FACILITY_MOSQUE:
                    mosqueCount++;
                    break;
                case FACILITY_PARK:
                    parkCount++;
                    break;
                case FACILITY_WATER_COOLER:
                    waterCount++;
                    break;
                }
            }

            current = current->next;
            cout << "----------------------------------------" << endl;
        }

        cout << endl << "========================================" << endl;
        cout << "SUMMARY:" << endl;
        cout << "  Mosques: " << mosqueCount << endl;
        cout << "  Parks: " << parkCount << endl;
        cout << "  Water Coolers: " << waterCount << endl;
        cout << "  Total: " << (mosqueCount + parkCount + waterCount) << endl;
        cout << "========================================" << endl;
    }

    // Displays all facilities of a specific type
    // Data Structure: LinkedList (O(n) traversal) + HashTable (O(1) lookup)
    void displayFacilitiesByType(FacilityType type) {
        string typeName = getFacilityTypeName(type);

        cout << endl << "========================================" << endl;
        cout << "       " << typeName << " FACILITIES" << endl;
        cout << "========================================" << endl << endl;

        int count = 0;
        LListNode<string>* current = facilityNodeIDs.getHead();

        while (current != nullptr) {
            FacilityDetails* details = facilitiesHash.search(current->data);
            CityNode* cityNode = cityGraph.getNode(current->data);

            if (details && cityNode && details->type == type) {
                count++;
                cout << count << ". " << cityNode->name << endl;
                cout << "   ID: " << current->data << endl;
                cout << "   Sector: " << cityGraph.getSectorSystem()
                    .getSectorFromCoordinates(cityNode->coords.latitude,
                        cityNode->coords.longitude) << endl;

                // Type-specific info
                if (type == FACILITY_MOSQUE) {
                    cout << "   Imam: " << details->imamName << endl;
                    cout << "   Capacity: " << details->capacity << " people" << endl;
                    cout << "   Wudu Area: " << (details->hasWuduArea ? "Yes" : "No") << endl;
                }
                else if (type == FACILITY_PARK) {
                    cout << "   Area: " << details->parkArea << " acres" << endl;
                    cout << "   Playground: " << (details->hasPlayground ? "Yes" : "No") << endl;
                }
                else if (type == FACILITY_WATER_COOLER) {
                    cout << "   Water Type: " << details->waterType << endl;
                    cout << "   Coolers: " << details->coolersCount << endl;
                    cout << "   24/7: " << (details->is24Hours ? "Yes" : "No") << endl;
                }

                cout << "   Timings: " << details->timings << endl;
                cout << "----------------------------------------" << endl;
            }

            current = current->next;
        }

        if (count == 0) {
            cout << "No " << typeName << " facilities found." << endl;
        }
        else {
            cout << endl << "Total " << typeName << " Facilities: " << count << endl;
        }
        cout << "========================================" << endl;
    }

    // Displays detailed information about a specific facility
    // Data Structure: HashTable (O(1) lookup) + Graph (node info)
    void displayFacilityDetails(string nodeID) {
        if (nodeID.empty()) {
            cout << "[ERROR] Node ID cannot be empty" << endl;
            return;
        }
        
        CityNode* cityNode = cityGraph.getNode(nodeID);
        FacilityDetails* details = facilitiesHash.search(nodeID);

        if (!cityNode || !details) {
            cout << "[ERROR] Facility not found: " << nodeID << endl;
            return;
        }

        string sector = cityGraph.getSectorSystem()
            .getSectorFromCoordinates(cityNode->coords.latitude,
                cityNode->coords.longitude);

        cout << endl << "FACILITY DETAILS:" << endl;
        cout << "==================" << endl;
        cout << "Name: " << cityNode->name << endl;
        cout << "ID: " << nodeID << endl;
        cout << "Type: " << getFacilityTypeName(details->type) << endl;
        cout << "Sector: " << sector << endl;
        cout << "Coordinates: (" << cityNode->coords.latitude << ", "
            << cityNode->coords.longitude << ")" << endl;

        // Type-specific details
        switch (details->type) {
        case FACILITY_MOSQUE:
            cout << "Imam: " << details->imamName << endl;
            cout << "Capacity: " << details->capacity << " people" << endl;
            cout << "Wudu Area: " << (details->hasWuduArea ? "Available" : "Not Available") << endl;
            break;

        case FACILITY_PARK:
            cout << "Area: " << details->parkArea << " acres" << endl;
            cout << "Estimated Capacity: " << details->capacity << " people" << endl;
            cout << "Playground: " << (details->hasPlayground ? "Available" : "Not Available") << endl;
            break;

        case FACILITY_WATER_COOLER:
            cout << "Water Type: " << details->waterType << endl;
            cout << "Number of Coolers: " << details->coolersCount << endl;
            cout << "Serving Capacity: " << details->capacity << " people/hour" << endl;
            break;
        }

        cout << "Timings: " << details->timings << endl;
        cout << "24/7 Service: " << (details->is24Hours ? "Yes" : "No") << endl;

        // Graph connections
        int degree = cityGraph.getCityGraph().getVertexDegree(nodeID);
        cout << "Connections: " << degree << " adjacent nodes" << endl;
    }

    // Displays statistics about all facilities
    // Data Structure: LinkedList (O(n) traversal) + Graph (degree calculation)
    void displayStatistics() {
        int mosqueCount = countFacilitiesByType(FACILITY_MOSQUE);
        int parkCount = countFacilitiesByType(FACILITY_PARK);
        int waterCount = countFacilitiesByType(FACILITY_WATER_COOLER);
        int total = mosqueCount + parkCount + waterCount;

        cout << endl << "========================================" << endl;
        cout << "  PUBLIC FACILITIES STATISTICS" << endl;
        cout << "========================================" << endl;
        cout << "Mosques: " << mosqueCount << endl;
        cout << "Parks: " << parkCount << endl;
        cout << "Water Coolers: " << waterCount << endl;
        cout << "----------------------------------------" << endl;
        cout << "Total Facilities: " << total << endl;

        if (total > 0) {
            cout << endl << "Distribution:" << endl;
            cout << "  Mosques: " << (mosqueCount * 100 / total) << "%" << endl;
            cout << "  Parks: " << (parkCount * 100 / total) << "%" << endl;
            cout << "  Water Coolers: " << (waterCount * 100 / total) << "%" << endl;
        }

        // Also show connectivity
        cout << endl << "Connectivity:" << endl;
        int connectedCount = 0;
        LListNode<string>* node = facilityNodeIDs.getHead();
        while (node != nullptr) {
            int degree = cityGraph.getCityGraph().getVertexDegree(node->data);
            if (degree > 0) {
                connectedCount++;
            }
            node = node->next;
        }
        cout << "  Connected facilities: " << connectedCount << "/" << total << endl;
        cout << "  Isolated facilities: " << (total - connectedCount) << "/" << total << endl;

        cout << "========================================" << endl;
    }

    // Tests and fixes connections between facilities
    // Data Structure: LinkedList (O(n) traversal) + Graph (degree calculation + path finding)
    void testConnections() {
        cout << "\n=== TESTING FACILITY CONNECTIONS ===\n";

        // Test specific problematic nodes
        cout << "\n1. Testing MOS_2 and PRK_2 connection:\n";

        CityNode* mos2 = cityGraph.getNode("MOS_2");
        CityNode* prk2 = cityGraph.getNode("PRK_2");

        if (mos2 && prk2) {
            string sector1 = cityGraph.getSectorSystem()
                .getSectorFromCoordinates(mos2->coords.latitude, mos2->coords.longitude);
            string sector2 = cityGraph.getSectorSystem()
                .getSectorFromCoordinates(prk2->coords.latitude, prk2->coords.longitude);

            cout << "   MOS_2 Sector: " << sector1 << endl;
            cout << "   PRK_2 Sector: " << sector2 << endl;

            // Check if they're connected
            int degree1 = cityGraph.getCityGraph().getVertexDegree("MOS_2");
            int degree2 = cityGraph.getCityGraph().getVertexDegree("PRK_2");
            cout << "   MOS_2 connections: " << degree1 << endl;
            cout << "   PRK_2 connections: " << degree2 << endl;

            if (sector1 == sector2) {
                cout << "\n2. Same sector! Checking distance...\n";
                double dist = calculateDistance(
                    mos2->coords.latitude, mos2->coords.longitude,
                    prk2->coords.latitude, prk2->coords.longitude
                );
                cout << "   Distance: " << (int)dist << " km\n";

                if (degree1 == 0 || degree2 == 0) {
                    cout << "\n3. One or both nodes are isolated. Connecting...\n";
                    cityGraph.connectNodes("MOS_2", "PRK_2", (int)dist);
                    cout << "   CONNECTED: MOS_2 <-> PRK_2 (" << (int)dist << " km)\n";
                }
            }

            // Test the connection
            cout << "\n4. Testing route between MOS_2 and PRK_2:\n";
            cityGraph.findShortestPath("MOS_2", "PRK_2");
        }
        else {
            cout << "   ERROR: One or both nodes not found!\n";
        }

        // Show overall connectivity
        cout << "\n5. Overall connectivity status:\n";
        int isolatedCount = 0;
        int connectedCount = 0;

        LListNode<string>* node = facilityNodeIDs.getHead();
        while (node != nullptr) {
            int degree = cityGraph.getCityGraph().getVertexDegree(node->data);
            CityNode* cityNode = cityGraph.getNode(node->data);

            if (cityNode) {
                string sector = cityGraph.getSectorSystem()
                    .getSectorFromCoordinates(cityNode->coords.latitude,
                        cityNode->coords.longitude);

                cout << "   " << node->data << " [" << sector << "]: "
                    << degree << " connection(s)\n";

                if (degree == 0) isolatedCount++;
                else connectedCount++;
            }

            node = node->next;
        }

        cout << "\n6. Summary:\n";
        cout << "   Connected facilities: " << connectedCount << endl;
        cout << "   Isolated facilities: " << isolatedCount << endl;
        cout << "   Total facilities: " << (connectedCount + isolatedCount) << endl;

        if (isolatedCount > 0) {
            cout << "\n7. Auto-fixing isolated nodes...\n";
            autoFixIsolatedNodes();
        }
        else {
            cout << "\n7. All facilities are connected! ?\n";
        }
    }

    // Automatically fixes isolated nodes by connecting them to nearby facilities
    // Data Structure: LinkedList (O(n) traversal) + Graph (edge creation)
    void autoFixIsolatedNodes() {
        cout << "\n=== AUTO-FIXING ISOLATED NODES ===\n";
        int fixedCount = 0;

        // Strategy 1: Connect nodes in same sector
        cout << "\nStrategy 1: Connecting nodes in same sectors...\n";
        LListNode<string>* node = facilityNodeIDs.getHead();

        // Get list of all sectors
        LinkedList<string> sectors;
        while (node != nullptr) {
            CityNode* cityNode = cityGraph.getNode(node->data);
            if (cityNode) {
                string sector = cityGraph.getSectorSystem()
                    .getSectorFromCoordinates(cityNode->coords.latitude,
                        cityNode->coords.longitude);

                // Check if sector already in list
                bool found = false;
                LListNode<string>* s = sectors.getHead();
                while (s != nullptr) {
                    if (s->data == sector) {
                        found = true;
                        break;
                    }
                    s = s->next;
                }
                if (!found) {
                    sectors.insertBack(sector);
                }
            }
            node = node->next;
        }

        // Connect nodes in each sector
        LListNode<string>* sectorNode = sectors.getHead();
        while (sectorNode != nullptr) {
            connectNodesInSameSector(sectorNode->data);
            sectorNode = sectorNode->next;
        }

        // Strategy 2: Connect remaining isolated nodes to nearest connected node
        cout << "\nStrategy 2: Connecting remaining isolated nodes...\n";
        node = facilityNodeIDs.getHead();
        while (node != nullptr) {
            int degree = cityGraph.getCityGraph().getVertexDegree(node->data);
            if (degree == 0) {
                // This node is isolated, find nearest node to connect to
                CityNode* isolatedNode = cityGraph.getNode(node->data);
                if (isolatedNode) {
                    string bestTarget = "";
                    double minDist = 999999;

                    LListNode<string>* otherNode = facilityNodeIDs.getHead();
                    while (otherNode != nullptr) {
                        if (node->data == otherNode->data) {
                            otherNode = otherNode->next;
                            continue;
                        }

                        CityNode* targetNode = cityGraph.getNode(otherNode->data);
                        if (targetNode) {
                            double dist = calculateDistance(
                                isolatedNode->coords.latitude, isolatedNode->coords.longitude,
                                targetNode->coords.latitude, targetNode->coords.longitude
                            );

                            if (dist < minDist && dist < 10) { // Max 10 km
                                minDist = dist;
                                bestTarget = otherNode->data;
                            }
                        }
                        otherNode = otherNode->next;
                    }

                    if (!bestTarget.empty() && !checkConnectionExists(node->data, bestTarget)) {
                        cityGraph.connectNodes(node->data, bestTarget, (int)minDist);
                        cout << "   Connected " << node->data << " <-> "
                            << bestTarget << " (" << (int)minDist << " km)\n";
                        fixedCount++;
                    }
                }
            }
            node = node->next;
        }

        if (fixedCount > 0) {
            cout << "\n[SUCCESS] Fixed " << fixedCount << " connections\n";
        }
        else {
            cout << "\n[INFO] No additional connections needed\n";
        }
    }

    // Main menu for Public Facilities module
    void displayMainMenu() {
        // ANSI color codes
        const string RESET = "\033[0m";
        const string BOLD = "\033[1m";
        const string YELLOW = "\033[33m";
        const string BRIGHT_YELLOW = "\033[93m";
        const string WHITE = "\033[37m";
        const string BRIGHT_WHITE = "\033[97m";
        const string CYAN = "\033[36m";
        const string BRIGHT_RED = "\033[91m";

        int choice;

        do {
            cout << endl << BRIGHT_YELLOW << BOLD << "========================================" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "    PUBLIC FACILITIES MANAGEMENT" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "========================================" << RESET << endl << endl;

            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "  FACILITY MANAGEMENT" << RESET << endl;
            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << "  1. Add New Facility      2. Remove Facility" << RESET << endl;
            cout << BRIGHT_WHITE << "  8. Display Facility Details" << RESET << endl << endl;

            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "  SEARCH & QUERIES" << RESET << endl;
            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << "  2. Find Nearest Facility  5. Search Facility by Name" << RESET << endl << endl;

            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "  DISPLAY & STATISTICS" << RESET << endl;
            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << "  3. Display All Facilities  4. Display Facilities by Type" << RESET << endl;
            cout << BRIGHT_WHITE << "  6. View Statistics" << RESET << endl << endl;

            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "  SYSTEM OPERATIONS" << RESET << endl;
            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_WHITE << "  9. Test & Fix Connections" << RESET << endl << endl;

            cout << BRIGHT_WHITE << BOLD << "  0. Return to Main Menu" << RESET << endl;
            cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
            cout << BRIGHT_YELLOW << BOLD << "Choose: " << RESET;

            cin >> choice;
            cin.ignore();

            switch (choice) {
            case 1:
                addFacilityMenu();
                break;
            case 2:
                findNearestFacilityMenu();
                break;
            case 3:
                displayAllFacilities();
                break;
            case 4:
                displayByTypeMenu();
                break;
            case 5:
                searchByNameMenu();
                break;
            case 6:
                displayStatistics();
                break;
            case 7:
                removeFacilityMenu();
                break;
            case 8:
                displayDetailsMenu();
                break;
            case 9:
                testConnections();
                break;
            case 0:
                cout << "Returning to main menu..." << endl;
                break;
            default:
                cout << BRIGHT_RED << "Invalid choice! Please try again." << RESET << endl;
            }

            if (choice != 0) {
                cout << endl << CYAN << "Press Enter to continue..." << RESET;
                cin.get();
            }

        } while (choice != 0);
    }

    // ==========================================
    // SUB-MENUS
    // ==========================================

    void addFacilityMenu() {
        const string RESET = "\033[0m";
        const string BRIGHT_YELLOW = "\033[93m";
        const string YELLOW = "\033[33m";
        const string BRIGHT_WHITE = "\033[97m";
        const string BRIGHT_RED = "\033[91m";

        cout << endl << BRIGHT_YELLOW << "========================================" << RESET << endl;
        cout << BRIGHT_YELLOW << "        ADD NEW FACILITY" << RESET << endl;
        cout << BRIGHT_YELLOW << "========================================" << RESET << endl;
        cout << BRIGHT_WHITE << "1. Add Mosque" << RESET << endl;
        cout << BRIGHT_WHITE << "2. Add Park" << RESET << endl;
        cout << BRIGHT_WHITE << "3. Add Water Cooler" << RESET << endl;
        cout << BRIGHT_WHITE << "4. Back to Main Menu" << RESET << endl;
        cout << YELLOW << "========================================" << RESET << endl;
        cout << BRIGHT_YELLOW << "Enter your choice: " << RESET;

        int choice;
        cin >> choice;
        cin.ignore();

        switch (choice) {
        case 1:
            addMosqueMenu();
            break;
        case 2:
            addParkMenu();
            break;
        case 3:
            addWaterCoolerMenu();
            break;
        case 4:
            return;
        default:
            cout << BRIGHT_RED << "Invalid choice!" << RESET << endl;
        }
    }

    void addMosqueMenu() {
        const string RESET = "\033[0m";
        const string CYAN = "\033[36m";
        const string BRIGHT_YELLOW = "\033[93m";

        cout << endl << BRIGHT_YELLOW << "--- Add New Mosque ---" << RESET << endl;

        string name = getStringInput(CYAN + "Enter mosque name: " + RESET);
        string sector = getStringInput(CYAN + "Enter sector (e.g., G-10, F-8): " + RESET);
        int capacity = getIntegerInput(CYAN + "Enter capacity (people): " + RESET);
        string timings = getStringInput(CYAN + "Enter timings (e.g., 04:00-22:00): " + RESET);
        string imamName = getStringInput(CYAN + "Enter imam name: " + RESET);

        cout << CYAN << "Does it have wudu area? (1=Yes, 0=No): " << RESET;
        bool hasWuduArea;
        cin >> hasWuduArea;
        cin.ignore();

        addMosque(name, sector, capacity, timings, imamName, hasWuduArea);
    }

    void addParkMenu() {
        const string RESET = "\033[0m";
        const string CYAN = "\033[36m";
        const string BRIGHT_YELLOW = "\033[93m";

        cout << endl << BRIGHT_YELLOW << "--- Add New Park ---" << RESET << endl;

        string name = getStringInput(CYAN + "Enter park name: " + RESET);
        string sector = getStringInput(CYAN + "Enter sector (e.g., F-9, G-6): " + RESET);
        double area = getDoubleInput(CYAN + "Enter area (in acres): " + RESET);
        string timings = getStringInput(CYAN + "Enter timings (e.g., 06:00-22:00): " + RESET);

        cout << CYAN << "Does it have playground? (1=Yes, 0=No): " << RESET;
        bool hasPlayground;
        cin >> hasPlayground;
        cin.ignore();

        addPark(name, sector, area, timings, hasPlayground);
    }

    void addWaterCoolerMenu() {
        const string RESET = "\033[0m";
        const string CYAN = "\033[36m";
        const string BRIGHT_YELLOW = "\033[93m";
        const string BRIGHT_WHITE = "\033[97m";

        cout << endl << BRIGHT_YELLOW << "--- Add New Water Cooler ---" << RESET << endl;

        string name = getStringInput(CYAN + "Enter water cooler name/location: " + RESET);
        string sector = getStringInput(CYAN + "Enter sector: " + RESET);

        cout << BRIGHT_WHITE << "Select water type:" << RESET << endl;
        cout << BRIGHT_WHITE << "1. Filtered" << RESET << endl;
        cout << BRIGHT_WHITE << "2. Mineral" << RESET << endl;
        cout << BRIGHT_WHITE << "3. Normal" << RESET << endl;
        cout << CYAN << "Enter choice (1-3): " << RESET;

        int typeChoice;
        cin >> typeChoice;
        cin.ignore();

        string waterType;
        switch (typeChoice) {
        case 1:
            waterType = "Filtered";
            break;
        case 2:
            waterType = "Mineral";
            break;
        case 3:
            waterType = "Normal";
            break;
        default:
            waterType = "Filtered";
        }

        int coolersCount = getIntegerInput(CYAN + "Enter number of coolers: " + RESET);

        cout << CYAN << "Is it 24/7? (1=Yes, 0=No): " << RESET;
        bool is24Hours;
        cin >> is24Hours;
        cin.ignore();

        addWaterCooler(name, sector, waterType, coolersCount, is24Hours);
    }

    void findNearestFacilityMenu() {
        const string RESET = "\033[0m";
        const string CYAN = "\033[36m";
        const string BRIGHT_YELLOW = "\033[93m";
        const string BRIGHT_WHITE = "\033[97m";
        const string BRIGHT_RED = "\033[91m";

        string currentNodeID = getStringInput(CYAN + "Enter your current node ID (e.g., T_Stop1, H_Hospital01): " + RESET);

        cout << endl << BRIGHT_WHITE << "Find nearest:" << RESET << endl;
        cout << BRIGHT_WHITE << "1. Mosque" << RESET << endl;
        cout << BRIGHT_WHITE << "2. Park" << RESET << endl;
        cout << BRIGHT_WHITE << "3. Water Cooler" << RESET << endl;
        cout << CYAN << "Enter choice: " << RESET;

        int choice;
        cin >> choice;
        cin.ignore();

        FacilityType type;
        switch (choice) {
        case 1:
            type = FACILITY_MOSQUE;
            break;
        case 2:
            type = FACILITY_PARK;
            break;
        case 3:
            type = FACILITY_WATER_COOLER;
            break;
        default:
            cout << BRIGHT_RED << "Invalid choice!" << RESET << endl;
            return;
        }

        findNearestFacility(currentNodeID, type);
    }

    void displayByTypeMenu() {
        const string RESET = "\033[0m";
        const string CYAN = "\033[36m";
        const string BRIGHT_WHITE = "\033[97m";
        const string BRIGHT_RED = "\033[91m";

        cout << endl << BRIGHT_WHITE << "Display facilities by type:" << RESET << endl;
        cout << BRIGHT_WHITE << "1. Mosques" << RESET << endl;
        cout << BRIGHT_WHITE << "2. Parks" << RESET << endl;
        cout << BRIGHT_WHITE << "3. Water Coolers" << RESET << endl;
        cout << CYAN << "Enter choice: " << RESET;

        int choice;
        cin >> choice;
        cin.ignore();

        FacilityType type;
        switch (choice) {
        case 1:
            type = FACILITY_MOSQUE;
            break;
        case 2:
            type = FACILITY_PARK;
            break;
        case 3:
            type = FACILITY_WATER_COOLER;
            break;
        default:
            cout << BRIGHT_RED << "Invalid choice!" << RESET << endl;
            return;
        }

        displayFacilitiesByType(type);
    }

    void searchByNameMenu() {
        const string RESET = "\033[0m";
        const string CYAN = "\033[36m";

        string currentNodeID = getStringInput(CYAN + "Enter your current node ID: " + RESET);
        string facilityName = getStringInput(CYAN + "Enter facility name to search: " + RESET);

        findNearestFacilityByName(currentNodeID, facilityName);
    }

    void removeFacilityMenu() {
        const string RESET = "\033[0m";
        const string CYAN = "\033[36m";
        const string BRIGHT_RED = "\033[91m";

        string nodeID = getStringInput(CYAN + "Enter facility node ID to remove: " + RESET);

        // Confirm deletion
        cout << CYAN << "Are you sure you want to remove facility " << nodeID << "? (1=Yes, 0=No): " << RESET;
        bool confirm;
        cin >> confirm;
        cin.ignore();

        if (confirm) {
            removeFacility(nodeID);
        }
        else {
            cout << "Removal cancelled." << endl;
        }
    }

    void displayDetailsMenu() {
        const string RESET = "\033[0m";
        const string CYAN = "\033[36m";

        string nodeID = getStringInput(CYAN + "Enter facility node ID: " + RESET);
        displayFacilityDetails(nodeID);
    }

    // ==========================================
    // DEMO FUNCTION
    // ==========================================
    void runDemo() {
        cout << "\n========================================" << endl;
        cout << "   PUBLIC FACILITIES MODULE DEMO" << endl;
        cout << "========================================" << endl;

        // Show statistics
        displayStatistics();

        cout << "\n\n--- Finding Nearest Facilities Demo ---" << endl;

        // Demo finding nearest mosque from a transport stop
        cout << "\n1. Finding nearest mosque from T_Stop1..." << endl;
        findNearestFacility("T_Stop1", FACILITY_MOSQUE);

        cout << "\n\n2. Finding nearest park from G_10_Markaz..." << endl;
        findNearestFacility("G_10_Markaz", FACILITY_PARK);

        cout << "\n\n3. Finding facility with 'Park' in name..." << endl;
        findNearestFacilityByName("F_8_Kacheri", "Park");

        cout << "\n\n--- Displaying Facilities by Type ---" << endl;
        displayFacilitiesByType(FACILITY_MOSQUE);

        cout << "\nPress Enter to return to menu...";
        cin.get();
    }

    // ==========================================
    // UTILITY FUNCTIONS
    // ==========================================

    // Checks if a facility exists in the system
    // Data Structure: HashTable (O(1) lookup)
    bool facilityExists(string nodeID) {
        return facilitiesHash.search(nodeID) != nullptr;
    }

    // Gets the facility type from a node ID
    // Data Structure: HashTable (O(1) lookup)
    FacilityType getFacilityType(string nodeID) {
        FacilityDetails* details = facilitiesHash.search(nodeID);
        if (details) {
            return details->type;
        }
        return FACILITY_MOSQUE; // Default
    }

    // Returns reference to the list of all facility node IDs
    // Data Structure: LinkedList
    LinkedList<string>& getFacilityNodeIDs() {
        return facilityNodeIDs;
    }

    // Removes a facility from the system
    // Data Structure: HashTable (O(1) removal) + LinkedList (O(n) removal)
    void removeFacility(string nodeID) {
        if (nodeID.empty()) {
            cout << "[ERROR] Node ID cannot be empty" << endl;
            return;
        }
        
        FacilityDetails* details = facilitiesHash.search(nodeID);
        if (!details) {
            cout << "[ERROR] Facility not found: " << nodeID << endl;
            return;
        }

        // Remove from hash table
        facilitiesHash.remove(nodeID);

        // Remove from tracking list
        facilityNodeIDs.removeByValue(nodeID);

        cout << "[SUCCESS] Removed facility: " << nodeID << endl;
    }

    // Calculates the distance between two coordinates using simple distance formula
    double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
        double latDiff = lat1 - lat2;
        double lonDiff = lon1 - lon2;
        double distKm = 111.0 * sqrt((latDiff * latDiff) + (lonDiff * lonDiff));
        return distKm;
    }
};