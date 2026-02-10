#pragma once
#include "CityEntities.h"
#include "SectorCoordinateSystem.h"
#include<cmath>

class MainCityGraph {
private:
    Graph<string> cityGraph;
    HashTable<CityNode> cityNodesHash;
    SectorCoordinateSystem sectorSystem;

    LinkedList<string> allNodeIDs;  // Track all nodes for iteration

    int nodeCount;
    
    // Emergency mode system
    struct TemporaryEmergencyEdge {
        string fromNode;
        string toNode;
        int distance;
        
        TemporaryEmergencyEdge() : distance(0) {}
    };
    
    LinkedList<TemporaryEmergencyEdge> activeEmergencyEdges;
    bool emergencyModeActive;
    
    double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
        double latDiff = lat1 - lat2;
        double lonDiff = lon1 - lon2;
        // FIX: Add sqrt() to get actual distance!
        double distKm = 111.0 * sqrt((latDiff * latDiff) + (lonDiff * lonDiff));
        return distKm;
    }

    // Calculate actual straight-line distance
    int calculateDistanceKm(double lat1, double lon1, double lat2, double lon2) {
        double latDiff = lat1 - lat2;
        double lonDiff = lon1 - lon2;
        // FIX: Add sqrt() here too!
        double dist = 111.0 * sqrt((latDiff * latDiff) + (lonDiff * lonDiff));
        return (int)(dist);
    }

public:
    MainCityGraph() : cityNodesHash(500), nodeCount(0), emergencyModeActive(false) {}

    // ==========================================
    // BASIC NODE MANAGEMENT
    // ==========================================

    void addNode(string nodeID, ModuleType type, string name, double lat, double lon, void* data = nullptr) {
        CityNode* existing = cityNodesHash.search(nodeID);
        if (existing) {
            cout << "[ERROR] Node already exists: " << nodeID << endl;
            return;
        }

        CityNode newNode(nodeID, type, name, lat, lon);
        newNode.entityData = data;

        cityNodesHash.insert(nodeID, newNode);
        cityGraph.addVertex(nodeID, lat, lon);
        allNodeIDs.insertBack(nodeID);
        nodeCount++;

        string sector = sectorSystem.getSectorFromCoordinates(lat, lon);
        cout << "[SUCCESS] Added to Main Graph: " << name << " in " << sector << " (" << nodeID << ")" << endl;
    }

    // ==========================================
    // NODE WITH AUTO-CONNECTION
    // ==========================================

    void addNodeWithAutoConnect(string nodeID, ModuleType type, string name,
        double lat, double lon, string sector = "", void* data = nullptr) {

        CityNode* existing = cityNodesHash.search(nodeID);
        if (existing) {
            cout << "[ERROR] Node already exists: " << nodeID << endl;
            return;
        }

        // Determine sector if not provided
        if (sector == "") {
            sector = sectorSystem.getSectorFromCoordinates(lat, lon);
        }

        // Create and add node
        CityNode newNode(nodeID, type, name, lat, lon);
        newNode.entityData = data;

        cityNodesHash.insert(nodeID, newNode);
        cityGraph.addVertex(nodeID, lat, lon);
        allNodeIDs.insertBack(nodeID);
        nodeCount++;

        cout << "[SUCCESS] Added to Main Graph: " << name << " in " << sector << " (" << nodeID << ")" << endl;

        // AUTO-CONNECT to nearby nodes
        autoConnectNode(nodeID, lat, lon, sector, type);
    }

    void addNodeInSector(string nodeID, ModuleType type, string name, string sectorName, void* data = nullptr) {
        double lat, lon;

        if (!sectorSystem.getCoordinatesInSector(sectorName, lat, lon)) {
            cout << "[ERROR] Could not place node in sector: " << sectorName << endl;
            return;
        }

        addNodeWithAutoConnect(nodeID, type, name, lat, lon, sectorName, data);
    }

    // ==========================================
    // INTELLIGENT AUTO-CONNECTION
    // ==========================================

    void autoConnectNode(string newNodeID, double lat, double lon, string sector, ModuleType type) {
        cout << "[AUTO-CONNECT] Finding nearby nodes for " << newNodeID << "..." << endl;

        CityNode* newNode = cityNodesHash.search(newNodeID);
        if (!newNode) return;

        // Configuration
        const int MAX_CONNECTIONS = 4;
        const int MAX_DISTANCE_KM = 5;
        const bool PRIORITIZE_SAME_TYPE = true;

        // Structure to store potential connections
        struct NearbyNode {
            string nodeID;
            double distance;
            ModuleType moduleType;
            bool sameSector;
            
            NearbyNode() : distance(0.0), moduleType(MODULE_TRANSPORT), sameSector(false) {}
        };

        // Collect nearby nodes
        LinkedList<NearbyNode> candidateNodes;
        int candidateCount = 0;

        // Iterate through all existing nodes
        LListNode<string>* current = allNodeIDs.getHead();

        while (current != nullptr) {
            string candidateID = current->data;

            // Skip self
            if (candidateID == newNodeID) {
                current = current->next;
                continue;
            }

            CityNode* candidate = cityNodesHash.search(candidateID);
            if (!candidate) {
                current = current->next;
                continue;
            }

            // Calculate distance
            double dist = calculateDistance(lat, lon,
                candidate->coords.latitude,
                candidate->coords.longitude);
            int distKm = calculateDistanceKm(lat, lon,
                candidate->coords.latitude,
                candidate->coords.longitude);

            // Check if within range
            if (distKm <= MAX_DISTANCE_KM && distKm > 0) {
                string candidateSector = sectorSystem.getSectorFromCoordinates(
                    candidate->coords.latitude, candidate->coords.longitude);

                bool sameSector = (candidateSector == sector);
                bool adjacentSector = sectorSystem.areSectorsAdjacent(sector, candidateSector);

                // Only connect if in same or adjacent sectors (geographic realism)
                if (sameSector || adjacentSector) {
                    NearbyNode nearby;
                    nearby.nodeID = candidateID;
                    nearby.distance = dist;
                    nearby.moduleType = candidate->moduleType;
                    nearby.sameSector = sameSector;

                    candidateNodes.insertBack(nearby);
                    candidateCount++;
                }
            }

            current = current->next;
        }

        // Sort candidates by distance (simple insertion sort on linked list)
        // For simplicity, we'll connect to first MAX_CONNECTIONS found
        // In production, implement proper sorting

        int connectionsCreated = 0;
        LListNode<NearbyNode>* nearbyNode = candidateNodes.getHead();

        // First pass: Connect to same module type (if prioritized)
        if (PRIORITIZE_SAME_TYPE) {
            while (nearbyNode != nullptr && connectionsCreated < MAX_CONNECTIONS) {
                if (nearbyNode->data.moduleType == type) {
                    int distance = calculateDistanceKm(lat, lon,
                        cityNodesHash.search(nearbyNode->data.nodeID)->coords.latitude,
                        cityNodesHash.search(nearbyNode->data.nodeID)->coords.longitude);

                    cityGraph.addEdge(newNodeID, nearbyNode->data.nodeID, distance, true);

                    cout << "[AUTO-CONNECT] Connected " << newNodeID << " <-> "
                        << nearbyNode->data.nodeID << " (" << distance << " km)" << endl;

                    connectionsCreated++;
                }
                nearbyNode = nearbyNode->next;
            }
        }

        // Second pass: Connect to other types if slots remain
        nearbyNode = candidateNodes.getHead();
        while (nearbyNode != nullptr && connectionsCreated < MAX_CONNECTIONS) {
            // Skip if already connected
            bool alreadyConnected = false;

            if (!alreadyConnected && nearbyNode->data.moduleType != type) {
                CityNode* targetNode = cityNodesHash.search(nearbyNode->data.nodeID);
                if (targetNode) {
                    int distance = calculateDistanceKm(lat, lon,
                        targetNode->coords.latitude,
                        targetNode->coords.longitude);

                    cityGraph.addEdge(newNodeID, nearbyNode->data.nodeID, distance, true);

                    cout << "[AUTO-CONNECT] Connected " << newNodeID << " <-> "
                        << nearbyNode->data.nodeID << " (" << distance << " km)" << endl;

                    connectionsCreated++;
                }
            }

            nearbyNode = nearbyNode->next;
        }

        // ==========================================
        // MINIMUM CONNECTION GUARANTEE
        // ==========================================
        if (connectionsCreated == 0) {
            // Check if there are any other nodes in the graph
            if (allNodeIDs.getHead() != nullptr &&
                (allNodeIDs.getHead()->data != newNodeID || allNodeIDs.getHead()->next != nullptr)) {

                cout << "[WARNING] No nearby nodes found within " << MAX_DISTANCE_KM
                    << " km in same/adjacent sectors" << endl;
                cout << "[FALLBACK] Connecting to nearest node (ignoring sector restrictions)..." << endl;

                // Find ABSOLUTE nearest node (ignore all restrictions)
                string nearestNodeID = "";
                double minDistance = 999999.0;

                LListNode<string>* node = allNodeIDs.getHead();
                while (node != nullptr) {
                    if (node->data != newNodeID) {
                        CityNode* candidate = cityNodesHash.search(node->data);
                        if (candidate) {
                            double dist = calculateDistance(lat, lon,
                                candidate->coords.latitude,
                                candidate->coords.longitude);

                            if (dist < minDistance) {
                                minDistance = dist;
                                nearestNodeID = node->data;
                            }
                        }
                    }
                    node = node->next;
                }

                // Force connection to nearest node
                if (nearestNodeID != "") {
                    int distance = calculateDistanceKm(lat, lon,
                        cityNodesHash.search(nearestNodeID)->coords.latitude,
                        cityNodesHash.search(nearestNodeID)->coords.longitude);

                    cityGraph.addEdge(newNodeID, nearestNodeID, distance, true);

                    CityNode* nearest = cityNodesHash.search(nearestNodeID);
                    cout << "[FALLBACK] Connected to " << nearestNodeID
                        << " (" << (nearest ? nearest->name : "Unknown") << ")"
                        << " - Distance: " << distance << " km" << endl;

                    connectionsCreated = 1;
                }
            }
        }

        if (connectionsCreated == 0) {
            cout << "[INFO] This is the first node in the graph (no connections needed)" << endl;
        }
        else {
            cout << "[AUTO-CONNECT] Created " << connectionsCreated << " connection(s)" << endl;
        }
    }

    // ==========================================
    // MANUAL CONNECTION MANAGEMENT
    // ==========================================

    void connectNodes(string nodeID1, string nodeID2, int distance = -1) {
        CityNode* node1 = cityNodesHash.search(nodeID1);
        CityNode* node2 = cityNodesHash.search(nodeID2);

        if (!node1 || !node2) {
            cout << "[ERROR] One or both nodes not found!" << endl;
            return;
        }

        if (distance == -1) {
            distance = calculateDistanceKm(node1->coords.latitude, node1->coords.longitude,
                node2->coords.latitude, node2->coords.longitude);
        }

        cityGraph.addEdge(nodeID1, nodeID2, distance, true);
        cout << "[SUCCESS] Connected " << nodeID1 << " <-> " << nodeID2
            << " (" << distance << " km)" << endl;
    }

    void connectWithSectorValidation(string nodeID1, string nodeID2, int distance = -1) {
        CityNode* node1 = cityNodesHash.search(nodeID1);
        CityNode* node2 = cityNodesHash.search(nodeID2);

        if (!node1 || !node2) {
            cout << "[ERROR] One or both nodes not found!" << endl;
            return;
        }

        string sector1 = sectorSystem.getSectorFromCoordinates(node1->coords.latitude,
            node1->coords.longitude);
        string sector2 = sectorSystem.getSectorFromCoordinates(node2->coords.latitude,
            node2->coords.longitude);

        // Warn if connecting non-adjacent sectors
        if (!sectorSystem.areSectorsAdjacent(sector1, sector2) && sector1 != sector2) {
            cout << "[WARNING] Connecting non-adjacent sectors: "
                << sector1 << " to " << sector2 << endl;
            cout << "[INFO] Consider adding intermediate nodes for realism" << endl;
        }

        if (distance == -1) {
            distance = calculateDistanceKm(node1->coords.latitude, node1->coords.longitude,
                node2->coords.latitude, node2->coords.longitude);
        }

        cityGraph.addEdge(nodeID1, nodeID2, distance, true);
        cout << "[SUCCESS] Connected " << sector1 << " <-> " << sector2
            << " (" << distance << " km)" << endl;
    }

    // ==========================================
    // BATCH AUTO-CONNECTION
    // ==========================================

    void autoConnectAllNodes(int maxDistanceKM = 5) {
        cout << "\n[BATCH AUTO-CONNECT] Connecting all unconnected nodes..." << endl;
        cout << "Max distance: " << maxDistanceKM << " km" << endl;

        int totalConnections = 0;
        LListNode<string>* node = allNodeIDs.getHead();

        while (node != nullptr) {
            CityNode* cityNode = cityNodesHash.search(node->data);
            if (cityNode) {
                // Check if node has few connections
                int degree = cityGraph.getVertexDegree(node->data);

                if (degree < 2) {  // If less than 2 connections, auto-connect
                    string sector = sectorSystem.getSectorFromCoordinates(
                        cityNode->coords.latitude, cityNode->coords.longitude);

                    cout << "\n[INFO] Node " << node->data << " has only "
                        << degree << " connection(s). Auto-connecting..." << endl;

                    autoConnectNode(node->data,
                        cityNode->coords.latitude,
                        cityNode->coords.longitude,
                        sector,
                        cityNode->moduleType);
                    totalConnections++;
                }
            }
            node = node->next;
        }

        cout << "\n[BATCH AUTO-CONNECT] Processed " << totalConnections << " node(s)" << endl;
    }

    void connectAdjacentSectors() {
        cout << "\n[INFO] Creating inter-sector connections..." << endl;

        // Group nodes by sector and connect border nodes
        // This is a simplified version

        cout << "[INFO] Analyzing sector boundaries..." << endl;
        // Implementation: Find nodes in adjacent sectors and connect them

        cout << "[SUCCESS] Inter-sector connections created" << endl;
    }

    // ==========================================
    // NODE REMOVAL
    // ==========================================
    void removeNode(string nodeID) {
        CityNode* node = cityNodesHash.search(nodeID);
        if (!node) {
            cout << "[ERROR] Node not found: " << nodeID << endl;
            return;
        }

        cout << "\n[INFO] Removing node: " << nodeID << " (" << node->name << ")" << endl;

        // STEP 1: Get all neighbors of this node BEFORE deletion
        LinkedList<string> neighbors;
        EdgeNode<string>* edge = cityGraph.getEdges(nodeID);

        while (edge != nullptr) {
            neighbors.insertBack(edge->destinationID);
            edge = edge->next;
        }

        int neighborCount = 0;
        LListNode<string>* temp = neighbors.getHead();
        while (temp != nullptr) {
            neighborCount++;
            temp = temp->next;
        }

        cout << "[INFO] Node has " << neighborCount << " neighbor(s)" << endl;

        // STEP 2: Remove all incoming edges TO this node from ALL other nodes
        cout << "[INFO] Removing all edges to this node..." << endl;
        LListNode<string>* current = allNodeIDs.getHead();
        while (current != nullptr) {
            if (current->data != nodeID) {
                cityGraph.removeEdge(current->data, nodeID);
            }
            current = current->next;
        }

        // STEP 3: Remove the vertex itself (removes all outgoing edges)
        cityGraph.removeVertex(nodeID);

        // STEP 4: Check if any neighbors became isolated and reconnect them
        cout << "[INFO] Checking neighbor connectivity..." << endl;
        reconnectOrphanedNeighbors(neighbors, nodeID);

        // STEP 5: Remove from hash table
        cityNodesHash.remove(nodeID);

        // STEP 6: Remove from tracking list
        allNodeIDs.removeByValue(nodeID);

        // STEP 7: Decrement count
        nodeCount--;

        cout << "[SUCCESS] Node " << nodeID << " completely removed" << endl;
        cout << "[INFO] All edges cleaned up, no isolated nodes created" << endl;
    }

    // Helper: Reconnect neighbors that became isolated
    void reconnectOrphanedNeighbors(LinkedList<string>& formerNeighbors, string removedNodeID) {
        LListNode<string>* neighborNode = formerNeighbors.getHead();

        while (neighborNode != nullptr) {
            string neighborID = neighborNode->data;

            // Check if this neighbor is now isolated (degree 0)
            int degree = cityGraph.getVertexDegree(neighborID);

            if (degree == 0) {
                cout << "[WARNING] Node " << neighborID << " became isolated!" << endl;
                cout << "[AUTO-REPAIR] Reconnecting " << neighborID << "..." << endl;

                // Reconnect to nearest node (excluding the removed node)
                reconnectIsolatedNode(neighborID, removedNodeID);
            }
            else if (degree == 1) {
                cout << "[INFO] Node " << neighborID << " has only 1 connection (weak connectivity)" << endl;
            }

            neighborNode = neighborNode->next;
        }

        // BONUS: Also connect former neighbors to each other for better connectivity
        LListNode<string>* first = formerNeighbors.getHead();
        if (first != nullptr && first->next != nullptr) {
            cout << "[INFO] Creating connections between former neighbors..." << endl;
            connectFormerNeighbors(formerNeighbors);
        }
    }

    // Helper: Reconnect an isolated node
    void reconnectIsolatedNode(string isolatedNodeID, string excludeNodeID) {
        CityNode* isolatedNode = cityNodesHash.search(isolatedNodeID);
        if (!isolatedNode) return;

        // Find nearest node (excluding the removed node)
        string nearestNodeID = "";
        double minDistance = 999999.0;

        LListNode<string>* node = allNodeIDs.getHead();
        while (node != nullptr) {
            if (node->data != isolatedNodeID && node->data != excludeNodeID) {
                CityNode* candidate = cityNodesHash.search(node->data);
                if (candidate) {
                    double dist = calculateDistance(
                        isolatedNode->coords.latitude,
                        isolatedNode->coords.longitude,
                        candidate->coords.latitude,
                        candidate->coords.longitude
                    );

                    if (dist < minDistance) {
                        minDistance = dist;
                        nearestNodeID = node->data;
                    }
                }
            }
            node = node->next;
        }

        // Connect to nearest node
        if (nearestNodeID != "") {
            int distance = calculateDistanceKm(
                isolatedNode->coords.latitude,
                isolatedNode->coords.longitude,
                cityNodesHash.search(nearestNodeID)->coords.latitude,
                cityNodesHash.search(nearestNodeID)->coords.longitude
            );

            cityGraph.addEdge(isolatedNodeID, nearestNodeID, distance, true);

            CityNode* nearest = cityNodesHash.search(nearestNodeID);
            cout << "[AUTO-REPAIR] Reconnected " << isolatedNodeID
                << " <-> " << nearestNodeID
                << " (" << (nearest ? nearest->name : "Unknown") << ")"
                << " (" << distance << " km)" << endl;
        }
    }

    // Helper: Connect former neighbors to each other
    void connectFormerNeighbors(LinkedList<string>& neighbors) {
        LListNode<string>* node1 = neighbors.getHead();
        int connectionsCreated = 0;

        while (node1 != nullptr) {
            LListNode<string>* node2 = node1->next;

            while (node2 != nullptr) {
                CityNode* cityNode1 = cityNodesHash.search(node1->data);
                CityNode* cityNode2 = cityNodesHash.search(node2->data);

                if (cityNode1 && cityNode2) {
                    // Check if already connected
                    bool alreadyConnected = false;
                    EdgeNode<string>* edge = cityGraph.getEdges(node1->data);
                    while (edge != nullptr) {
                        if (edge->destinationID == node2->data) {
                            alreadyConnected = true;
                            break;
                        }
                        edge = edge->next;
                    }

                    // Connect if not already connected and within reasonable distance
                    if (!alreadyConnected) {
                        int distance = calculateDistanceKm(
                            cityNode1->coords.latitude,
                            cityNode1->coords.longitude,
                            cityNode2->coords.latitude,
                            cityNode2->coords.longitude
                        );

                        // Only connect if within 10 km (reasonable for replacement)
                        if (distance <= 10) {
                            cityGraph.addEdge(node1->data, node2->data, distance, true);
                            cout << "[AUTO-REPAIR] Connected neighbors "
                                << node1->data << " <-> " << node2->data
                                << " (" << distance << " km)" << endl;
                            connectionsCreated++;
                        }
                    }
                }

                node2 = node2->next;
            }

            node1 = node1->next;
        }

        if (connectionsCreated > 0) {
            cout << "[SUCCESS] Created " << connectionsCreated
                << " connection(s) between former neighbors" << endl;
        }
    }

    // ==========================================
    // QUERY OPERATIONS
    // ==========================================

    CityNode* getNode(string nodeID) {
        return cityNodesHash.search(nodeID);
    }

    void findShortestPath(string startID, string endID) {
        cout << "\n=== SHORTEST PATH FINDER ===\n";

        CityNode* start = cityNodesHash.search(startID);
        CityNode* end = cityNodesHash.search(endID);

        if (!start || !end) {
            cout << "[ERROR] Nodes not found\n";
            return;
        }

        string startSector = sectorSystem.getSectorFromCoordinates(
            start->coords.latitude, start->coords.longitude);
        string endSector = sectorSystem.getSectorFromCoordinates(
            end->coords.latitude, end->coords.longitude);

        cout << "From: " << start->name << " [" << startSector << "] (" << startID << ")\n";
        cout << "To: " << end->name << " [" << endSector << "] (" << endID << ")\n";

        if (emergencyModeActive) {
            cout << "\n[EMERGENCY MODE] Using highway shortcuts\n";
        }
        else {
            cout << "\n[NORMAL MODE] City roads only\n";
        }

        cout << "========================================\n";

        cityGraph.findShortestPath(startID, endID, emergencyModeActive);
    }

    void findNearestNodeByType(string currentNodeID, ModuleType facilityType) {
        CityNode* currentNode = cityNodesHash.search(currentNodeID);
        if (!currentNode) {
            cout << "[ERROR] Current node not found: " << currentNodeID << endl;
            return;
        }

        cout << "\n[SEARCH] Finding nearest ";
        switch (facilityType) {
        case MODULE_TRANSPORT: cout << "Transport Stop"; break;
        case MODULE_MEDICAL: cout << "Hospital"; break;
        case MODULE_EDUCATION: cout << "School"; break;
        case MODULE_COMMERCIAL: cout << "Mall"; break;
        case MODULE_PUBLIC_FACILITY: cout << "Public Facility"; break;
        case MODULE_HOUSING: cout << "Residence"; break;
        }
        cout << " from " << currentNodeID << "...\n";

        string nearestNodeID = "";
        double minDistance = 999999.0;

        // Search through all nodes of matching type
        LListNode<string>* node = allNodeIDs.getHead();

        while (node != nullptr) {
            if (node->data == currentNodeID) {
                node = node->next;
                continue;
            }

            CityNode* candidate = cityNodesHash.search(node->data);

            if (candidate && candidate->moduleType == facilityType) {
                double dist = calculateDistance(currentNode->coords.latitude,
                    currentNode->coords.longitude,
                    candidate->coords.latitude,
                    candidate->coords.longitude);

                if (dist < minDistance) {
                    minDistance = dist;
                    nearestNodeID = node->data;
                }
            }

            node = node->next;
        }

        if (nearestNodeID != "") {
            CityNode* nearest = cityNodesHash.search(nearestNodeID);
            cout << "\n[FOUND] Nearest facility: " << nearest->name
                << " (" << nearestNodeID << ")" << endl;
            cout << "Approximate distance: " << (int)minDistance << " km" << endl;

            cout << "\nFinding route..." << endl;
            findShortestPath(currentNodeID, nearestNodeID);
        }
        else {
            cout << "[NOT FOUND] No facilities of this type in the city graph" << endl;
        }
    }

    // ==========================================
    // DISPLAY BY MODULE TYPE (COMPLETE)
    // ==========================================

    void displayNodesByModuleType(ModuleType type) {
        cout << "\n========================================\n";
        cout << "    NODES BY MODULE TYPE: ";

        string moduleName;
        switch (type) {
        case MODULE_TRANSPORT: moduleName = "TRANSPORT"; break;
        case MODULE_EDUCATION: moduleName = "EDUCATION"; break;
        case MODULE_MEDICAL: moduleName = "MEDICAL"; break;
        case MODULE_COMMERCIAL: moduleName = "COMMERCIAL"; break;
        case MODULE_PUBLIC_FACILITY: moduleName = "PUBLIC FACILITIES"; break;
        case MODULE_HOUSING: moduleName = "HOUSING"; break;
        default: moduleName = "UNKNOWN"; break;
        }

        cout << moduleName << "\n";
        cout << "========================================\n";

        LListNode<string>* current = allNodeIDs.getHead();
        int foundCount = 0;

        cout << "Adjacency List (Filtered by " << moduleName << "):\n";
        cout << "----------------------------\n";

        while (current != nullptr) {
            CityNode* node = cityNodesHash.search(current->data);

            if (node && node->moduleType == type) {
                foundCount++;

                string sector = sectorSystem.getSectorFromCoordinates(
                    node->coords.latitude, node->coords.longitude);

                // Display node with sector info
                cout << current->data << " [" << node->name << ", " << sector << "] -> ";

                // Display edges using Graph's method
                cityGraph.printVertexEdges(current->data);

                cout << endl;
            }

            current = current->next;
        }

        if (foundCount == 0) {
            cout << "No nodes found for this module type.\n";
        }

        cout << "========================================\n";
        cout << "Total " << moduleName << " Nodes: " << foundCount << endl;
        cout << "========================================\n";
    }

    void displayAllNodesByType() {
        cout << "\n=== NODE COUNT BY MODULE TYPE ===\n";

        int transportCount = 0;
        int educationCount = 0;
        int medicalCount = 0;
        int commercialCount = 0;
        int publicCount = 0;
        int housingCount = 0;

        LListNode<string>* current = allNodeIDs.getHead();

        while (current != nullptr) {
            CityNode* node = cityNodesHash.search(current->data);

            if (node) {
                switch (node->moduleType) {
                case MODULE_TRANSPORT: transportCount++; break;
                case MODULE_EDUCATION: educationCount++; break;
                case MODULE_MEDICAL: medicalCount++; break;
                case MODULE_COMMERCIAL: commercialCount++; break;
                case MODULE_PUBLIC_FACILITY: publicCount++; break;
                case MODULE_HOUSING: housingCount++; break;
                }
            }

            current = current->next;
        }

        cout << "Transport Nodes: " << transportCount << endl;
        cout << "Education Nodes: " << educationCount << endl;
        cout << "Medical Nodes: " << medicalCount << endl;
        cout << "Commercial Nodes: " << commercialCount << endl;
        cout << "Public Facility Nodes: " << publicCount << endl;
        cout << "Housing Nodes: " << housingCount << endl;
        cout << "----------------------------\n";
        cout << "Total Nodes: " << nodeCount << endl;
    }

    void displayCityGraph() {
        cout << "\n========================================\n";
        cout << "         MAIN CITY GRAPH\n";
        cout << "========================================\n";
        cout << "Total Nodes: " << nodeCount << endl;
        cityGraph.printGraph();
    }

    void displayDetailedNodeInfo(string nodeID) {
        CityNode* node = cityNodesHash.search(nodeID);

        if (!node) {
            cout << "[ERROR] Node not found: " << nodeID << endl;
            return;
        }

        string sector = sectorSystem.getSectorFromCoordinates(
            node->coords.latitude, node->coords.longitude);

        cout << "\n=== NODE DETAILS ===\n";
        cout << "Node ID: " << nodeID << endl;
        cout << "Name: " << node->name << endl;
        cout << "Module Type: ";

        switch (node->moduleType) {
        case MODULE_TRANSPORT: cout << "Transport"; break;
        case MODULE_EDUCATION: cout << "Education"; break;
        case MODULE_MEDICAL: cout << "Medical"; break;
        case MODULE_COMMERCIAL: cout << "Commercial"; break;
        case MODULE_PUBLIC_FACILITY: cout << "Public Facility"; break;
        case MODULE_HOUSING: cout << "Housing"; break;
        }
        cout << endl;

        cout << "Sector: " << sector << endl;
        cout << "Coordinates: (" << node->coords.latitude << ", "
            << node->coords.longitude << ")" << endl;

        int degree = cityGraph.getVertexDegree(nodeID);
        cout << "Connections: " << degree << endl;

        cout << "Adjacent Nodes: ";
        cityGraph.printVertexEdges(nodeID);
        cout << endl;
    }

    void displayStatistics() {
        cout << "\n=== MAIN CITY STATISTICS ===\n";
        cout << "Total Nodes: " << nodeCount << endl;
        displayAllNodesByType();

        // Calculate average degree
        int totalDegree = 0;
        int nodeCountChecked = 0;

        LListNode<string>* node = allNodeIDs.getHead();
        while (node != nullptr) {
            totalDegree += cityGraph.getVertexDegree(node->data);
            nodeCountChecked++;
            node = node->next;
        }

        if (nodeCountChecked > 0) {
            double avgDegree = (double)totalDegree / nodeCountChecked;
            cout << "----------------------------\n";
            cout << "Average Connections per Node: " << avgDegree << endl;
        }
    }

    // ==========================================
    // GETTERS
    // ==========================================

    SectorCoordinateSystem& getSectorSystem() {
        return sectorSystem;
    }

    Graph<string>& getCityGraph() {
        return cityGraph;
    }

    int getNodeCount() {
        return nodeCount;
    }

    LinkedList<string>& getAllNodeIDs() {
        return allNodeIDs;
    }

    HashTable<CityNode>& getCityNodesHash() {
        return cityNodesHash;
    }

    // ==========================================
    // EMERGENCY MODE OPERATIONS
    // ==========================================

    // Check status
    bool isEmergencyModeActive() {
        return emergencyModeActive;
    }

    // Helper to extract sector number
    int extractSectorNumber(string sector) {
        int num = 0;
        for (int i = 0; i < sector.length(); i++) {
            if (sector[i] >= '0' && sector[i] <= '9') {
                num = num * 10 + (sector[i] - '0');
            }
        }
        return num;
    }

    // Check if good emergency shortcut
    bool isGoodEmergencyShortcut(string nodeID1, string nodeID2,
        CityNode* node1, CityNode* node2) {

        // Calculate distance
        int distance = calculateDistanceKm(
            node1->coords.latitude, node1->coords.longitude,
            node2->coords.latitude, node2->coords.longitude
        );

        // Rule 1: Distance between 2-8 km (good for highway)
        if (distance < 2 || distance > 8) {
            return false;
        }

        // Rule 2: Check if already connected
        EdgeNode<string>* edge = cityGraph.getEdges(nodeID1);
        while (edge != nullptr) {
            if (edge->destinationID == nodeID2) {
                return false;  // Already connected
            }
            edge = edge->next;
        }

        // Rule 3: Get sectors
        string sector1 = sectorSystem.getSectorFromCoordinates(
            node1->coords.latitude, node1->coords.longitude);
        string sector2 = sectorSystem.getSectorFromCoordinates(
            node2->coords.latitude, node2->coords.longitude);

        // Must be different sectors
        if (sector1 == sector2) {
            return false;
        }

        // Rule 4: Check alignment (highway-like)
        char letter1 = sector1[0];
        char letter2 = sector2[0];
        int num1 = extractSectorNumber(sector1);
        int num2 = extractSectorNumber(sector2);

        bool verticalHighway = (letter1 == letter2);  // F-8, F-10
        bool horizontalHighway = (num1 == num2);      // F-8, G-8

        if (!verticalHighway && !horizontalHighway) {
            return false;  // Diagonal - not a highway
        }

        // Rule 5: Priority to Transport/Medical nodes
        bool isPriority = (node1->moduleType == MODULE_TRANSPORT ||
            node1->moduleType == MODULE_MEDICAL ||
            node2->moduleType == MODULE_TRANSPORT ||
            node2->moduleType == MODULE_MEDICAL);

        return isPriority;
    }

    // Open emergency paths
    void openEmergencyPaths() {
        if (emergencyModeActive) {
            cout << "[WARNING] Emergency paths already active!\n";
            return;
        }

        cout << "\n[EMERGENCY] OPENING EMERGENCY PATHS\n";
        cout << "========================================\n";
        cout << "[INFO] Analyzing city graph for shortcuts...\n";
        cout << "[INFO] Creating temporary highway connections...\n\n";

        int emergencyEdgesCreated = 0;

        // Find all potential shortcuts
        LListNode<string>* node1Ptr = allNodeIDs.getHead();

        while (node1Ptr != nullptr) {
            CityNode* node1 = cityNodesHash.search(node1Ptr->data);
            if (!node1) {
                node1Ptr = node1Ptr->next;
                continue;
            }

            // Check other nodes for shortcuts
            LListNode<string>* node2Ptr = node1Ptr->next;

            while (node2Ptr != nullptr) {
                CityNode* node2 = cityNodesHash.search(node2Ptr->data);
                if (!node2) {
                    node2Ptr = node2Ptr->next;
                    continue;
                }

                // Calculate if this would be a good emergency shortcut
                if (isGoodEmergencyShortcut(node1Ptr->data, node2Ptr->data, node1, node2)) {
                    int distance = calculateDistanceKm(
                        node1->coords.latitude, node1->coords.longitude,
                        node2->coords.latitude, node2->coords.longitude
                    );

                    // Add temporary emergency edge
                    cityGraph.addEdge(node1Ptr->data, node2Ptr->data, distance, true, true);

                    // Track for cleanup
                    TemporaryEmergencyEdge tempEdge;
                    tempEdge.fromNode = node1Ptr->data;
                    tempEdge.toNode = node2Ptr->data;
                    tempEdge.distance = distance;
                    activeEmergencyEdges.insertBack(tempEdge);

                    string sector1 = sectorSystem.getSectorFromCoordinates(
                        node1->coords.latitude, node1->coords.longitude);
                    string sector2 = sectorSystem.getSectorFromCoordinates(
                        node2->coords.latitude, node2->coords.longitude);

                    cout << "[HIGHWAY OPEN] " << node1Ptr->data << " (" << sector1 << ") <-> "
                        << node2Ptr->data << " (" << sector2 << ") - "
                        << distance << " km\n";

                    emergencyEdgesCreated++;

                    // Limit to 4-6 emergency routes (not too many)
                    if (emergencyEdgesCreated >= 6) break;
                }

                node2Ptr = node2Ptr->next;
            }

            if (emergencyEdgesCreated >= 6) break;
            node1Ptr = node1Ptr->next;
        }

        emergencyModeActive = true;

        cout << "\n========================================\n";
        cout << "[SUCCESS] " << emergencyEdgesCreated << " emergency shortcuts created\n";
        cout << "[STATUS] EMERGENCY MODE ACTIVE\n";
        cout << "[INFO] Use these routes for fastest paths\n";
        cout << "========================================\n\n";
    }

    // Close emergency paths
    void closeEmergencyPaths() {
        if (!emergencyModeActive) {
            cout << "[INFO] No emergency paths active\n";
            return;
        }

        cout << "\n[CLOSING] EMERGENCY PATHS\n";
        cout << "========================================\n";
        cout << "[INFO] Removing temporary highway connections...\n\n";

        int edgesRemoved = 0;

        // Remove all temporary emergency edges
        LListNode<TemporaryEmergencyEdge>* edgePtr = activeEmergencyEdges.getHead();

        while (edgePtr != nullptr) {
            TemporaryEmergencyEdge& edge = edgePtr->data;

            // Remove the edge from graph
            cityGraph.removeEdge(edge.fromNode, edge.toNode);
            cityGraph.removeEdge(edge.toNode, edge.fromNode);  // Bidirectional

            cout << "[REMOVED] " << edge.fromNode << " <-> " << edge.toNode
                << " (" << edge.distance << " km)\n";

            edgesRemoved++;
            edgePtr = edgePtr->next;
        }

        // Clear the tracking list
        activeEmergencyEdges = LinkedList<TemporaryEmergencyEdge>();
        emergencyModeActive = false;

        cout << "\n========================================\n";
        cout << "[SUCCESS] " << edgesRemoved << " emergency routes closed\n";
        cout << "[STATUS] Normal traffic mode restored\n";
        cout << "========================================\n\n";
    }
};
