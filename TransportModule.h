#pragma once
#include "CityEntities.h"
#include <fstream>
#include <sstream>
#include"MainCityGraph.h"

// Forward declaration

struct BusStop {
    string stopID;
    string stopName;
    Coordinates coords;
    LinkedList<string> busesAtStop;

    BusStop() {}
    BusStop(string id, string name, double lat, double lon)
        : stopID(id), stopName(name), coords(lat, lon) {
    }
};
class Passenger; // Forward declaration
struct Bus {
    string busNumber;
    string company;
    string currentStopID;
    LinkedList<string> routeStops;
    int passengerCount;
    int capacity;
    LinkedList<Passenger> passengersOnBoard;  
    Bus() : passengerCount(0), capacity(50) {}
    Bus(string num, string comp, string currentStop)
        : busNumber(num), company(comp), currentStopID(currentStop),
        passengerCount(0), capacity(50) {
    }

};

struct TransportCompany {
    string companyName;
    LinkedList<string> busNumbers;
    int totalBuses;

    TransportCompany() : totalBuses(0) {}
    TransportCompany(string name) : companyName(name), totalBuses(0) {}
};

struct Passenger {
    string name;
    string currentLocation;
    string destination;

    Passenger() {}
    Passenger(string n, string curr, string dest)
        : name(n), currentLocation(curr), destination(dest) {
    }
    // ADD COPY CONSTRUCTOR:
    Passenger(const Passenger& other)
        : name(other.name),
        currentLocation(other.currentLocation),
        destination(other.destination) {
    }

    // ADD ASSIGNMENT OPERATOR:
    Passenger& operator=(const Passenger& other) {
        if (this != &other) {
            name = other.name;
            currentLocation = other.currentLocation;
            destination = other.destination;
        }
        return *this;
    }
};

// ==========================================
// AIRPORT STRUCTURES
// ==========================================
struct Flight {
    string flightNumber;
    string airline;
    string origin;
    string destination;
    string departureTime;
    string arrivalTime;
    int passengerCount;
    int capacity;
    string status;  // "Scheduled", "Boarding", "In Flight", "Landed", "Delayed"

    Flight() : passengerCount(0), capacity(200), status("Scheduled") {}
    Flight(string num, string air, string orig, string dest, string dep, string arr)
        : flightNumber(num), airline(air), origin(orig), destination(dest),
        departureTime(dep), arrivalTime(arr), passengerCount(0), capacity(200), status("Scheduled") {}
};

struct Airport {
    string airportID;
    string airportName;
    Coordinates coords;
    LinkedList<string> terminals;
    LinkedList<string> flights;
    int totalFlights;

    Airport() : totalFlights(0) {}
    Airport(string id, string name, double lat, double lon)
        : airportID(id), airportName(name), coords(lat, lon), totalFlights(0) {}
};

// ==========================================
// RAILWAY STRUCTURES
// ==========================================
struct Train {
    string trainNumber;
    string trainName;
    string originStation;
    string destinationStation;
    string departureTime;
    string arrivalTime;
    int passengerCount;
    int capacity;
    string status;  // "Scheduled", "Boarding", "In Transit", "Arrived", "Delayed"
    LinkedList<string> routeStations;

    Train() : passengerCount(0), capacity(500), status("Scheduled") {}
    Train(string num, string name, string orig, string dest, string dep, string arr)
        : trainNumber(num), trainName(name), originStation(orig), destinationStation(dest),
        departureTime(dep), arrivalTime(arr), passengerCount(0), capacity(500), status("Scheduled") {}
};

struct RailwayStation {
    string stationID;
    string stationName;
    Coordinates coords;
    LinkedList<string> platforms;
    LinkedList<string> trains;
    int totalTrains;

    RailwayStation() : totalTrains(0) {}
    RailwayStation(string id, string name, double lat, double lon)
        : stationID(id), stationName(name), coords(lat, lon), totalTrains(0) {}
};

// ==========================================
// REAL-TIME ROUTE SIMULATION
// ==========================================
struct ActiveRoute {
    string vehicleID;  // Bus number, Flight number, or Train number
    string vehicleType;  // "Bus", "Flight", "Train"
    string currentLocation;
    string destination;
    LinkedList<string> routePath;
    int currentStep;
    int totalSteps;
    int estimatedTimeMinutes;
    int elapsedTimeMinutes;

    ActiveRoute() : currentStep(0), totalSteps(0), estimatedTimeMinutes(0), elapsedTimeMinutes(0) {}
};
class TransportModule {
public:
    MainCityGraph* mainCityGraphPtr;  // SINGLE GRAPH REFERENCE

    // No internal graph!
    HashTable<BusStop> busStopsHash;
    HashTable<Bus> busesHash;
    HashTable<TransportCompany> companiesHash;
    
    // Airport and Railway
    HashTable<Airport> airportsHash;
    HashTable<Flight> flightsHash;
    HashTable<RailwayStation> railwayStationsHash;
    HashTable<Train> trainsHash;

    Stack<string> routeHistory;
    CircularQueue<Passenger> passengerQueue;

    LinkedList<string> allStopIDs;
    LinkedList<string> allBusNumbers;  // Track all buses
    LinkedList<string> allAirportIDs;
    LinkedList<string> allStationIDs;

    // Real-time route simulation
    LinkedList<ActiveRoute> activeRoutes;
    bool simulationRunning;
    int simulationTick;

    int totalStops;
    int totalBuses;

    // Helper: Calculate distance
    int calculateDistance(double lat1, double lon1, double lat2, double lon2) {
        double latDiff = lat1 - lat2;
        double lonDiff = lon1 - lon2;
        double dist = 111.0 * (latDiff * latDiff + lonDiff * lonDiff);
        return (int)(dist * 10);
    }

    // Helper: Trim whitespace
    string trim(string str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (first == string::npos) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }

public:
    TransportModule(MainCityGraph* cityGraph = nullptr)
        : mainCityGraphPtr(cityGraph), busStopsHash(100), busesHash(100),
        companiesHash(50), airportsHash(20), flightsHash(100),
        railwayStationsHash(30), trainsHash(50),
        passengerQueue(200),
        simulationRunning(false), simulationTick(0),
        totalStops(0), totalBuses(0) {
    }

    void setMainCityGraph(MainCityGraph* cityGraph) {
        mainCityGraphPtr = cityGraph;
    }

    // ==========================================
    // COMPANY MANAGEMENT
    // ==========================================
    void registerCompany(string companyName) {
        TransportCompany* existing = companiesHash.search(companyName);
        if (existing) {
            cout << "[ERROR] Company already exists: " << companyName << endl;
            return;
        }

        TransportCompany newCompany(companyName);
        companiesHash.insert(companyName, newCompany);
        cout << "[SUCCESS] Transport company registered: " << companyName << endl;
    }

    // ==========================================
    // BUS STOP MANAGEMENT (SINGLE GRAPH)
    // ==========================================
    void addBusStop(string stopID, string stopName, double lat, double lon) {
        BusStop* existing = busStopsHash.search(stopID);
        if (existing) {
            cout << "[ERROR] Bus stop already exists: " << stopID << endl;
            return;
        }

        // Create bus stop
        BusStop newStop(stopID, stopName, lat, lon);
        busStopsHash.insert(stopID, newStop);

        // Track stop ID
        allStopIDs.insertBack(stopID);
        totalStops++;

        // ADD TO MAIN CITY GRAPH ONLY (Single Source of Truth!)
        if (mainCityGraphPtr != nullptr) {
            string mainGraphNodeID = "T_" + stopID;
            mainCityGraphPtr->addNodeWithAutoConnect(
                mainGraphNodeID,
                MODULE_TRANSPORT,
                stopName,
                lat,
                lon,
                "",
                &newStop
            );
        }
        else {
            cout << "[WARNING] Main City Graph not connected!" << endl;
        }

        cout << "[SUCCESS] Bus stop added: " << stopName << " (" << stopID << ")" << endl;
    }

    void connectBusStops(string stopID1, string stopID2, int distance = -1) {
        BusStop* stop1 = busStopsHash.search(stopID1);
        BusStop* stop2 = busStopsHash.search(stopID2);

        if (!stop1 || !stop2) {
            cout << "[ERROR] One or both stops not found!" << endl;
            return;
        }

        if (distance == -1) {
            distance = calculateDistance(stop1->coords.latitude, stop1->coords.longitude,
                stop2->coords.latitude, stop2->coords.longitude);
        }

        // CONNECT IN MAIN CITY GRAPH ONLY
        if (mainCityGraphPtr != nullptr) {
            string nodeID1 = "T_" + stopID1;
            string nodeID2 = "T_" + stopID2;
            mainCityGraphPtr->connectWithSectorValidation(nodeID1, nodeID2, distance);
        }

        cout << "[SUCCESS] Connected " << stopID1 << " <-> " << stopID2 << endl;
    }

    void deleteBusStop(string stopID) {
        BusStop* stop = busStopsHash.search(stopID);
        if (!stop) {
            cout << "[ERROR] Stop not found: " << stopID << endl;
            return;
        }

        // Remove from Main City Graph (Single deletion!)
        if (mainCityGraphPtr != nullptr) {
            mainCityGraphPtr->removeNode("T_" + stopID);
        }

        // Remove from hash table
        busStopsHash.remove(stopID);

        // Remove from tracking list (simplified)
        totalStops--;

        cout << "[SUCCESS] Bus stop deleted: " << stopID << endl;
    }

    // ==========================================
    // AUTO-CONNECT NEARBY STOPS
    // ==========================================
    void autoConnectNearbyStops(int maxDistanceKM = 5) {
        cout << "\n[AUTO-CONNECT] Connecting nearby bus stops (max " << maxDistanceKM << " km)...\n";

        LListNode<string>* node1 = allStopIDs.getHead();
        int connectionsCreated = 0;

        while (node1 != nullptr) {
            BusStop* stop1 = busStopsHash.search(node1->data);
            if (!stop1) {
                node1 = node1->next;
                continue;
            }

            LListNode<string>* node2 = node1->next;
            while (node2 != nullptr) {
                BusStop* stop2 = busStopsHash.search(node2->data);
                if (stop2) {
                    int dist = calculateDistance(stop1->coords.latitude, stop1->coords.longitude,
                        stop2->coords.latitude, stop2->coords.longitude);

                    if (dist <= maxDistanceKM && dist > 0) {
                        connectBusStops(node1->data, node2->data, dist);
                        connectionsCreated++;
                    }
                }
                node2 = node2->next;
            }
            node1 = node1->next;
        }

        cout << "[SUCCESS] Auto-connected " << connectionsCreated << " stop pairs\n";
    }

    // ==========================================
    // DATA LOADING
    // ==========================================
    void loadDefaultStops() {
        cout << "\n=== LOADING DEFAULT BUS STOPS ===\n";

        // Coordinates aligned with your sector grid system
        // Format: StopID, Name, Latitude, Longitude

        addBusStop("Stop1", "G-10 Markaz", 33.683, 73.054);      // G-10 sector
        addBusStop("Stop2", "G-9 Markaz", 33.693, 73.054);       // G-9 sector
        addBusStop("Stop3", "G-8 Markaz", 33.703, 73.054);       // G-8 sector
        addBusStop("Stop4", "G-7 Markaz", 33.703, 73.066);       // G-7 sector
        addBusStop("Stop5", "G-6 Markaz", 33.713, 73.066);       // G-6 sector
        addBusStop("Stop6", "G-11 Markaz", 33.673, 73.054);      // G-11 sector
        addBusStop("Stop7", "F-10 Park", 33.693, 73.042);        // F-10 sector
        addBusStop("Stop8", "F-11 Markaz", 33.683, 73.042);      // F-11 sector
        addBusStop("Stop9", "F-8 Kacheri", 33.713, 73.042);      // F-8 sector
        addBusStop("Stop10", "F-7 Jinnah", 33.713, 73.054);      // F-7 sector
        addBusStop("Stop11", "F-6 Super", 33.723, 73.054);       // F-6 sector
        addBusStop("Stop12", "F-5 Markaz", 33.723, 73.066);      // F-5 sector
        addBusStop("Stop13", "F-9 Lake View", 33.713, 73.078);   // F-9 sector
        addBusStop("Stop14", "H-8 Stop", 33.703, 73.078);        // H-8 sector
        addBusStop("Stop15", "H-9 Stop", 33.693, 73.078);        // H-9 sector
        addBusStop("Stop16", "H-10 Stop", 33.683, 73.078);       // H-10 sector
        addBusStop("Stop17", "I-8 Stop", 33.703, 73.090);        // I-8 sector
        addBusStop("Stop18", "I-9 Stop", 33.693, 73.090);        // I-9 sector
        addBusStop("Stop19", "Blue Area", 33.719, 73.062);       // Blue Area
        addBusStop("Stop20", "Zero Point", 33.700, 73.057);      // Zero Point
        addBusStop("Stop21", "E-7 Markaz", 33.723, 73.042);      // E-7 sector

        cout << "[SUCCESS] Loaded 21 default bus stops\n";
        cout << "[INFO] Stops automatically connected by Main City Graph\n";
        cout << "[INFO] All coordinates aligned with sector grid system\n";

        // NO MANUAL CONNECTIONS NEEDED!
        // MainCityGraph's addNodeWithAutoConnect() already handled it
    }

    // ==========================================
    // BUS MANAGEMENT
    // ==========================================
    void registerBus(string busNumber, string company) {
        Bus* existing = busesHash.search(busNumber);
        if (existing) {
            cout << "[ERROR] Bus already registered: " << busNumber << endl;
            return;
        }

        TransportCompany* comp = companiesHash.search(company);
        if (!comp) {
            cout << "[WARNING] Company not found. Creating..." << endl;
            registerCompany(company);
            comp = companiesHash.search(company);
        }

        // Ask for number of stops in route
        int numStops;
        cout << "Enter number of stops in route: ";
        cin >> numStops;
        cin.ignore(); // Clear newline

        if (numStops <= 0) {
            cout << "[ERROR] Route must have at least 1 stop!" << endl;
            return;
        }

        // Get first stop (becomes initial location)
        string firstStop;
        cout << "\nStop 1 (Starting location): ";
        getline(cin, firstStop);
        firstStop = trim(firstStop);

        // Validate first stop exists
        BusStop* stop = busStopsHash.search(firstStop);
        if (!stop) {
            cout << "[ERROR] Stop not found: " << firstStop << endl;
            return;
        }

        // Create bus with first stop as current location
        Bus newBus(busNumber, company, firstStop);
        busesHash.insert(busNumber, newBus);

        comp->busNumbers.insertBack(busNumber);
        comp->totalBuses++;
        stop->busesAtStop.insertBack(busNumber);

        allBusNumbers.insertBack(busNumber);
        totalBuses++;

        cout << "[SUCCESS] Bus " << busNumber << " registered at " << firstStop << endl;

        // Add first stop to route
        addStopToRoute(busNumber, firstStop, false); // false = no auto-connect (it's the first stop)

        // Now ask for remaining stops and use addStopToRoute for each
        cout << "\n[BUILDING ROUTE] Enter remaining stops:\n";
        for (int i = 2; i <= numStops; i++) {
            string stopID;
            cout << "Stop " << i << ": ";
            getline(cin, stopID);
            stopID = trim(stopID);

            // addStopToRoute handles validation AND auto-connection!
            addStopToRoute(busNumber, stopID, true); // true = auto-connect
        }

        // Show final route
        cout << "\n[SUCCESS] Bus " << busNumber << " route complete!" << endl;
        cout << "Route: ";
        displayBusRoute(busNumber);
        cout << endl;
    }

    // Helper to display route
    void displayBusRoute(string busNumber) {
        Bus* bus = busesHash.search(busNumber);
        if (!bus) return;

        LListNode<string>* stop = bus->routeStops.getHead();
        while (stop != nullptr) {
            cout << stop->data;
            if (stop->next) cout << " ? ";
            stop = stop->next;
        }
    }
    void loadBusesFromCSV(string filename) {
        cout << "\n=== LOADING BUSES FROM CSV ===\n";
        cout << "Opening file: " << filename << endl;

        ifstream file(filename.c_str());
        if (!file.is_open()) {
            cout << "[ERROR] Could not open file: " << filename << endl;
            cout << "[INFO] Make sure the file exists in the same directory" << endl;
            return;
        }

        string line;
        bool isFirstLine = true;
        int busesLoaded = 0;
        int lineNumber = 0;

        while (getline(file, line)) {
            lineNumber++;

            // Skip header line
            if (isFirstLine) {
                cout << "[INFO] Skipping header: " << line << endl;
                isFirstLine = false;
                continue;
            }

            // Skip empty lines
            line = trim(line);
            if (line.empty()) {
                continue;
            }

            // Parse CSV line: BusNo,Company,CurrentStop,Route
            // Example: B101,MetroBus,Stop3,Stop1 > Stop2 > Stop3 > Stop4

            stringstream ss(line);
            string busNumber, company, currentStop, routeStr;

            // Read fields
            if (!getline(ss, busNumber, ',')) {
                cout << "[WARNING] Line " << lineNumber << ": Could not read Bus Number" << endl;
                continue;
            }
            busNumber = trim(busNumber);

            if (!getline(ss, company, ',')) {
                cout << "[WARNING] Line " << lineNumber << ": Could not read Company" << endl;
                continue;
            }
            company = trim(company);

            if (!getline(ss, currentStop, ',')) {
                cout << "[WARNING] Line " << lineNumber << ": Could not read Current Stop" << endl;
                continue;
            }
            currentStop = trim(currentStop);

            if (!getline(ss, routeStr)) {
                cout << "[WARNING] Line " << lineNumber << ": Could not read Route" << endl;
                continue;
            }
            routeStr = trim(routeStr);

            // Parse route string: "Stop1 > Stop2 > Stop3 > Stop4"
            LinkedList<string> routeStops;
            parseRouteString(routeStr, routeStops);

            if (routeStops.getHead() == nullptr) {
                cout << "[WARNING] Line " << lineNumber << ": Empty route for bus " << busNumber << endl;
                continue;
            }

            // Validate current stop exists
            BusStop* currStop = busStopsHash.search(currentStop);
            if (!currStop) {
                cout << "[ERROR] Line " << lineNumber << ": Current stop not found: " << currentStop << endl;
                continue;
            }

            // Check if bus already exists
            Bus* existingBus = busesHash.search(busNumber);
            if (existingBus) {
                cout << "[WARNING] Line " << lineNumber << ": Bus " << busNumber << " already registered, skipping" << endl;
                continue;
            }

            // Register company if not exists
            TransportCompany* comp = companiesHash.search(company);
            if (!comp) {
                registerCompany(company);
                comp = companiesHash.search(company);
            }

            // Create bus
            Bus newBus(busNumber, company, currentStop);
            busesHash.insert(busNumber, newBus);

            comp->busNumbers.insertBack(busNumber);
            comp->totalBuses++;
            currStop->busesAtStop.insertBack(busNumber);

            allBusNumbers.insertBack(busNumber);
            totalBuses++;

            cout << "\n[LOADING] Bus " << busNumber << " (" << company << ") at " << currentStop << endl;

            // Add all stops to route using addStopToRoute (handles auto-connect!)
            LListNode<string>* stopNode = routeStops.getHead();
            int stopNum = 1;

            while (stopNode != nullptr) {
                cout << "  Adding stop " << stopNum << ": " << stopNode->data;

                // Use addStopToRoute - it does validation AND auto-connection!
                addStopToRoute(busNumber, stopNode->data, true);  // true = auto-connect

                stopNode = stopNode->next;
                stopNum++;
            }

            busesLoaded++;

            // Show final route
            cout << "  [ROUTE] ";
            displayBusRoute(busNumber);
            cout << endl;
        }

        file.close();

        cout << "\n========================================\n";
        cout << "[SUCCESS] Loaded " << busesLoaded << " buses from CSV" << endl;
        cout << "========================================\n";
    }

    // Helper function to parse route string
    void parseRouteString(string routeStr, LinkedList<string>& routeStops) {
        // Handle format: "Stop1 > Stop2 > Stop3" or "Stop1>Stop2>Stop3"

        stringstream ss(routeStr);
        string segment;

        // Split by '>'
        while (getline(ss, segment, '>')) {
            segment = trim(segment);
            if (!segment.empty()) {
                routeStops.insertBack(segment);
            }
        }
    }


    void loadStopsFromCSV(string filename) {
        cout << "\n=== LOADING BUS STOPS FROM CSV ===\n";
        cout << "Opening file: " << filename << endl;

        ifstream file(filename.c_str());
        if (!file.is_open()) {
            cout << "[ERROR] Could not open file: " << filename << endl;
            cout << "[INFO] Make sure the file exists in the same directory as the executable" << endl;
            return;
        }

        string line;
        bool isFirstLine = true;
        int loadedCount = 0;
        int lineNumber = 0;

        while (getline(file, line)) {
            lineNumber++;

            // Skip header line
            if (isFirstLine) {
                cout << "[INFO] Skipping header: " << line << endl;
                isFirstLine = false;
                continue;
            }

            // Skip empty lines
            line = trim(line);
            if (line.empty()) {
                continue;
            }

            // Parse CSV line: StopID,Name,Coordinates
            // Example: Stop1,G-10 Markaz,33.684, 73.025

            stringstream ss(line);
            string stopID, name, coordinatesStr;

            // Read StopID (first field)
            if (!getline(ss, stopID, ',')) {
                cout << "[WARNING] Line " << lineNumber << ": Could not read StopID" << endl;
                continue;
            }
            stopID = trim(stopID);

            // Read Name (second field)
            if (!getline(ss, name, ',')) {
                cout << "[WARNING] Line " << lineNumber << ": Could not read Name" << endl;
                continue;
            }
            name = trim(name);

            // Read rest as coordinates (third field contains: "33.684, 73.025")
            if (!getline(ss, coordinatesStr)) {
                cout << "[WARNING] Line " << lineNumber << ": Could not read Coordinates" << endl;
                continue;
            }
            coordinatesStr = trim(coordinatesStr);

            // Parse coordinates: "33.684, 73.025"
            double latitude = 0.0;
            double longitude = 0.0;

            // Split coordinates by comma
            size_t commaPos = coordinatesStr.find(',');
            if (commaPos != string::npos) {
                string latStr = trim(coordinatesStr.substr(0, commaPos));
                string lonStr = trim(coordinatesStr.substr(commaPos + 1));

                // Convert strings to double
                latitude = stringToDouble(latStr);
                longitude = stringToDouble(lonStr);

                // Validate coordinates
                if (latitude == 0.0 && longitude == 0.0) {
                    cout << "[WARNING] Line " << lineNumber << ": Invalid coordinates" << endl;
                    continue;
                }
            }
            else {
                cout << "[WARNING] Line " << lineNumber << ": Coordinates format error" << endl;
                continue;
            }

            // Add the bus stop
            cout << "[LOADING] " << stopID << " - " << name
                << " (" << latitude << ", " << longitude << ")" << endl;

            addBusStop(stopID, name, latitude, longitude);
            loadedCount++;
        }

        file.close();

        cout << "\n========================================\n";
        cout << "[SUCCESS] Loaded " << loadedCount << " stops from CSV" << endl;
        cout << "========================================\n";
    }
    void boardPassengers(string busNumber, string currentStopID) {
        Bus* bus = busesHash.search(busNumber);
        if (!bus) return;

        // Check if queue has passengers
        if (passengerQueue.isEmpty()) {
            cout << "[INFO] No passengers waiting" << endl;
            return;
        }

        // Board passengers who are at this stop
        CircularQueue<Passenger> tempQueue(200);
        int boarded = 0;

        while (!passengerQueue.isEmpty() && bus->passengerCount < bus->capacity) {
            Passenger p = passengerQueue.dequeue();

            // Check if passenger is at this stop
            if (p.currentLocation == currentStopID) {
                bus->passengerCount++;
                boarded++;
                cout << "[BOARDED] " << p.name << " boarded bus " << busNumber
                    << " at " << currentStopID << endl;

                // Store passenger with bus (need to add list to Bus struct)
                // bus->passengersOnBoard.insertBack(p);
            }
            else {
                // Put back in queue if not at this stop
                tempQueue.enqueue(p);
            }
        }

        // Restore unboarded passengers to queue
        while (!tempQueue.isEmpty()) {
            passengerQueue.enqueue(tempQueue.dequeue());
        }

        cout << "[INFO] " << boarded << " passengers boarded. Bus now has "
            << bus->passengerCount << "/" << bus->capacity << " passengers" << endl;
    }


    // Helper function to convert string to double
    double stringToDouble(string str) {
        double result = 0.0;
        bool negative = false;
        bool decimal = false;
        double decimalPlace = 0.1;

        // Remove whitespace
        str = trim(str);

        for (size_t i = 0; i < str.length(); i++) {
            char c = str[i];

            if (c == '-' && i == 0) {
                negative = true;
            }
            else if (c == '.') {
                decimal = true;
            }
            else if (c >= '0' && c <= '9') {
                if (!decimal) {
                    result = result * 10.0 + (c - '0');
                }
                else {
                    result += (c - '0') * decimalPlace;
                    decimalPlace *= 0.1;
                }
            }
        }

        return negative ? -result : result;
    }
    bool checkConnectionExists(string nodeID1, string nodeID2) {
        if (!mainCityGraphPtr) {
            return false;  // Can't check without graph
        }

        // Get edges from node1 using Graph's getEdges() method
        EdgeNode<string>* edges = mainCityGraphPtr->getCityGraph().getEdges(nodeID1);

        // Search through the edge list to see if node2 is connected
        while (edges != nullptr) {
            if (edges->destinationID == nodeID2) {
                return true;  // Connection found!
            }
            edges = edges->next;
        }

        return false;  // No connection exists
    }
    void addStopToRoute(string busNumber, string stopID, bool autoConnect = true) {
        Bus* bus = busesHash.search(busNumber);

        if (!bus) {
            cout << "[ERROR] Bus not found!" << endl;
            return;
        }

        // Check if node exists in Main City Graph
        if (mainCityGraphPtr) {
            CityNode* node = mainCityGraphPtr->getNode("T_" + stopID);

            if (!node) {
                node = mainCityGraphPtr->getNode(stopID);
            }

            if (node) {
                // Find last stop in current route
                LListNode<string>* lastStop = nullptr;
                LListNode<string>* temp = bus->routeStops.getHead();

                while (temp != nullptr) {
                    lastStop = temp;
                    temp = temp->next;
                }

                // AUTO-CONNECT: If route has stops and auto-connect is enabled
                if (lastStop != nullptr && autoConnect) {
                    string prevStopNodeID = "T_" + lastStop->data;
                    string newStopNodeID = "T_" + stopID;

                    // Check if connection already exists
                    if (!checkConnectionExists(prevStopNodeID, newStopNodeID)) {
                        BusStop* prevStop = busStopsHash.search(lastStop->data);
                        BusStop* newStop = busStopsHash.search(stopID);

                        cout << "[AUTO-CONNECT] No connection found between "
                            << (prevStop ? prevStop->stopName : lastStop->data)
                            << " and " << stopID << endl;
                        cout << "[AUTO-CONNECT] Creating connection automatically..." << endl;

                        // Create the connection!
                        connectBusStops(lastStop->data, stopID);

                        cout << "[SUCCESS] Route connection created automatically" << endl;
                    }
                    else {
                        cout << "[INFO] Connection already exists - no action needed" << endl;
                    }
                }

                // Add stop to route
                bus->routeStops.insertBack(stopID);

                if (node->moduleType == MODULE_EDUCATION) {
                    cout << "[INFO] Bus will stop at Education facility: " << stopID << endl;
                }
                else if (node->moduleType == MODULE_MEDICAL) {
                    cout << "[INFO] Bus will stop at Medical facility: " << stopID << endl;
                }

                cout << "[SUCCESS] Added " << stopID << " to route" << endl;
            }
            else {
                cout << "[ERROR] Node not found in city graph!" << endl;
            }
        }
        else {
            // Fallback
            bus->routeStops.insertBack(stopID);
            cout << "[WARNING] Added without validation (Main Graph not connected)" << endl;
        }
    }

    void updateBusLocation(string busNumber, string newStopID) {
        Bus* bus = busesHash.search(busNumber);

        if (!bus) {
            cout << "[ERROR] Bus not found!" << endl;
            return;
        }

        bus->currentStopID = newStopID;

        BusStop* newStop = busStopsHash.search(newStopID);
        if (newStop) {
            newStop->busesAtStop.insertBack(busNumber);
        }

        cout << "[INFO] Bus " << busNumber << " now at " << newStopID << endl;
    }
    void alightPassengers(string busNumber, string currentStopID) {
        Bus* bus = busesHash.search(busNumber);
        if (!bus) return;

        if (bus->passengerCount == 0) {
            return; // No passengers to alight
        }

        // Need to track which passengers are on bus
        // For now, simulate random alighting
        int alighted = 0;

        // Check each passenger if this is their destination
        // (You need to add LinkedList<Passenger> passengersOnBoard to Bus struct)

        if (alighted > 0) {
            bus->passengerCount -= alighted;
            cout << "[ALIGHTED] " << alighted << " passengers got off at "
                << currentStopID << endl;
            cout << "[INFO] Bus now has " << bus->passengerCount
                << "/" << bus->capacity << " passengers" << endl;
        }
    }


    // ==========================================
    // BUS SIMULATION (NEW!)
    // ==========================================
    void startSimulation() {
        simulationRunning = true;
        simulationTick = 0;
        cout << "\n[SIMULATION] Started! Buses will move along routes.\n";
    }

    void stopSimulation() {
        simulationRunning = false;
        cout << "\n[SIMULATION] Stopped.\n";
    }

    void simulateOneTick() {
        if (!simulationRunning) return;

        simulationTick++;
        cout << "\n=== SIMULATION TICK " << simulationTick << " ===\n";

        LListNode<string>* busNode = allBusNumbers.getHead();

        while (busNode != nullptr) {
            Bus* bus = busesHash.search(busNode->data);

            if (bus) {
                // 1. Alight passengers at current stop
                alightPassengers(bus->busNumber, bus->currentStopID);

                // 2. Board new passengers at current stop
                boardPassengers(bus->busNumber, bus->currentStopID);

                // 3. Move bus to next stop
                moveBusToNextStop(bus);
            }

            busNode = busNode->next;
        }
    }


    void moveBusToNextStop(Bus* bus) {
        if (!bus) return;

        // Find current position in route
        LListNode<string>* routeNode = bus->routeStops.getHead();
        bool foundCurrent = false;
        LListNode<string>* nextStop = nullptr;

        while (routeNode != nullptr) {
            if (foundCurrent) {
                nextStop = routeNode;
                break;
            }

            if (routeNode->data == bus->currentStopID) {
                foundCurrent = true;
            }

            routeNode = routeNode->next;
        }

        // If at end, loop back to start
        if (!nextStop && foundCurrent) {
            nextStop = bus->routeStops.getHead();
        }

        if (nextStop) {
            updateBusLocation(bus->busNumber, nextStop->data);

            BusStop* stop = busStopsHash.search(nextStop->data);
            if (stop) {
                cout << "[SIMULATED] Bus " << bus->busNumber << " ? "
                    << stop->stopName << " (" << nextStop->data << ")" << endl;
            }
            else {
                cout << "[SIMULATED] Bus " << bus->busNumber << " ? "
                    << nextStop->data << endl;
            }
        }
    }

    void displayLiveBusPositions() {
        cout << "\n=== LIVE BUS POSITIONS ===\n";

        LListNode<string>* busNode = allBusNumbers.getHead();
        int count = 1;

        while (busNode != nullptr) {
            Bus* bus = busesHash.search(busNode->data);

            if (bus) {
                BusStop* stop = busStopsHash.search(bus->currentStopID);

                cout << count++ << ". Bus " << bus->busNumber
                    << " (" << bus->company << ") at ";

                if (stop) {
                    cout << stop->stopName << " (" << bus->currentStopID << ")";
                }
                else {
                    cout << bus->currentStopID;
                }

                cout << " [Passengers: " << bus->passengerCount << "/"
                    << bus->capacity << "]" << endl;
            }

            busNode = busNode->next;
        }

        if (count == 1) {
            cout << "No buses registered.\n";
        }
    }

    // ==========================================
    // ROUTING (USES MAIN GRAPH)
    // ==========================================
    void findShortestRoute(string startStop, string endStop) {
        BusStop* start = busStopsHash.search(startStop);
        BusStop* end = busStopsHash.search(endStop);

        if (!start || !end) {
            cout << "[ERROR] Start or end stop not found!" << endl;
            return;
        }

        if (mainCityGraphPtr) {
            cout << "\n=== FINDING SHORTEST ROUTE (Main City Graph) ===\n";
            cout << "From: " << start->stopName << " (" << startStop << ")" << endl;
            cout << "To: " << end->stopName << " (" << endStop << ")" << endl;

            routeHistory.push(startStop);

            // Use Main City Graph with T_ prefix
            string startNode = "T_" + startStop;
            string endNode = "T_" + endStop;
            mainCityGraphPtr->findShortestPath(startNode, endNode);

            routeHistory.push(endStop);
        }
        else {
            cout << "[ERROR] Main City Graph not connected!" << endl;
        }
    }

    // ==========================================
    // QUERY OPERATIONS
    // ==========================================
    void getBusDetails(string busNumber) {
        Bus* bus = busesHash.search(busNumber);

        if (!bus) {
            cout << "[ERROR] Bus not found: " << busNumber << endl;
            return;
        }

        BusStop* currentStop = busStopsHash.search(bus->currentStopID);

        cout << "\n=== BUS DETAILS ===\n";
        cout << "Bus Number: " << bus->busNumber << endl;
        cout << "Company: " << bus->company << endl;
        cout << "Current Stop: " << bus->currentStopID;
        if (currentStop) {
            cout << " (" << currentStop->stopName << ")";
        }
        cout << endl;
        cout << "Passengers: " << bus->passengerCount << "/" << bus->capacity << endl;
        cout << "Route: ";

        LListNode<string>* temp = bus->routeStops.getHead();
        if (!temp) {
            cout << "No route defined";
        }
        else {
            while (temp) {
                cout << temp->data;
                if (temp->next) cout << " ? ";
                temp = temp->next;
            }
        }
        cout << endl;
    }

    void getStopDetails(string stopID) {
        BusStop* stop = busStopsHash.search(stopID);

        if (!stop) {
            cout << "[ERROR] Stop not found: " << stopID << endl;
            return;
        }

        cout << "\n=== BUS STOP DETAILS ===\n";
        cout << "Stop ID: " << stop->stopID << endl;
        cout << "Name: " << stop->stopName << endl;
        cout << "Coordinates: (" << stop->coords.latitude << ", "
            << stop->coords.longitude << ")" << endl;
        cout << "Buses at stop: ";

        LListNode<string>* temp = stop->busesAtStop.getHead();
        if (!temp) {
            cout << "None";
        }
        else {
            while (temp) {
                cout << temp->data;
                if (temp->next) cout << ", ";
                temp = temp->next;
            }
        }
        cout << endl;
    }

    // ==========================================
    // AIRPORT MANAGEMENT
    // ==========================================
    // Data Structure: HashTable - O(1) average lookup for airports
    void addAirport(string airportID, string airportName, double lat, double lon) {
        Airport* existing = airportsHash.search(airportID);
        if (existing) {
            cout << "[ERROR] Airport already exists: " << airportID << endl;
            return;
        }

        Airport newAirport(airportID, airportName, lat, lon);
        airportsHash.insert(airportID, newAirport);
        allAirportIDs.insertBack(airportID);

        // Add to main city graph
        if (mainCityGraphPtr != nullptr) {
            string mainGraphNodeID = "AIR_" + airportID;
            mainCityGraphPtr->addNodeWithAutoConnect(
                mainGraphNodeID,
                MODULE_TRANSPORT,
                airportName,
                lat,
                lon,
                "",
                &newAirport
            );
        }

        cout << "[SUCCESS] Airport added: " << airportName << " (" << airportID << ")" << endl;
    }

    // Data Structure: HashTable - O(1) average lookup for flights
    void addFlight(string flightNumber, string airline, string origin, string destination,
        string departureTime, string arrivalTime) {
        Flight* existing = flightsHash.search(flightNumber);
        if (existing) {
            cout << "[ERROR] Flight already exists: " << flightNumber << endl;
            return;
        }

        Flight newFlight(flightNumber, airline, origin, destination, departureTime, arrivalTime);
        flightsHash.insert(flightNumber, newFlight);

        // Add to origin airport
        Airport* originAirport = airportsHash.search(origin);
        if (originAirport) {
            originAirport->flights.insertBack(flightNumber);
            originAirport->totalFlights++;
        }

        cout << "[SUCCESS] Flight added: " << flightNumber << " (" << airline << ")" << endl;
        cout << "[INFO] Route: " << origin << " -> " << destination << endl;
        cout << "[INFO] Departure: " << departureTime << ", Arrival: " << arrivalTime << endl;
    }

    void displayAllAirports() {
        cout << "\n=== ALL AIRPORTS ===\n";
        LListNode<string>* current = allAirportIDs.getHead();
        int count = 1;

        while (current != nullptr) {
            Airport* airport = airportsHash.search(current->data);
            if (airport) {
                cout << count++ << ". " << airport->airportName
                    << " (" << airport->airportID << ") - ["
                    << airport->coords.latitude << ", "
                    << airport->coords.longitude << "]"
                    << " - Flights: " << airport->totalFlights << endl;
            }
            current = current->next;
        }

        if (count == 1) {
            cout << "No airports registered.\n";
        }
    }

    // ==========================================
    // RAILWAY STATION MANAGEMENT
    // ==========================================
    // Data Structure: HashTable - O(1) average lookup for stations
    void addRailwayStation(string stationID, string stationName, double lat, double lon) {
        RailwayStation* existing = railwayStationsHash.search(stationID);
        if (existing) {
            cout << "[ERROR] Railway station already exists: " << stationID << endl;
            return;
        }

        RailwayStation newStation(stationID, stationName, lat, lon);
        railwayStationsHash.insert(stationID, newStation);
        allStationIDs.insertBack(stationID);

        // Add to main city graph
        if (mainCityGraphPtr != nullptr) {
            string mainGraphNodeID = "RAIL_" + stationID;
            mainCityGraphPtr->addNodeWithAutoConnect(
                mainGraphNodeID,
                MODULE_TRANSPORT,
                stationName,
                lat,
                lon,
                "",
                &newStation
            );
        }

        cout << "[SUCCESS] Railway station added: " << stationName << " (" << stationID << ")" << endl;
    }

    // Data Structure: HashTable - O(1) average lookup for trains
    void addTrain(string trainNumber, string trainName, string originStation,
        string destinationStation, string departureTime, string arrivalTime) {
        Train* existing = trainsHash.search(trainNumber);
        if (existing) {
            cout << "[ERROR] Train already exists: " << trainNumber << endl;
            return;
        }

        Train newTrain(trainNumber, trainName, originStation, destinationStation,
            departureTime, arrivalTime);
        trainsHash.insert(trainNumber, newTrain);

        // Add to origin station
        RailwayStation* originStationPtr = railwayStationsHash.search(originStation);
        if (originStationPtr) {
            originStationPtr->trains.insertBack(trainNumber);
            originStationPtr->totalTrains++;
        }

        cout << "[SUCCESS] Train added: " << trainNumber << " (" << trainName << ")" << endl;
        cout << "[INFO] Route: " << originStation << " -> " << destinationStation << endl;
        cout << "[INFO] Departure: " << departureTime << ", Arrival: " << arrivalTime << endl;
    }

    void displayAllRailwayStations() {
        cout << "\n=== ALL RAILWAY STATIONS ===\n";
        LListNode<string>* current = allStationIDs.getHead();
        int count = 1;

        while (current != nullptr) {
            RailwayStation* station = railwayStationsHash.search(current->data);
            if (station) {
                cout << count++ << ". " << station->stationName
                    << " (" << station->stationID << ") - ["
                    << station->coords.latitude << ", "
                    << station->coords.longitude << "]"
                    << " - Trains: " << station->totalTrains << endl;
            }
            current = current->next;
        }

        if (count == 1) {
            cout << "No railway stations registered.\n";
        }
    }

    // ==========================================
    // REAL-TIME ROUTE SIMULATION
    // ==========================================
    // Data Structure: LinkedList - O(1) insertion for active routes
    void startRouteSimulation(string vehicleID, string vehicleType, string startLocation, string destination) {
        ActiveRoute newRoute;
        newRoute.vehicleID = vehicleID;
        newRoute.vehicleType = vehicleType;
        newRoute.currentLocation = startLocation;
        newRoute.destination = destination;
        newRoute.currentStep = 0;
        newRoute.elapsedTimeMinutes = 0;

        // Calculate route path based on vehicle type
        if (vehicleType == "Bus") {
            Bus* bus = busesHash.search(vehicleID);
            if (bus) {
                LListNode<string>* stop = bus->routeStops.getHead();
                while (stop) {
                    newRoute.routePath.insertBack(stop->data);
                    stop = stop->next;
                }
                newRoute.totalSteps = 0;
                LListNode<string>* temp = newRoute.routePath.getHead();
                while (temp) {
                    newRoute.totalSteps++;
                    temp = temp->next;
                }
                newRoute.estimatedTimeMinutes = newRoute.totalSteps * 5; // 5 min per stop
            }
        }
        else if (vehicleType == "Flight") {
            Flight* flight = flightsHash.search(vehicleID);
            if (flight) {
                newRoute.routePath.insertBack(flight->origin);
                newRoute.routePath.insertBack(flight->destination);
                newRoute.totalSteps = 2;
                newRoute.estimatedTimeMinutes = 120; // 2 hours default
            }
        }
        else if (vehicleType == "Train") {
            Train* train = trainsHash.search(vehicleID);
            if (train) {
                newRoute.routePath.insertBack(train->originStation);
                newRoute.routePath.insertBack(train->destinationStation);
                newRoute.totalSteps = 2;
                newRoute.estimatedTimeMinutes = 180; // 3 hours default
            }
        }

        activeRoutes.insertBack(newRoute);
        cout << "[SIMULATION] Started tracking " << vehicleType << " " << vehicleID << endl;
        cout << "[INFO] Route: " << startLocation << " -> " << destination << endl;
        cout << "[INFO] Estimated time: " << newRoute.estimatedTimeMinutes << " minutes" << endl;
    }

    // Data Structure: LinkedList - O(n) traversal for all active routes
    void updateRouteSimulation() {
        if (activeRoutes.getHead() == nullptr) {
            cout << "[INFO] No active routes to simulate" << endl;
            return;
        }

        cout << "\n=== REAL-TIME ROUTE SIMULATION UPDATE ===\n";
        LListNode<ActiveRoute>* routeNode = activeRoutes.getHead();
        LinkedList<string> completedVehicleIDs;  // Track completed by ID

        while (routeNode != nullptr) {
            ActiveRoute& route = routeNode->data;
            route.elapsedTimeMinutes += 1; // Simulate 1 minute passing

            // Move to next step if enough time has passed
            if (route.currentStep < route.totalSteps - 1) {
                int timePerStep = (route.totalSteps > 0) ? (route.estimatedTimeMinutes / route.totalSteps) : 1;
                if (timePerStep == 0) timePerStep = 1;
                if (route.elapsedTimeMinutes >= (route.currentStep + 1) * timePerStep) {
                    route.currentStep++;
                    LListNode<string>* pathNode = route.routePath.getHead();
                    for (int i = 0; i < route.currentStep && pathNode; i++) {
                        pathNode = pathNode->next;
                    }
                    if (pathNode) {
                        route.currentLocation = pathNode->data;
                    }
                }
            }

            // Display current status
            cout << "\n[" << route.vehicleType << "] " << route.vehicleID << endl;
            cout << "  Current Location: " << route.currentLocation << endl;
            cout << "  Destination: " << route.destination << endl;
            cout << "  Progress: " << route.currentStep << "/" << route.totalSteps << " steps" << endl;
            cout << "  Time Elapsed: " << route.elapsedTimeMinutes << "/" << route.estimatedTimeMinutes << " minutes" << endl;

            // Check if route completed
            if (route.currentStep >= route.totalSteps - 1 && 
                route.currentLocation == route.destination) {
                cout << "  Status: [ARRIVED]" << endl;
                completedVehicleIDs.insertBack(route.vehicleID + "::" + route.vehicleType);
            }
            else {
                int progress = (route.estimatedTimeMinutes > 0) ? 
                    (route.elapsedTimeMinutes * 100) / route.estimatedTimeMinutes : 0;
                if (progress > 100) progress = 100;
                cout << "  Status: [IN TRANSIT] - " << progress << "% complete" << endl;
            }

            routeNode = routeNode->next;
        }

        // Remove completed routes by rebuilding list
        if (completedVehicleIDs.getHead() != nullptr) {
            LinkedList<ActiveRoute> newActiveRoutes;
            LListNode<ActiveRoute>* current = activeRoutes.getHead();
            while (current != nullptr) {
                string currentKey = current->data.vehicleID + "::" + current->data.vehicleType;
                bool isCompleted = false;
                LListNode<string>* check = completedVehicleIDs.getHead();
                while (check != nullptr) {
                    if (check->data == currentKey) {
                        isCompleted = true;
                        break;
                    }
                    check = check->next;
                }
                if (!isCompleted) {
                    newActiveRoutes.insertBack(current->data);
                }
                current = current->next;
            }
            activeRoutes = newActiveRoutes;
            
            int completedCount = 0;
            LListNode<string>* temp = completedVehicleIDs.getHead();
            while (temp != nullptr) {
                completedCount++;
                temp = temp->next;
            }
            cout << "\n[INFO] " << completedCount << " route(s) completed and removed" << endl;
        }
    }

    void displayActiveRoutes() {
        cout << "\n=== ACTIVE ROUTES (REAL-TIME) ===\n";
        if (activeRoutes.getHead() == nullptr) {
            cout << "No active routes being tracked." << endl;
            return;
        }

        LListNode<ActiveRoute>* routeNode = activeRoutes.getHead();
        int count = 1;

        while (routeNode != nullptr) {
            ActiveRoute& route = routeNode->data;
            cout << count++ << ". [" << route.vehicleType << "] " << route.vehicleID << endl;
            cout << "   Route: " << route.currentLocation << " -> " << route.destination << endl;
            cout << "   Progress: " << route.currentStep << "/" << route.totalSteps << " steps" << endl;
            cout << "   Time: " << route.elapsedTimeMinutes << "/" << route.estimatedTimeMinutes << " min" << endl;
            routeNode = routeNode->next;
        }
    }

    // ==========================================
    // PASSENGER MANAGEMENT
    // ==========================================
    void addPassengerToQueue(string name, string currentLoc, string dest) {
        Passenger p(name, currentLoc, dest);
        if (passengerQueue.enqueue(p)) {
            cout << "[SUCCESS] Passenger " << name << " added to queue" << endl;
        }
    }

    void processNextPassenger() {
        if (passengerQueue.isEmpty()) {
            cout << "[INFO] No passengers in queue" << endl;
            return;
        }

        Passenger p = passengerQueue.dequeue();
        cout << "\n[PROCESSING] Passenger: " << p.name << endl;
        findShortestRoute(p.currentLocation, p.destination);
    }

    // ==========================================
    // DISPLAY OPERATIONS
    // ==========================================
    void displayTransportNetwork() {
        if (mainCityGraphPtr) {
            mainCityGraphPtr->displayNodesByModuleType(MODULE_TRANSPORT);
        }
        else {
            cout << "[ERROR] Main City Graph not connected!" << endl;
        }
    }

    void displayAllStops() {
        cout << "\n=== ALL BUS STOPS ===\n";

        LListNode<string>* current = allStopIDs.getHead();
        int count = 1;

        while (current != nullptr) {
            BusStop* stop = busStopsHash.search(current->data);
            if (stop) {
                cout << count++ << ". " << stop->stopName
                    << " (" << stop->stopID << ") - ["
                    << stop->coords.latitude << ", "
                    << stop->coords.longitude << "]" << endl;
            }
            current = current->next;
        }

        if (count == 1) {
            cout << "No bus stops registered.\n";
        }
    }

    void displayAllBuses() {
        cout << "\n=== ALL REGISTERED BUSES ===\n";

        LListNode<string>* busNode = allBusNumbers.getHead();
        int count = 1;

        while (busNode != nullptr) {
            Bus* bus = busesHash.search(busNode->data);
            if (bus) {
                cout << count++ << ". " << bus->busNumber
                    << " (" << bus->company << ") - Current: "
                    << bus->currentStopID << endl;
            }
            busNode = busNode->next;
        }

        if (count == 1) {
            cout << "No buses registered.\n";
        }
    }

    void displayRouteHistory() {
        cout << "\n=== RECENT ROUTE HISTORY ===\n";
        if (routeHistory.isEmpty()) {
            cout << "No history available" << endl;
            return;
        }

        Stack<string> tempStack;
        int count = 1;

        while (!routeHistory.isEmpty()) {
            string loc = routeHistory.pop();
            cout << count++ << ". " << loc << endl;
            tempStack.push(loc);
        }

        while (!tempStack.isEmpty()) {
            routeHistory.push(tempStack.pop());
        }
    }

    void displayStatistics() {
        cout << "\n=== TRANSPORT MODULE STATISTICS ===\n";
        cout << "Total Bus Stops: " << totalStops << endl;
        cout << "Total Buses: " << totalBuses << endl;
        cout << "Simulation Status: " << (simulationRunning ? "RUNNING" : "STOPPED") << endl;
        if (simulationRunning) {
            cout << "Simulation Tick: " << simulationTick << endl;
        }
        cout << "Passengers in Queue: " << (passengerQueue.isEmpty() ? 0 : 1) << "+ \n";
    }

    // ==========================================
    // GETTERS
    // ==========================================
    int getTotalStops() { return totalStops; }
    int getTotalBuses() { return totalBuses; }
    bool isSimulationRunning() { return simulationRunning; }
};

void displayTransportModuleMenu() {
    const string RESET = "\033[0m";
    const string YELLOW = "\033[33m";
    const string BRIGHT_YELLOW = "\033[93m";
    const string WHITE = "\033[37m";
    const string BRIGHT_WHITE = "\033[97m";
    const string CYAN = "\033[36m";

    cout << endl << BRIGHT_YELLOW << "========================================" << RESET << endl;
    cout << BRIGHT_YELLOW << "         TRANSPORT MODULE" << RESET << endl;
    cout << BRIGHT_YELLOW << "========================================" << RESET << endl << endl;

    cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
    cout << BRIGHT_YELLOW << "  SETUP & DATA LOADING" << RESET << endl;
    cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
    cout << BRIGHT_WHITE << "  1. Load Default Stops" << RESET << endl;
    cout << BRIGHT_WHITE << "  2. Load Stops from CSV" << RESET << endl;
    cout << BRIGHT_WHITE << "  3. Load Buses from CSV" << RESET << endl;
    cout << BRIGHT_WHITE << "  4. Register Transport Company" << RESET << endl << endl;

    cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
    cout << BRIGHT_YELLOW << "  BUS STOP MANAGEMENT" << RESET << endl;
    cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
    cout << BRIGHT_WHITE << "  5. Add Bus Stop" << RESET << endl;
    cout << BRIGHT_WHITE << "  6. Connect Two Bus Stops" << RESET << endl;
    cout << BRIGHT_WHITE << "  7. Auto-Connect Nearby Stops" << RESET << endl;
    cout << BRIGHT_WHITE << "  8. Delete Bus Stop" << RESET << endl;
    cout << BRIGHT_WHITE << "  9. Display All Bus Stops" << RESET << endl;
    cout << BRIGHT_WHITE << " 10. Get Stop Details" << RESET << endl << endl;

    cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
    cout << BRIGHT_YELLOW << "  BUS MANAGEMENT" << RESET << endl;
    cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
    cout << BRIGHT_WHITE << " 11. Register Bus" << RESET << endl;
    cout << BRIGHT_WHITE << " 12. Add Stop to Bus Route" << RESET << endl;
    cout << BRIGHT_WHITE << " 13. Update Bus Location" << RESET << endl;
    cout << BRIGHT_WHITE << " 14. Get Bus Details" << RESET << endl;
    cout << BRIGHT_WHITE << " 15. Display All Buses" << RESET << endl << endl;

    cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
    cout << BRIGHT_YELLOW << "  ROUTING & NAVIGATION" << RESET << endl;
    cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
    cout << BRIGHT_WHITE << " 16. Find Shortest Route Between Stops" << RESET << endl;
    cout << BRIGHT_WHITE << " 17. Display Transport Network" << RESET << endl;
    cout << BRIGHT_WHITE << " 18. Display Route History" << RESET << endl << endl;

    cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
    cout << BRIGHT_YELLOW << "  PASSENGER SERVICES" << RESET << endl;
    cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
    cout << BRIGHT_WHITE << " 19. Add Passenger to Queue" << RESET << endl;
    cout << BRIGHT_WHITE << " 20. Process Next Passenger" << RESET << endl << endl;

    cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
    cout << BRIGHT_YELLOW << "  BUS SIMULATION" << RESET << endl;
    cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
    cout << BRIGHT_WHITE << " 21. Start Simulation" << RESET << endl;
    cout << BRIGHT_WHITE << " 22. Simulate One Tick" << RESET << endl;
    cout << BRIGHT_WHITE << " 23. Display Live Bus Positions" << RESET << endl;
    cout << BRIGHT_WHITE << " 24. Stop Simulation" << RESET << endl << endl;

    cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
    cout << BRIGHT_YELLOW << "  AIRPORT MANAGEMENT" << RESET << endl;
    cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
    cout << BRIGHT_WHITE << " 25. Add Airport" << RESET << endl;
    cout << BRIGHT_WHITE << " 26. Add Flight" << RESET << endl;
    cout << BRIGHT_WHITE << " 27. Display All Airports" << RESET << endl << endl;

    cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
    cout << BRIGHT_YELLOW << "  RAILWAY MANAGEMENT" << RESET << endl;
    cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
    cout << BRIGHT_WHITE << " 28. Add Railway Station" << RESET << endl;
    cout << BRIGHT_WHITE << " 29. Add Train" << RESET << endl;
    cout << BRIGHT_WHITE << " 30. Display All Railway Stations" << RESET << endl << endl;

    cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
    cout << BRIGHT_YELLOW << "  REAL-TIME ROUTE SIMULATION" << RESET << endl;
    cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
    cout << BRIGHT_WHITE << " 31. Start Route Simulation (Bus/Flight/Train)" << RESET << endl;
    cout << BRIGHT_WHITE << " 32. Update Route Simulation (1 minute)" << RESET << endl;
    cout << BRIGHT_WHITE << " 33. Display Active Routes" << RESET << endl << endl;

    cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
    cout << BRIGHT_YELLOW << "  STATISTICS" << RESET << endl;
    cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
    cout << BRIGHT_WHITE << " 34. Display Transport Statistics" << RESET << endl << endl;

    cout << BRIGHT_WHITE << "  0. Back to Main Menu" << RESET << endl;
    cout << YELLOW << "--------------------------------------------------------" << RESET << endl;
    cout << BRIGHT_YELLOW << "Choose: " << RESET;
}

void TransportMenu(TransportModule& transport) {
    const string RESET = "\033[0m";
    const string CYAN = "\033[36m";
    const string BRIGHT_RED = "\033[91m";

    int ch;
    string a, b, c;
    double lat, lon;
    int d, maxD;

    while (true) {
        displayTransportModuleMenu();
        
        if (!(cin >> ch)) {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << BRIGHT_RED << "[ERROR] Invalid input. Please enter a number." << RESET << endl;
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            continue;
        }
        cin.ignore();

        if (ch == 0) return;

        switch (ch) {

            // -----------------------------
            // SETUP & DATA LOADING
            // -----------------------------
        case 1:
            transport.loadDefaultStops();
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        case 2:
            cout << "CSV File Name: ";
            getline(cin, a);
            transport.loadStopsFromCSV(a);
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        case 3:
            cout << "CSV File Name: ";
            getline(cin, a);
            transport.loadBusesFromCSV(a);
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        case 4:
            cout << "Company Name: ";
            getline(cin, a);
            transport.registerCompany(a);
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

            // -----------------------------
            // BUS STOP MANAGEMENT
            // -----------------------------
        case 5:
            cout << "Stop ID: ";
            getline(cin, a);
            cout << "Stop Name: ";
            getline(cin, b);
            cout << "Latitude: ";
            cin >> lat;
            cout << "Longitude: ";
            cin >> lon;
            cin.ignore();
            transport.addBusStop(a, b, lat, lon);
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        case 6:
            cout << "Stop 1 ID: ";
            getline(cin, a);
            cout << "Stop 2 ID: ";
            getline(cin, b);
            cout << "Distance (-1 = auto): ";
            cin >> d;
            cin.ignore();
            transport.connectBusStops(a, b, d);
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        case 7:
            cout << "Max Distance KM: ";
            cin >> maxD;
            cin.ignore();
            transport.autoConnectNearbyStops(maxD);
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        case 8:
            cout << "Stop ID to delete: ";
            getline(cin, a);
            transport.deleteBusStop(a);
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        case 9:
            transport.displayAllStops();
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        case 10:
            cout << "Stop ID: ";
            getline(cin, a);
            transport.getStopDetails(a);
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

            // -----------------------------
            // BUS MANAGEMENT
            // -----------------------------
        case 11:
            cout << "Bus Number: ";
            getline(cin, a);
            cout << "Company: ";
            getline(cin, b);
            transport.registerBus(a, b);
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        case 12:
            cout << "Bus Number: ";
            getline(cin, a);
            cout << "Stop ID to add: ";
            getline(cin, b);
            transport.addStopToRoute(a, b, true);
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        case 13:
            cout << "Bus Number: ";
            getline(cin, a);
            cout << "New Stop ID: ";
            getline(cin, b);
            transport.updateBusLocation(a, b);
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        case 14:
            cout << "Bus Number: ";
            getline(cin, a);
            transport.getBusDetails(a);
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        case 15:
            transport.displayAllBuses();
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

            // -----------------------------
            // ROUTING
            // -----------------------------
        case 16:
            cout << "Start Stop ID: ";
            getline(cin, a);
            cout << "End Stop ID: ";
            getline(cin, b);
            transport.findShortestRoute(a, b);
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        case 17:
            transport.displayTransportNetwork();
            break;

        case 18:
            transport.displayRouteHistory();
            break;

            // -----------------------------
            // PASSENGERS
            // -----------------------------
        case 19:
            cout << "Passenger Name: ";
            getline(cin, a);
            cout << "Current Stop: ";
            getline(cin, b);
            cout << "Destination Stop: ";
            getline(cin, c);
            transport.addPassengerToQueue(a, b, c);
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        case 20:
            transport.processNextPassenger();
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

            // -----------------------------
            // SIMULATION
            // -----------------------------
        case 21:
            transport.startSimulation();
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        case 22:
            transport.simulateOneTick();
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        case 23:
            transport.displayLiveBusPositions();
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        case 24:
            transport.stopSimulation();
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

            // -----------------------------
            // AIRPORT MANAGEMENT
            // -----------------------------
        case 25: {
            cout << "Airport ID: ";
            getline(cin, a);
            cout << "Airport Name: ";
            getline(cin, b);
            cout << "Latitude: ";
            cin >> lat;
            cout << "Longitude: ";
            cin >> lon;
            cin.ignore();
            transport.addAirport(a, b, lat, lon);
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;
        }

        case 26: {
            string airline, origin, dest, depTime, arrTime;
            cout << "Flight Number: ";
            getline(cin, a);
            cout << "Airline: ";
            getline(cin, airline);
            cout << "Origin Airport ID: ";
            getline(cin, origin);
            cout << "Destination Airport ID: ";
            getline(cin, dest);
            cout << "Departure Time (HH:MM): ";
            getline(cin, depTime);
            cout << "Arrival Time (HH:MM): ";
            getline(cin, arrTime);
            transport.addFlight(a, airline, origin, dest, depTime, arrTime);
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;
        }

        case 27:
            transport.displayAllAirports();
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

            // -----------------------------
            // RAILWAY MANAGEMENT
            // -----------------------------
        case 28: {
            cout << "Station ID: ";
            getline(cin, a);
            cout << "Station Name: ";
            getline(cin, b);
            cout << "Latitude: ";
            cin >> lat;
            cout << "Longitude: ";
            cin >> lon;
            cin.ignore();
            transport.addRailwayStation(a, b, lat, lon);
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;
        }

        case 29: {
            string trainName, origin, dest, depTime, arrTime;
            cout << "Train Number: ";
            getline(cin, a);
            cout << "Train Name: ";
            getline(cin, trainName);
            cout << "Origin Station ID: ";
            getline(cin, origin);
            cout << "Destination Station ID: ";
            getline(cin, dest);
            cout << "Departure Time (HH:MM): ";
            getline(cin, depTime);
            cout << "Arrival Time (HH:MM): ";
            getline(cin, arrTime);
            transport.addTrain(a, trainName, origin, dest, depTime, arrTime);
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;
        }

        case 30:
            transport.displayAllRailwayStations();
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

            // -----------------------------
            // REAL-TIME ROUTE SIMULATION
            // -----------------------------
        case 31: {
            string vehicleType, startLoc, dest;
            cout << "Vehicle Type (Bus/Flight/Train): ";
            getline(cin, vehicleType);
            cout << "Vehicle ID: ";
            getline(cin, a);
            cout << "Start Location: ";
            getline(cin, startLoc);
            cout << "Destination: ";
            getline(cin, dest);
            transport.startRouteSimulation(a, vehicleType, startLoc, dest);
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;
        }

        case 32:
            transport.updateRouteSimulation();
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        case 33:
            transport.displayActiveRoutes();
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

            // -----------------------------
            // STATISTICS
            // -----------------------------
        case 34:
            transport.displayStatistics();
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
            break;

        default:
            cout << BRIGHT_RED << "[ERROR] Invalid choice. Please enter a number between 0-34." << RESET << endl;
            cout << endl << CYAN << "Press Enter to continue..." << RESET;
            cin.get();
        }

        cout << endl << CYAN << "Press Enter to continue..." << RESET;
        cin.get();
    }
}
